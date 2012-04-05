/*
 * Copyright (C) 2012 - Adam Streck
 *
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 *
 * ParSyBoNe is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 *
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno.
 * See http://sybila.fi.muni.cz/ .
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
#include "../reforging/parametrized_structure.hpp"
#include "../reforging/automaton_structure.hpp"
#include "../reforging/product.hpp"
#include "../reforging/functions_structure.hpp"
#include "../results/results.hpp"
#include "../results/output_manager.hpp"


class SynthesisManager {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const UserOptions & user_options; // Values provided as parameters
	const ParametrizedStructure & structure; // Stores info about KS states
	const AutomatonStructure & automaton; // Stores info about BA states
	const FunctionsStructure & functions;
	Product & product;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SYNTHESIS CONTROL
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Function that does all the coloring. This part only covers iterating through subparts.
	 */
	void cycleRounds(ModelChecker & model_checker, SplitManager & split_manager, Results & results) {
		// Cycle through the rounds
		while (true) {
			syntetizeParameters(model_checker, split_manager, results);
			// Increase rounds until the last one is reached - then end the loop
			if (!split_manager.lastRound()) {
				split_manager.increaseRound();
			}
			else {
				// Remove the rounds line 
				output_streamer.output(verbose, "", OutputStreamer::rewrite_ln | OutputStreamer::no_newl);
				break;
			}
		}	
	}

	/**
	 * Entry point of the parameter synthesis. 
	 * In the first part, all states are colored with parameters that are transitive from some initial state. At the end, all final states are stored together with their color.
	 * In the second part, for all final states the strucutre is reset and colores are distributed from the state. After coloring the resulting color of the state is stored.
	 */
	void syntetizeParameters(ModelChecker & model_checker, SplitManager & split_manager, Results & results) {
		split_manager.outputRound();
		// Basic coloring
		colorProduct(model_checker, split_manager, results);
		// Store colored final vertices
		std::queue<Coloring> final_states = std::move(product.storeFinalStates());

		// Get the actuall results by cycle detection for each final vertex
		for (std::size_t state_index = 0; !final_states.empty(); state_index++) {
			// Restart the coloring using coloring of the first final state if there are at least some parameters
			if (!none(final_states.front().second))
				detectCycle(final_states.front(), model_checker, split_manager, results);
			// Store the result
			results.addResult(state_index, product.getParameters(final_states.front().first));
			// Remove the state
			final_states.pop();
		}
	}

	/**
	 * For each final state that has at least one parameter assigned, start cycle detection.
	 *
	 * @param init_coloring	reference to the final state that starts the coloring search with its parameters
	 */
	void detectCycle(const Coloring & init_coloring, ModelChecker & model_checker, SplitManager & split_manager, Results & results) {
		// Assure emptyness
		product.resetProduct();
		model_checker.setUpdates();
		// Send updates from the initial state
		model_checker.transferUpdates(init_coloring.first, init_coloring.second);
		// Start coloring procedure
		model_checker.doAcceptingColoring(init_coloring.first, init_coloring.second);
	}

	/**
	 * Do initial coloring of states - start from initial states and distribute all the transitible parameters.
	 */
	void colorProduct(ModelChecker & model_checker, SplitManager & split_manager, Results & results) {
		// Assure emptyness
		product.resetProduct();
		// For each initial state, store all the parameters and schedule for the update
		model_checker.setUpdates(product.colorInitials(split_manager.createStartingParameters()));
		// Start coloring procedure
		model_checker.doColoring();
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREATION
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	SynthesisManager(const SynthesisManager & other);            // Forbidden copy constructor.
	SynthesisManager& operator=(const SynthesisManager & other); // Forbidden assignment operator.

public:
	SynthesisManager(const UserOptions & _user_options, const FunctionsStructure & _functions, Product & _product)
		            : user_options(_user_options), functions(_functions), structure(_product.getKS()), automaton(_product.getBA()), product(_product) { }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SYNTHESIS ENTRY
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Main synthesis function that iterates through all the rounds of the synthesis
	 */
	void doSynthesis() {
		// Pre-create data
		SplitManager split_manager(user_options.process_number, user_options.processes_count, structure.getParametersCount());
		Results results(product, split_manager);
		ModelChecker model_checker(user_options, split_manager, results, product);
		
		time_manager.startClock("coloring runtime");
		
		cycleRounds(model_checker, split_manager, results);

		time_manager.ouputClock("coloring runtime");


		/*while (true) {
			// Synthetize round
			doRound();
			// Get colors

			// Synthetize witnesses

			// Ouput params

			// Increase round
			if (!split_manager.lastRound())
				split_manager.increaseRound();
			else 
				break;
		}*/

		// Do output
		OutputManager output_manager(user_options, results, functions, split_manager);
		output_manager.basicOutput();
	}
};

#endif