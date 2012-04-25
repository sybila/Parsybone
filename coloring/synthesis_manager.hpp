/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_SYNTHESIS_MANAGER_INCLUDED
#define PARSYBONE_SYNTHESIS_MANAGER_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class that shelters all of the synthesis and output of the results
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../auxiliary/time_manager.hpp"
#include "parameters_functions.hpp"
#include "model_checker.hpp"
#include "split_manager.hpp"
#include "../results/coloring_analyzer.hpp"
#include "../results/witness_searcher.hpp"
#include "../results/output_manager.hpp"

class SynthesisManager {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Provided
	const ParametrizedStructure & structure; // Stores info about KS states
	const AutomatonStructure & automaton; // Stores info about BA states
	const ProductStructure & product; // Product to compute on
	ColorStorage & storage; // Auxiliary product storage

	// Created within the constructor
	std::unique_ptr<SplitManager> split_manager; // Control of independent rounds
	std::unique_ptr<OutputManager> output; // Class for output
	std::unique_ptr<ModelChecker> model_checker; // Class for synthesis
	std::unique_ptr<ColoringAnalyzer> analyzer; // Class for analysis
	std::unique_ptr<WitnessSearcher> searcher; // Class to build wintesses

	// Overall statistics
	std::size_t total_colors;
	std::size_t shortest_path_lenght; 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SYNTHESIS CONTROL
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	/**
	 * Setup everything that needs it for computation in this round
	 */
	void doPreparation() {
		// Output round number
		output->outputRoundNum();
		// Pass information about round (necessary for setup)
		analyzer->strartNewRound(split_manager->getRoundRange());	
	}

	/**
	 * Store results that have not been stored yet and finalize the round where needed
	 */
	void doConclusion() {
		total_colors += count(analyzer->getUnion());
		// Output what has been synthetized (colors, witnesses)
		output->outputRound(shortest_path_lenght);
	}
	
	/**
	 * Entry point of the parameter synthesis. 
	 * In the first part, all states are colored with parameters that are transitive from some initial state. At the end, all final states are stored together with their color.
	 * In the second part, for all final states the strucutre is reset and colores are distributed from the state. After coloring the resulting color of the state is stored.
	 *
	 * @param witness_use - how to handle witnesses
	 */
	void doComputation() {
		// Basic (initial) coloring
		shortest_path_lenght = colorProduct(user_options.witnesses());

		// Store colored final vertices
		std::vector<Coloring> final_states = std::move(storage.getColor(product.getFinalStates()));
		// Get the actuall results by cycle detection for each final vertex
		for (auto final_it = final_states.begin(); final_it != final_states.end(); final_it++) {
			// For general property, there must be new coloring for each final state!
 			if (!none(final_it->second) && !user_options.timeSerie())
				detectCycle(*final_it);

			// Store results from this final state
			analyzer->storeResults(Coloring(final_it->first, storage.getColor(final_it->first)));
		}
	}

	/**
	 * Do initial coloring of states - start from initial states and distribute all the transitible parameters.
	 *
	 * @param witness_use - how to handle witnesses
	 */
	const std::size_t colorProduct(const WitnessUse wits_use) {
		// Assure emptyness
		storage.reset();

		// Get initial coloring
		Parameters starting;
		if(coloring_parser.input())
			starting = coloring_parser.getColors()[split_manager->getRoundNum()];
		else
			starting = split_manager->createStartingParameters();

		// Set all the initial states to initial color
		for (auto init_it = product.getInitialStates().begin(); init_it != product.getInitialStates().end(); init_it++) 
			storage.update(starting, *init_it);

		// Schedule all initial states for updates
		std::set<StateID> updates(product.getInitialStates().begin(), product.getInitialStates().end());

		// Start coloring procedure
		return model_checker->startColoring(starting, updates, split_manager->getRoundRange(), wits_use);
	}

	/**
	 * For each final state that has at least one parameter assigned, start cycle detection.
	 *
	 * @param init_coloring	reference to the final state that starts the coloring search with its parameters
	 */
	void detectCycle(const Coloring & init_coloring) {
		// Assure emptyness
		storage.reset();

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
	SynthesisManager(const ProductStructure & _product, ColorStorage & _storage)
		            : structure(_product.getKS()), storage(_storage), automaton(_product.getBA()), product(_product) {
		// Create classes that help with the synthesis
		split_manager.reset(new SplitManager(product.getFunc().getParametersCount()));
		model_checker.reset(new ModelChecker(product, storage));
		analyzer.reset(new ColoringAnalyzer(product));
		searcher.reset(new WitnessSearcher(*analyzer, storage, product));
		output.reset(new OutputManager(*analyzer, product, *split_manager, *searcher));

		total_colors = 0;
		shortest_path_lenght = ~0;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SYNTHESIS ENTRY
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

		// Output final numbers
		output->outputSummary(total_colors);
	}
};

#endif