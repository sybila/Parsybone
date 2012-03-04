/*
 * Copyright (C) 2012 - Adam Streck
 *
 * This file is part of PoSeIDoN (Parameter Synthetizer for Discrete Networks) verification tool
 *
 * Poseidon is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Poseidon is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 *
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno.
 * See http://sybila.fi.muni.cz/ .
 */

// #define DEBUG_OUTPUT

#ifndef POSEIDON_MODEL_CHECKER_INCLUDED
#define POSEIDON_MODEL_CHECKER_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ModelChecker class does the parameter synthesis by at first reachability coloring and the cycle detection from all the vertices that got colored.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdexcept>
#include <queue>

#include "../reforging/parametrized_structure.hpp"
#include "../reforging/automaton_structure.hpp"
#include "product_structure.hpp"
#include "../results/results.hpp"

class ModelChecker {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::queue<Coloring> final_states; // States colored in basic coloring
	
	const ParametrizedStructure & structure; // Stores info about KS states
	const AutomatonStructure & automaton; // Stores info about BA states
	std::unique_ptr<ProductStructure> product; // Used for coloring
	Results & results; // Filled with computed data

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTING FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	/**
	 * Color initial states (as for initial states of the BA)
	 */
	void colorStates() {
		std::queue<Coloring> updates;

		Parameters all_parameters(product->getParametersCount());
		all_parameters = ~all_parameters;
		for (std::size_t state_num = 0; state_num < product->getStatesCount(); state_num++) {
			// Starting Buchi state
			if (state_num % automaton.getStatesCount() == 0) {

				Coloring init_coloring(state_num, all_parameters);
				updates.push(init_coloring);
			}
		}

		while (!updates.empty()) {
#ifdef DEBUG_OUTPUT
				std::cout << "  New BFS level.\n";
#endif
			if (product->updateParameters(updates.front().second, updates.front().first))
				transferUpdates(updates, updates.front());
			updates.pop();
		}
	}

	/**
	 * Pick final states from basic coloring and store them with their parameters
	 */
	void storeFinalStates() {
		std::size_t state_num = 0;
		// List throught product states that are final
		for (std::size_t ba_state_num = 0; ba_state_num < automaton.getStatesCount(); ba_state_num++) {
			if (!automaton.isFinal(ba_state_num)) 
				continue;
			// For each final state of the product store the coloring
			for (std::size_t ks_state_num = 0; ks_state_num < structure.getStatesCount(); ks_state_num++) {
				// Compute the product state position
				state_num = ks_state_num * automaton.getStatesCount() + ba_state_num;
				// If there is a coloring, store
				if (product->isEmpty(state_num)) 
					final_states.push(std::make_pair(state_num, product->getParameters(state_num)));
			}
		}
	}

	/**
	 * Remove non-transitive parameters.
	 */
	void passParameters(Parameters & target_param, std::size_t step_size, const std::vector<bool> & transitive_values) {
		for (std::size_t param_num = 0; param_num < target_param.size(); param_num) {
			for (std::size_t value = 0; value < transitive_values.size(); value++) {
				if (transitive_values[value] != true) {
					for (std::size_t substep = 0; substep < step_size; substep++) {
						target_param[param_num++] = 0;
					}
				}
				else {
					param_num += step_size;
				}
			}
		}
	}


	/**
	 * Create a new update from the transition
	 */
	void pushUpdate(const Coloring & source, const std::size_t transition_index, const std::size_t KS_ID, const std::size_t BA_ID, std::queue<Coloring> & updates) {
		Parameters passed = source.second;
		passParameters(passed, structure.getStepSize(KS_ID, transition_index), structure.getTransitive(KS_ID, transition_index));
		// If there is at least some update
		if (!passed.none()) {
			updates.push(Coloring(structure.getTargetID(KS_ID,transition_index) * automaton.getStatesCount() + BA_ID, passed));
#ifdef DEBUG_OUTPUT
			std::cout << "    UPDATE: State " << structure.getTargetID(KS_ID,transition_index) * automaton.getStatesCount() + BA_ID;
			std::cout << ", parameters ";
			for (std::size_t param = 0; param < passed.size(); param++) {
				std::cout << passed[param];
			}
			std::cout << ".\n";
#endif
		}
	}

	void transferUpdates(std::queue<Coloring> & updates, Coloring & source) {
		// Compute position in product
		std::size_t automaton_state = source.first % automaton.getStatesCount();
		std::size_t structure_state = source.first / automaton.getStatesCount();

		// Get the feasible transitions numbers
		std::queue<std::size_t> transitible_ba;
		for (std::size_t transition_num = automaton.getBeginIndex(automaton_state); transition_num < automaton.getBeginIndex(automaton_state + 1); transition_num++) {
			if (automaton.isTransitionFeasible(transition_num, structure.getStateLevels(structure_state)))
				transitible_ba.push(automaton.getTarget(transition_num));
		}

		// Push updates for each BA transition times each KS transition
		while (!transitible_ba.empty()) {
			std::size_t ba_state_num = transitible_ba.front();
			for (std::size_t transition_num = 0; transition_num < structure.getTransitionsCount(structure_state); transition_num++) {
				pushUpdate(source, transition_num, structure_state, ba_state_num, updates);
			}
			transitible_ba.pop();
		}
	}

	/**
	 * @param init_coloring	reference to the final state that starts the coloring search with its parameters
	 *
	 * For each final state that has at least one parameter assigned, start cycle detection.
	 */
	void floodColoring(const Coloring & init_coloring) {
		std::queue<Coloring> updates;
		updates.push(init_coloring);
		transferUpdates(updates, updates.front());
		updates.pop();
		while (!updates.empty()) {
#ifdef DEBUG_OUTPUT
				std::cout << "  New BFS level.\n";
#endif
			if (product->updateParameters(updates.front().second, updates.front().first))
				transferUpdates(updates, updates.front());
			updates.pop();
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTING FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ModelChecker(const ModelChecker & other);            // Forbidden copy constructor.
	ModelChecker& operator=(const ModelChecker & other); // Forbidden assignment operator.

public:
	/**
	 * Constructor, passes the data
	 */
	ModelChecker(const ParametrizedStructure & _structure, const AutomatonStructure & _automaton, Results & _results) 
		: structure(_structure), automaton(_automaton), results(_results) {
		product.reset(new ProductStructure(structure.getStatesCount() * automaton.getStatesCount() , structure.getParametersCount()));
	}

	/**
	 * Entry point of the parameter synthesis
	 */
	void syntetizeParameters() {
		// Basic coloring
		colorStates();
		// Store colored final vertices
		storeFinalStates();
		// Get the actuall results by cycle detection
		while (!final_states.empty()) {
			// Restart the coloring using coloring of the first final state
			const Coloring & final_state = final_states.front();
			product->reset();
#ifdef DEBUG_OUTPUT
				std::cout << "Cycling from the state: " << final_state.first << " with parameters: " << final_state.second << ":\n";
#endif
			floodColoring(final_state);
			// Store the result
			const std::size_t state_num = final_state.first;
			if (!product->getParameters(state_num).none())
				results.addColoredState(Coloring(state_num, product->getParameters(state_num)));
			// Remove the state
			final_states.pop();
		}
	}
};


#endif