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
// This class uses some global data that have to be handled safely, but it makes the computation faster.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdexcept>
#include <queue>
#include <cmath>

#include "../reforging/parametrized_structure.hpp"
#include "../reforging/automaton_structure.hpp"
#include "../reforging/product_structure.hpp"
#include "parameters_functions.hpp"
#include "../results/results.hpp"
#include "../auxiliary/output_streamer.hpp"

class ModelChecker {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	// Provided with constructor
	const UserOptions & user_options;
	const ParametrizedStructure & structure; // Stores info about KS states
	const AutomatonStructure & automaton; // Stores info about BA states
	ProductStructure & product;

	// Used for computation
	std::set<std::size_t> updates;
	Range synthesis_range;

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
		std::size_t param_num = synthesis_range.first;
		// First value might not bet 0 - get it from current parameter position
		std::size_t value_num = (param_num / step_size) % transitive_values.size();
		// As well current value step might not be the first one, it is also necessary to get it from current parameter position
		std::size_t substep = param_num % step_size;
		// Mask that will be created
		register Parameters temporary = 0;

		// List through all the paramters
		while (true) {
			// List through ALL the target values
			for (; value_num < transitive_values.size(); value_num++) {
				// Get size of the step for current value 
				std::size_t bits_in_step = std::min<std::size_t>(step_size, synthesis_range.second - param_num);
				// Move the mask so new value data can be add
				temporary <<= bits_in_step;
				// If transitive, add ones for the width of the step
				if (transitive_values[value_num]) {
					Parameters add = ~0;
					add >>= (getParamsetSize() - bits_in_step);
					temporary |= add;
				}
				// If we went throught the whole size, end
				if ((param_num += bits_in_step) == synthesis_range.second) {
					// Create interection of source parameters and transition parameters
					target_param &= temporary;
					return;
				}
				// Reset steps for the value
				substep = 0;
			}
			// Reset the value
			value_num = 0;
		}
	}

	/**
	 * Update target of the transition with transitible parameters
	 *
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
			std::size_t target_state = product.getProductIndex(structure.getTargetID(source_KS_state,KS_transition_num), target_BA_state);
			if (product.updateParameters(parameters, target_state))
				updates.insert(target_state);
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
	ModelChecker(const UserOptions & _user_options, ProductStructure & _product) 
		        : user_options(_user_options), structure(_product.getKS()), automaton(_product.getBA()), product(_product) { 
	}

	/**
	 * From the source distribute its parameters and newly colored neighbours shedule for update.
	 *
	 * @param souce_state	ID of the source state in the producte
	 * @param parameters	parameters that will be distributed
	 */
	void transferUpdates(const std::size_t source_state, const Parameters parameters) {
		// From ID of product state compute IDs of KS and BA states
		std::size_t source_KS_state = product.getStateIndexes(source_state).first;
		std::size_t source_BA_state = product.getStateIndexes(source_state).second;

		// For each feasible transition of BA store its ID in the queue
		std::queue<std::size_t> transitible_ba;
		for (std::size_t transition_num = automaton.getBeginIndex(source_BA_state); transition_num < automaton.getBeginIndex(source_BA_state + 1); transition_num++) {
			// Check the transitibility
			if (automaton.isTransitionFeasible(transition_num, structure.getStateLevels(source_KS_state)))
				transitible_ba.push(automaton.getTarget(transition_num));
		}

		// Push updates for each BA transition times each KS transition
		while (!transitible_ba.empty()) {
			// Get index of target state of BA and remove it from the queue
			std::size_t target_BA_state = transitible_ba.front();
			transitible_ba.pop();
			// Combine BA transition with all KS transitions and update those
			for (std::size_t KS_transition_num = 0; KS_transition_num < structure.getTransitionsCount(source_KS_state); KS_transition_num++) {
				updateTarget(parameters, KS_transition_num, source_KS_state, target_BA_state);
			}
		}
	}

	/**
	 * Distribute updates and store the new ones. Used only for initial coloring.
	 */
	void doColoring() {
		// While there are updates, pass them to succesing vertices
		while (!updates.empty()) {
			// Heuristics for an update with approximatelly maximal number of bits set
			std::size_t state_num = 0; std::size_t current_par = 0;
			for (auto update_it = updates.begin(); update_it != updates.end(); update_it++) {
				if (product.getParameters(*update_it) == (current_par | product.getParameters(*update_it))) {
					state_num = *update_it;
					current_par = product.getParameters(state_num);
				}
			}
			// Pass data from updated vertex to its succesors
			transferUpdates(state_num, product.getParameters(state_num));
			// Erase completed update from the set
			updates.erase(state_num);
		}
	}

	/**
	 * Distribute updates and store the new ones. Used only in cycle detection - prefers updates of the source vertex.
	 */
	void doAcceptingColoring(const std::size_t source_state, const Parameters parameters) {
		// While there are updates, pass them to succesing vertices
		while (!updates.empty()) {
			// Heuristics for an update with approximatelly maximal number of bits set
			std::size_t state_num = 0; std::size_t current_par = 0;
			for (auto update_it = updates.begin(); update_it != updates.end(); update_it++) {
				if (product.getParameters(*update_it) == (current_par | product.getParameters(*update_it))) {
					state_num = *update_it;
					current_par = product.getParameters(state_num);
				}
				if (source_state == *update_it) {
					state_num = *update_it;
					break;
				}
			}
			// Pass data from updated vertex to its succesors
			transferUpdates(state_num, product.getParameters(state_num));
			if (source_state == state_num)
				if (product.getParameters(state_num) == parameters)
					return;
			// Erase completed update from the set
			updates.erase(state_num);
		}
	}

	/**
	 * Assign provided set as current updates.
	 */
	void setUpdates(const std::set<std::size_t> & _updates = std::set<std::size_t>()) {
		updates = _updates;
	}

	/**
	 * Set first and last parameters for this round.
	 */
	void setRange(const Range & _range) {
		synthesis_range = _range;
	}
};


#endif