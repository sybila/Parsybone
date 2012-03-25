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

// #define DEBUG_OUTPUT

#ifndef PARSYBONE_MODEL_CHECKER_INCLUDED
#define PARSYBONE_MODEL_CHECKER_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ModelChecker class does the parameter synthesis by at first reachability coloring and the cycle detection from all the vertices that got colored.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdexcept>
#include <queue>
#include <cmath>

#include "../reforging/parametrized_structure.hpp"
#include "../reforging/automaton_structure.hpp"
#include "product_structure.hpp"
#include "parameters_functions.hpp"
#include "../results/results.hpp"

class ModelChecker {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	// Provided with constructor
	const UserOptions & user_options;
	const ParametrizedStructure & structure; // Stores info about KS states
	const AutomatonStructure & automaton; // Stores info about BA states
	SplitManager split_manager; // Copy of a split manager just for the checking

	// Used for computation
	std::unique_ptr<ProductStructure> product;
	std::set<std::size_t> updates;

	// Filled with computed data	
	Results & results; 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA STORING FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	/**
	 * Pick final states from basic coloring and store them with their parameters
	 *
	 * @return queue with all colorings of final states
	 */
	std::queue<Coloring> storeFinalStates() {
		// States colored in basic coloring
		std::queue<Coloring> final_states; 

		// List throught product states that are final
		for (std::size_t ba_state_num = 0; ba_state_num < automaton.getStatesCount(); ba_state_num++) {
			if (automaton.isFinal(ba_state_num)) {
				// For each final state of the product store the coloring
				for (std::size_t ks_state_num = 0; ks_state_num < structure.getStatesCount(); ks_state_num++) {
					// Compute the product state position and store it with its parameters
					std::size_t state_num = ks_state_num * automaton.getStatesCount() + ba_state_num;
					final_states.push(std::make_pair(state_num, product->getParameters(state_num)));
				}
			}
		}

		// Return final vertices with their positions
		return final_states;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTING FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	/**
	 * Remove non-transitive parameters.
	 *
	 * @param target_param	paramters that are passed through the transition
	 * @param step_size	how many parameters share the same value for given function
	 * @param transitive_values	mask of all values from which those that have false are non-transitive
	 */
	void passParameters(Parameters & target_param, const std::size_t step_size, const std::vector<bool> & transitive_values) {
		// Number of the first parameter
		std::size_t param_num = split_manager.getRoundRange().first;
		// First value might not bet 0 - get it from current parameter position
		std::size_t value_num = (param_num / step_size) % transitive_values.size();
		// As well current value step might not be the first one, it is also necessary to get it from current parameter position
		std::size_t substep = param_num % step_size;
		// Mask that will be created
		Parameters temporary = 0;

		// List through all the paramters
		while (param_num < split_manager.getRoundRange().second) {
			// List through ALL the target values
			for (; value_num < transitive_values.size(); value_num++) {
				// For the current value add 1s if its color is present and 0s if not within the size of the step
				while (substep++ < step_size) {
					// Move from previous round
					temporary <<= 1;
					// Add 1 if transitive
					if (transitive_values[value_num]) 
						temporary |= 1;
					// Check if there is necessity for another round
					if(++param_num == split_manager.getRoundRange().second) {
						target_param &= temporary;
						return;
					}
				}
				// Reset steps for the value
				substep = 0;
			}
			// Reset the value
			value_num = 0;
		}

		// Create interection of source parameters and transition parameters

	}

	/**
	 * Update target of the transition with transitible parameters
	 *
	 * @param updates	set that will hold ID of newly updated state
	 * @param parameters	parameters that will be distributed
	 * Positioning:
	 * @param KS_transition_num		index of transition for this KS state
	 * @param source_KS_state	this KS state - target_KS is obtained from the transition index
	 * @param target_BA_state	target state of the BA
	 */
	void updateTarget(Parameters parameters, const std::size_t KS_transition_num,  const std::size_t source_KS_state, const std::size_t target_BA_state) {
		// From an update strip all the parameters that can not pass through the transition
		passParameters(parameters, structure.getStepSize(source_KS_state, KS_transition_num), structure.getTransitive(source_KS_state, KS_transition_num));
		// If some parameters get passed
		if (!none(parameters)) {
			// Compute and update target state of product
			std::size_t target_state = structure.getTargetID(source_KS_state,KS_transition_num) * automaton.getStatesCount() + target_BA_state;
			if (product->updateParameters(parameters, target_state))
				updates.insert(target_state);
		}
	}

	/**
	 * From the source distribute its parameters and newly colored neighbours shedule for update.
	 *
	 * @param updates	set containing IDs of states that are scheduled for update
	 * @param souce_state	ID of the source state in the producte
	 * @param parameters	parameters that will be distributed
	 */
	void transferUpdates(const std::size_t source_state, const Parameters parameters) {
		// From ID of product state compute IDs of KS and BA states
		std::size_t source_BA_state = source_state % automaton.getStatesCount();
		std::size_t source_KS_state = source_state / automaton.getStatesCount();

		// For each feasible transition of BA store its ID in the queue
		std::queue<std::size_t> transitible_ba;
		for (std::size_t transition_num = automaton.getBeginIndex(source_BA_state); transition_num < automaton.getBeginIndex(source_BA_state + 1); transition_num++) {
			// Check the transitibility
			if (automaton.isTransitionFeasible(transition_num, structure.getStateLevels(source_KS_state)))
				transitible_ba.push(automaton.getTarget(transition_num));
		}

		// Push updates for each BA transition times each KS transition
		for (std::size_t target_BA_state = (transitible_ba.empty() ? 0 : transitible_ba.front()); !transitible_ba.empty(); transitible_ba.pop()) {
			// Combine BA transition with all KS transitions and update those
			for (std::size_t KS_transition_num = 0; KS_transition_num < structure.getTransitionsCount(source_KS_state); KS_transition_num++) {
				updateTarget(parameters, KS_transition_num, source_KS_state, target_BA_state);
			}
		}
	}

	/**
	 * Distribute updates and store the new ones.
	 *
	 * @param updates	set containing IDs of states that are scheduled for update
	 */
	void doColoring() {
		// While there are updates, pass them to succesing vertices
		while (!updates.empty()) {
			// Heuristics for an update with approximatelly maximal number of bits set
			
			std::size_t state_num = 0; std::size_t current_par = 0;
			for (auto update_it = updates.begin(); update_it != updates.end(); update_it++) {
				if (product->getParameters(*update_it) == (current_par | product->getParameters(*update_it))) {
					state_num = *update_it;
					current_par = product->getParameters(state_num);
				} 
			}
			// Pass data from updated vertex to its succesors
			transferUpdates(state_num, product->getParameters(state_num));
			// Erase completed update from the set
			updates.erase(state_num);
		}
	}

	/**
	 * For each final state that has at least one parameter assigned, start cycle detection.
	 *
	 * @param init_coloring	reference to the final state that starts the coloring search with its parameters
	 */
	void detectCycle(const Coloring & init_coloring) {
		// Assure emptyness
		product->reset();
		updates.clear();
		// Send updates from the initial state
		transferUpdates( init_coloring.first, init_coloring.second);
		// Start coloring procedure
		doColoring();
	}

	/**
	 * Do initial coloring of states - start from initial states and distribute all the transitible parameters.
	 */
	void colorProduct() {
		// Assure emptyness
		product->reset();
		updates.clear();
		// For each initial state, store all the parameters and schedule for the update
		for (std::size_t state_num = 0; state_num < product->getStatesCount(); state_num++) {
			// Use only those states built from initial states of the BA
			if (state_num % automaton.getStatesCount() == 0) {
				// Schedule for an update
				updates.insert(state_num);
				// Get all the parameters for current round and pass them to structure
				product->updateParameters(split_manager.createStartingParameters(), state_num);
			}
		}
		// Start coloring procedure
		doColoring();
	}

	/**
	 * Entry point of the parameter synthesis. 
	 * In the first part, all states are colored with parameters that are transitive from some initial state. At the end, all final states are stored together with their color.
	 * In the second part, for all final states the strucutre is reset and colores are distributed from the state. After coloring the resulting color of the state is stored.
	 */
	void syntetizeParameters() {
		// Basic coloring
		colorProduct();
		// Store colored final vertices
		std::queue<Coloring> final_states = std::move(storeFinalStates());

		// Get the actuall results by cycle detection for each final vertex
		for (std::size_t state_index = 0; !final_states.empty(); state_index++) {
			// Restart the coloring using coloring of the first final state if there are at least some parameters
			//if (!none(final_states.front().second))
			//	detectCycle(final_states.front());
			// Store the result
			results.addResult(state_index, product->getParameters(final_states.front().first));
			// Remove the state
			final_states.pop();
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
	ModelChecker(const UserOptions & _user_options, const SplitManager _split_manager, const ParametrizedStructure & _structure, 
		         const AutomatonStructure & _automaton, Results & _results) 
	            : split_manager(_split_manager), user_options(_user_options), structure(_structure), automaton(_automaton), results(_results) { 
	}

	/**
	 * Function that does all the coloring. This part only covers iterating through subparts.
	 */
	void computeResults() {
		// Create a new structure
		product.reset(new ProductStructure(structure.getStatesCount() * automaton.getStatesCount() , getParamsetSize()));

		// Cycle through the rounds
		while (!split_manager.lastRound()) {
			syntetizeParameters();
			split_manager.increaseRound();
		}
		// Compute the last round
		syntetizeParameters();
	}
};


#endif