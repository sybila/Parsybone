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
	// Provided with constructor
	const ParametrizedStructure & structure; // Stores info about KS states
	const AutomatonStructure & automaton; // Stores info about BA states

	// Filled with computed data	
	Results & results; 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTING FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	

	/**
	 * Pick final states from basic coloring and store them with their parameters
	 *
	 * @param product	colored product
	 *
	 * @return queue with all colorings of final states
	 */
	std::queue<Coloring> storeFinalStates(std::unique_ptr<ProductStructure> & product) {
		std::queue<Coloring> final_states; // States colored in basic coloring
		std::size_t state_num = 0; // ID of the state in product
		// List throught product states that are final
		for (std::size_t ba_state_num = 0; ba_state_num < automaton.getStatesCount(); ba_state_num++) {
			// Skip non-final states
			if (!automaton.isFinal(ba_state_num)) 
				continue;
			// For each final state of the product store the coloring
			for (std::size_t ks_state_num = 0; ks_state_num < structure.getStatesCount(); ks_state_num++) {
				// Compute the product state position
				state_num = ks_state_num * automaton.getStatesCount() + ba_state_num;
				// If there is a coloring, store
				if (!product->isEmpty(state_num)) 
					final_states.push(std::make_pair(state_num, product->getParameters(state_num)));
			}
		}
		return final_states;
	}

	/**
	 * Remove non-transitive parameters.
	 *
	 * @param target_param	paramters that are passed through the transition
	 * @param step_size	how many parameters share the same value for given function
	 * @param transitive_values	mask of all values from which those that have false are non-transitive
	 */
	void passParameters(Parameters & target_param, const std::size_t step_size, const std::vector<bool> & transitive_values) {
		// List through all the paramters
		for (std::size_t param_num = 0; param_num < target_param.size(); param_num) {
			// List through ALL the target values
			for (std::size_t value = 0; value < transitive_values.size(); value++) {
				// Remove nont-transitive
				if (transitive_values[value] != true) {
					for (std::size_t substep = 0; substep < step_size; substep++) {
						target_param[param_num++] = 0;
					}
				}
				// Others skip
				else {
					param_num += step_size;
				}
			}
		}
	}


	/**
	 * Update target of the transition with transitible parameters
	 *
	 * @param product	product to color
	 * @param updates	set that will hold ID of newly updated state
	 * @param parameters	parameters that will be distributed
	 * Positioning:
	 * @param KS_transition_num		index of transition for this KS state
	 * @param source_KS_state	this KS state - target_KS is obtained from the transition index
	 * @param target_BA_state	target state of the BA
	 */
	void pushUpdate(std::unique_ptr<ProductStructure> & product, std::set<std::size_t> & updates, const Parameters & parameters, const std::size_t KS_transition_num, const std::size_t source_KS_state, const std::size_t target_BA_state) {
		Parameters passed = parameters;
		passParameters(passed, structure.getStepSize(source_KS_state, KS_transition_num), structure.getTransitive(source_KS_state, KS_transition_num));
		// If there is at least some update
		if (!passed.none()) {
			// Compute and update target state
			std::size_t target_state = structure.getTargetID(source_KS_state,KS_transition_num) * automaton.getStatesCount() + target_BA_state;
			if (product->updateParameters(passed, target_state))
				updates.insert(target_state);
		}
	}

	/**
	 * From the source distribute its parameters and newly colored neighbours shedule for update.
	 *
	 * @param souce_state	ID of the source state in the producte
	 * @param parameters	parameters that will be distributed
	 * @param product	product to color
	 * @param updates	set containing IDs of states that are scheduled for update
	 */
	void transferUpdates(std::unique_ptr<ProductStructure> & product, std::set<std::size_t> & updates, const std::size_t source_state, const Parameters & parameters) {
		// From ID of product state compute IDs of KS and BA states
		std::size_t source_BA_state = source_state % automaton.getStatesCount();
		std::size_t source_KS_state = source_state / automaton.getStatesCount();

		// For each feasible transition of BA store its ID in the queue
		std::queue<std::size_t> transitible_ba;
		for (std::size_t transition_num = automaton.getBeginIndex(source_BA_state); transition_num < automaton.getBeginIndex(source_BA_state + 1); transition_num++) {
			if (automaton.isTransitionFeasible(transition_num, structure.getStateLevels(source_KS_state)))
				transitible_ba.push(automaton.getTarget(transition_num));
		}

		// Push updates for each BA transition times each KS transition
		while (!transitible_ba.empty()) {
			std::size_t target_BA_state = transitible_ba.front();
			// Combine BA transition with all KS transitions and update those
			for (std::size_t KS_transition_num = 0; KS_transition_num < structure.getTransitionsCount(source_KS_state); KS_transition_num++) {
				pushUpdate(product, updates, parameters, KS_transition_num, source_KS_state, target_BA_state);
			}
			transitible_ba.pop();
		}
	}

	/**
	 * Distribute updates and store the new ones.
	 *
	 * @param product	product to color
	 * @param updates	set containing IDs of states that are scheduled for update
	 */
	void doColoring(std::set<std::size_t> & updates, std::unique_ptr<ProductStructure> & product) {
		while (!updates.empty()) {
			std::size_t state_num = *updates.begin();
			transferUpdates(product, updates, state_num, product->getParameters(state_num));
			updates.erase(state_num);
		}
	}

	/**
	 * For each final state that has at least one parameter assigned, start cycle detection.
	 *
	 * @param product	product to color
	 * @param init_coloring	reference to the final state that starts the coloring search with its parameters
	 */
	void detectCycle(const Coloring & init_coloring, std::unique_ptr<ProductStructure> & product) {
		// Assure the product is empty
		product->reset();
		std::set<std::size_t> updates;
		// Send updates from the initial state
		transferUpdates(product, updates, init_coloring.first, init_coloring.second);

		doColoring(updates, product);
	}

	/**
	 * Do initial coloring of states - start from initial states and distribute all the transitible parameters.
	 *
	 * @param product	product to color
	 */
	void colorProduct(std::unique_ptr<ProductStructure> & product) {
		std::set<std::size_t> updates;

		// Fill all the initial states with all parameters and schedule them to update
		Parameters all_parameters(product->getParametersCount());
		all_parameters = ~all_parameters; // Set all to one
		// For each initial state, store all the parameters and schedule for the update
		for (std::size_t state_num = 0; state_num < product->getStatesCount(); state_num++) {
			if (state_num % automaton.getStatesCount() == 0) {
					updates.insert(state_num);
					product->updateParameters(all_parameters, state_num);
			}
		}
		
		doColoring(updates, product);
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
		: structure(_structure), automaton(_automaton), results(_results) {	}

	/**
	 * Entry point of the parameter synthesis. 
	 * In the first round, all states are colored with parameters that are transitive from some initial state. At the end, all final states are stored together with their color.
	 * In the second round, for all final states the strucutre is reset and colores are distributed from the state. After coloring the resulting color of the state is stored.
	 */
	void syntetizeParameters() {
		// Create a new structure
		std::unique_ptr<ProductStructure> product(new ProductStructure(structure.getStatesCount() * automaton.getStatesCount() , structure.getParametersCount()));
		// Basic coloring
		colorProduct(product);
		// Store colored final vertices
		std::queue<Coloring> final_states = std::move(storeFinalStates(product));

		// Get the actuall results by cycle detection
		while (!final_states.empty()) {
			// Restart the coloring using coloring of the first final state
			detectCycle(final_states.front(), product);

			// Store the result
			const std::size_t state_num = final_states.front().first;
			if (!product->getParameters(state_num).none())
				results.addColoredState(Coloring(state_num, product->getParameters(state_num)));

			// Remove the state
			final_states.pop();
		}
	}
};


#endif