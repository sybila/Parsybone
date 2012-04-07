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

#include "../auxiliary/output_streamer.hpp"
#include "../auxiliary/time_manager.hpp"
#include "parameters_functions.hpp"
#include "model_checker.hpp"
#include "../results/result_storage.hpp"
#include "../results/output_manager.hpp"
#include "../results/product_analyzer.hpp"


class SynthesisManager {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Provided
	const ParametrizedStructure & structure; // Stores info about KS states
	const AutomatonStructure & automaton; // Stores info about BA states
	ProductStructure & product; // Product to compute on

	// Created within the constructor
	std::unique_ptr<SplitManager> split_manager; // Control of independent rounds
	std::unique_ptr<ModelChecker> model_checker; // Class for synthesis
	std::unique_ptr<ResultStorage> results; // Class to store results
	std::unique_ptr<ProductAnalyzer> analyzer; // Class for analysis
	std::unique_ptr<OutputManager> output; // Class for output

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SYNTHESIS CONTROL
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	/**
	 * Function that does all the coloring. This part only covers iterating through subparts.
	 */
	void cycleRounds() {
		// Cycle through the rounds
		for (;split_manager->valid(); split_manager->increaseRound()) {
			output->outputRound();
			model_checker->setRange(split_manager->getRoundRange());
			analyzer->setRange(split_manager->getRoundRange());
			syntetizeParameters();
			output->outputColors();
			results->finishRound();
		}
		// After last round, 
		output_streamer.output(verbose, "", OutputStreamer::rewrite_ln | OutputStreamer::no_newl);
	}

	/**
	 * Entry point of the parameter synthesis. 
	 * In the first part, all states are colored with parameters that are transitive from some initial state. At the end, all final states are stored together with their color.
	 * In the second part, for all final states the strucutre is reset and colores are distributed from the state. After coloring the resulting color of the state is stored.
	 */
	void syntetizeParameters() {
		// Basic (initial) coloring
		colorProduct();
		// Store colored final vertices
		std::vector<Coloring> final_states = std::move(product.storeFinalStates());

		// Get the actuall results by cycle detection for each final vertex
		for (std::size_t state_index = 0; state_index < final_states.size(); state_index++) {
			// Reference a final state for this round
			auto & final = final_states[state_index];
			// Restart the coloring using coloring of the first final state if there are at least some parameters
			if (!none(final.second))
				detectCycle(final);
			analyzer->storeResults(final.first, product.getParameters(final.first));
		}
	}

	/**
	 * For each final state that has at least one parameter assigned, start cycle detection.
	 *
	 * @param init_coloring	reference to the final state that starts the coloring search with its parameters
	 */
	void detectCycle(const Coloring & init_coloring) {
		// Assure emptyness
		product.resetProduct();
		model_checker->setUpdates();
		// Send updates from the initial state
		model_checker->transferUpdates(init_coloring.first, init_coloring.second);
		// Start coloring procedure
		model_checker->doColoring(init_coloring.first, init_coloring.second);
	}

	/**
	 * Do initial coloring of states - start from initial states and distribute all the transitible parameters.
	 */
	void colorProduct() {
		// Assure emptyness
		product.resetProduct();
		// For each initial state, store all the parameters and schedule for the update
		product.colorInitials(split_manager->createStartingParameters());
		model_checker->setUpdates(std::move(product.getInitialUpdates()));
		// Start coloring procedure
		model_checker->doColoring();
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
	SynthesisManager(ProductStructure & _product)
		            : structure(_product.getKS()), automaton(_product.getBA()), product(_product) {
		// Create classes that help with the synthesis
		split_manager.reset(new SplitManager(structure.getParametersCount()));
		model_checker.reset(new ModelChecker(product));
		results.reset(new ResultStorage(product));
		analyzer.reset(new ProductAnalyzer(product, *results));
		output.reset(new OutputManager(product, *split_manager, *results));
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SYNTHESIS ENTRY
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Main synthesis function that iterates through all the rounds of the synthesis
	 */
	void doSynthesis() {
		// Do computation
		time_manager.startClock("coloring runtime");
		
		cycleRounds();

		output->outputSum();

		time_manager.ouputClock("coloring runtime");
	}
};

#endif