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
#include "../results/output_manager.hpp"
#include "../results/product_analyzer.hpp"
#include "../results/witness_searcher.hpp"

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
	std::unique_ptr<OutputManager> output; // Class for output
	std::unique_ptr<ModelChecker> model_checker; // Class for synthesis
	std::unique_ptr<ProductAnalyzer> analyzer; // Class for analysis
	std::unique_ptr<ResultStorage> results; // Class to store results
	std::unique_ptr<WitnessSearcher> searcher; // Class to build wintesses
	std::unique_ptr<WitnessStorage> witnesses; // Class to store witnesses

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SYNTHESIS CONTROL
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	/**
	 * Setup everything that needs it for computation in this round
	 */
	void doPreparation() {
		// Output round number
		output->outputRoundNum();
		// Pass information about round (necessary for setup of those classes)
		model_checker->setRange(split_manager->getRoundRange());
		analyzer->setRange(split_manager->getRoundRange());	
	}

	/**
	 * Compute data that are requested
	 */
	void doComputation() {
		// Do the synthesis, storing all feasible parameters
		synthetizeParameters(none_wit);
		// Compute witnesses
		if (user_options.witnesses() && count(results->getAllParameters()))
			synthetizeParameters(all_wit);
	}

	/**
	 * Store results that have not been stored yet and finalize the round where needed
	 */
	void doConclusion() {
		// Output what has been synthetized (colors, witnesses)
		output->outputData();
		// Do finishing changes and reset results in this round
		results->finishRound();
		// Do finishing changes and reset witnesses in this round
		witnesses->finishRound();
	}
	
	/**
	 * Entry point of the parameter synthesis. 
	 * In the first part, all states are colored with parameters that are transitive from some initial state. At the end, all final states are stored together with their color.
	 * In the second part, for all final states the strucutre is reset and colores are distributed from the state. After coloring the resulting color of the state is stored.
	 *
	 * @param witness_use - how to handle witnesses
	 */
	void synthetizeParameters(WitnessUse witness_use) {
		// FIRST PART: // 
		// Basic (initial) coloring
		colorProduct(witness_use);
		// Store witnesses for basic coloring
		if (witness_use != none_wit)
			searcher->storeWitnesses(product.getFinals(), true);

		// SECOND PART: //
		// Store colored final vertices
		std::vector<Coloring> final_states = std::move(analyzer->getFinalColoring());
		// Get the actuall results by cycle detection for each final vertex
		for (auto final_it = final_states.begin(); final_it != final_states.end(); final_it++) {
			// Restart the coloring using coloring of the first final state if there are at least some parameters
			if (!none(final_it->second) && !user_options.timeSerie())
				detectCycle(*final_it);
			// For the round without witnesses, store only coloring, for the other, store only witnesses
			if (witness_use == none_wit) 
				analyzer->storeResults(final_it->first, user_options.coloring());
			else if (!user_options.timeSerie())
				searcher->storeWitnesses(final_it->first, false);
		}
	}

	/**
	 * Do initial coloring of states - start from initial states and distribute all the transitible parameters.
	 *
	 * @param witness_use - how to handle witnesses
	 */
	void colorProduct(WitnessUse witness_use) {
		// Assure emptyness
		product.resetProduct();
		// Pass the information about witness usage
		model_checker->setWitnessUse(witness_use);

		// Color each initial state with current parameters (All for the first round)
		if (witness_use == none_wit)
			product.colorInitials(split_manager->createStartingParameters());
		else 
			product.colorInitials(results->getAllParameters());
		// Schedule all initial states for updates
		model_checker->setUpdates(std::move(product.getInitialUpdates()));

		// Start coloring procedure
		model_checker->doColoring();
	}

	/**
	 * For each final state that has at least one parameter assigned, start cycle detection.
	 *
	 * @param init_coloring	reference to the final state that starts the coloring search with its parameters
	 */
	void detectCycle(const Coloring & init_coloring) {
		// Assure emptyness
		product.resetProduct();
		// Sechedule nothing for updates (will be done during transfer in the next step)
		model_checker->setUpdates();

		// Send updates from the initial state
		model_checker->transferUpdates(init_coloring.first, init_coloring.second);

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
		split_manager.reset(new SplitManager(product.getFunc().getParametersCount()));
		model_checker.reset(new ModelChecker(product));
		results.reset(new ResultStorage(product));
		analyzer.reset(new ProductAnalyzer(product, *results));
		witnesses.reset(new WitnessStorage(product));
		searcher.reset(new WitnessSearcher(product, *witnesses));
		output.reset(new OutputManager(product, *split_manager, *results, *witnesses));
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
		output->outputSummary();
	}
};

#endif