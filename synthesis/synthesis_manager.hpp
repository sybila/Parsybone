/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_SYNTHESIS_MANAGER_INCLUDED
#define PARSYBONE_SYNTHESIS_MANAGER_INCLUDED

#include "../auxiliary/time_manager.hpp"
#include "coloring_analyzer.hpp"
#include "witness_searcher.hpp"
#include "output_manager.hpp"
#include "color_storage.hpp"
#include "model_checker.hpp"
#include "paramset_helper.hpp"
#include "split_manager.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Manager of the synthesis procedure - takes the reference data constructed during previous steps and computes and executes the synthesis.
/// Synthesis is done in three steps:
///	-# preparation: empties data and starts a new round.
///   -# synthesis: computes the coloring, stored in the storage object and adds data to coloring analyzer if needed.
///   -# conclusion: stores additional data and outputs
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SynthesisManager {
    const ConstructionHolder & holder; ///< Holder of all the reference data.

	std::unique_ptr<ColoringAnalyzer> analyzer; ///< Class for analysis
	std::unique_ptr<ModelChecker> model_checker; ///< Class for synthesis
	std::unique_ptr<OutputManager> output; ///< Class for output
	std::unique_ptr<SplitManager> split_manager; ///< Control of independent rounds
	std::unique_ptr<ColorStorage> storage; ///< Class that holds
	std::unique_ptr<WitnessSearcher> searcher; ///< Class to build wintesses

	/// Overall statistics
	std::size_t total_colors;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SYNTHESIS CONTROL
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	/**
	 * Setup everything that needs it for computation in this round
	 */
	void doPreparation() {
		// Assure emptyness
		storage.get()->reset();
		// Output round number
		output->outputRoundNum();
		// Pass information about round (necessary for setup)
		analyzer->strartNewRound(split_manager->getRoundRange());
	}

	/**
	 * Store results that have not been stored yet and finalize the round where needed
	 */
	void doConclusion() {
        total_colors += paramset_helper.count(analyzer->getMask());
		// Output what has been synthetized (colors, witnesses)
		output->outputRound();
	}

	/**
	 * Entry point of the parameter synthesis. 
	 * In the first part, all states are colored with parameters that are transitive from some initial state. At the end, all final states are stored together with their color.
	 * In the second part, for all final states the strucutre is reset and colores are distributed from the state. After coloring the resulting color of the state is stored.
	 */
	void doComputation() {
		// Basic (initial) coloring
        colorProduct();

		// Store colored final vertices
		std::vector<Coloring> final_states = std::move(storage.get()->getColor(holder.getProduct().getFinalStates()));
		// Get the actuall results by cycle detection for each final vertex
		for (auto final_it = final_states.begin(); final_it != final_states.end(); final_it++) {
			// For general property, there must be new coloring for each final state!
            if (!paramset_helper.none(final_it->second) && !user_options.timeSerie())
				detectCycle(*final_it);

			// Store results from this final state
			analyzer->storeResults(Coloring(final_it->first, storage.get()->getColor(final_it->first)));
		}
	}

	/**
	 * Do initial coloring of states - start from initial states and distribute all the transitible parameters.
	 */
    void colorProduct() {
		// Get initial coloring
		Paramset starting;
		if(coloring_parser.input())
			starting = coloring_parser.getColors()[static_cast<unsigned int>(split_manager->getRoundNum())];
		else
			starting = split_manager->createStartingParameters();

        if (paramset_helper.none(starting))
			return;

		// Set all the initial states to initial color
		for (auto init_it = holder.getProduct().getInitialStates().begin(); init_it != holder.getProduct().getInitialStates().end(); init_it++)
			storage.get()->update(starting, *init_it);

		// Schedule all initial states for updates
		std::set<StateID> updates(holder.getProduct().getInitialStates().begin(), holder.getProduct().getInitialStates().end());

		// Start coloring procedure
		model_checker->startColoring(starting, updates, split_manager->getRoundRange());
	}

	/**
	 * For each final state that has at least one parameter assigned, start cycle detection.
	 *
	 * @param init_coloring	reference to the final state that starts the coloring search with its parameters
	 */
	void detectCycle(const Coloring & init_coloring) {
		// Assure emptyness
		storage.get()->reset();

		// Sechedule nothing for updates (will be done during transfer in the next step)
		model_checker->startColoring(init_coloring.first, init_coloring.second, split_manager->getRoundRange());
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREATION
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	SynthesisManager(const SynthesisManager & other);            // Forbidden copy constructor.
	SynthesisManager& operator=(const SynthesisManager & other); // Forbidden assignment operator.

public:
	/**
	 * Constructor builds all the data objects that are used within
	 */
	SynthesisManager(const ConstructionHolder & _holder) : holder(_holder) {
		// Create classes that help with the synthesis
		analyzer.reset(new ColoringAnalyzer(holder));
		storage.reset(new ColorStorage(holder.getProduct().getStateCount()));
		split_manager.reset(new SplitManager(holder.getParametrizations().getSpaceSize()));
		model_checker.reset(new ModelChecker(holder, *storage.get()));
		searcher.reset(new WitnessSearcher(holder, *analyzer, *storage.get()));
		output.reset(new OutputManager(*analyzer, *split_manager, *searcher));

		total_colors = 0;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SYNTHESIS ENTRY FUNCTION
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Main synthesis function that iterates through all the rounds of the synthesis
	 */
	void doSynthesis() {
		time_manager.startClock("coloring");

		// Do the computation for all the rounds
		for (;split_manager->valid(); split_manager->increaseRound()) {
			doPreparation();
			doComputation();
			doConclusion();
		}

		time_manager.ouputClock("coloring");
		output->outputSummary(total_colors);
	}
};

#endif
