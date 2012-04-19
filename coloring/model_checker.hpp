/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_MODEL_CHECKER_INCLUDED
#define PARSYBONE_MODEL_CHECKER_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ModelChecker class solves the parameter synthesis problem by iterative transfer parameters from initial states to final ones.
// Functions in model checker use many supporting variables and therefore are quite long, it would not make sense to split them, though.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../reforging/product_structure.hpp"
#include "parameters_functions.hpp"

class ModelChecker {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	// Provided with constructor
	const ParametrizedStructure & structure; // Stores info about KS states
	const AutomatonStructure & automaton; // Stores info about BA states
	const ProductStructure & product; // Product on which the computation will be conducted
	ColorStorage & storage; // Auxiliary product storage

	// Used for computation
	std::set<StateID> updates; // Set of states that need to spread their updates
	std::set<StateID> next_updates; // Set of states that will be scheduled for update during next BFS level
	std::size_t BFS_level; // Number of current BFS level during coloring
	bool witness_found; // True when final state is reached
	Range synthesis_range; // First and one beyond last color to be computed in this round
	WitnessUse witness_use; // How wintesses will be held in this computation

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTING FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	/**
	 * Main function of coloring - creates intersection of passing and transition colors to create and update color.
	 *
	 * @param passed	parameters that are passed through the transition
	 * @param step_size	how many parameters share the same value for given function
	 * @param transitive_values	mask of all values from which those that have false are non-transitive
	 */
	void passParameters(Parameters & passed, const std::size_t step_size, const std::vector<bool> & transitive_values) const {
		// INITIALIZATION OF VALUES FOR POSITIONING
		// Number of the first parameter
		std::size_t param_num = synthesis_range.first;
		// First value might not bet 0 - get it from current parameter position
		std::size_t value_num = (param_num / step_size) % transitive_values.size();
		// As well current value step might not be the first one, it is also necessary to get it from current parameter position
		std::size_t substep = param_num % step_size;
		// Mask that will be created
		register Parameters temporary = 0;

		// COMPUTATION OF MASK
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
					passed &= temporary;
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
	 * From all the updates pick the one from the state with most bits
	 *
	 * @return index of the state to start an update from
	 */
	const StateID getStrongestUpdate() const {
		// Reference value
		StateID ID = 0;
		Parameters current_par = 0;
		// Cycle throught the updates
		for (auto update_it = updates.begin(); update_it != updates.end(); update_it++) {
			// Compapre with current data - if better, replace
			if (storage.getColor(*update_it) == (current_par | storage.getColor(*update_it))) {
				ID = *update_it;
				current_par = storage.getColor(ID);
			}
		}
		return ID;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COLORING FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Get stripped parameters for each unique edge (if there are multi-edges, intersect their values)
	 *
	 * @param ID	ID of the source state in the product
	 * @param parameters	parameters that will be distributed
	 */
	std::vector<Coloring> broadcastParameters(const StateID ID, const Parameters parameters) const {
		// To store parameters that passed the transition but were not yet added to the target
		std::vector<Coloring> update(product.getTransitionCount(ID));
		// Number of unique updates
		std::size_t updates_count = 0;

		// Cycle through all the transition
		for (std::size_t trans_num = 0; trans_num < product.getTransitionCount(ID); trans_num++) {
			// Parameters to pass through the transition
			Parameters passed = parameters;
			// From an update strip all the parameters that can not pass through the transition - color intersection on the transition
			passParameters(passed, product.getStepSize(ID, trans_num), product.getTransitive(ID, trans_num));

			// If the update is already present for this state (only for self-loops), do an intersection
			StateID target_ID = product.getTargetID(ID, trans_num);
			// Remains true if no update for this state has been found
			bool is_new = true;
			// Test all currently known updates for equivalence on states
			for (std::size_t update_num = 0; update_num < updates_count; update_num++) {
				// If they are the same
				if (update[update_num].first == target_ID) {
					update[update_num].second &= passed;
					is_new = false;
					break;
				}
			}
			// If it not found, add it
			if (is_new) {
				update[updates_count++] = std::make_pair(target_ID, passed);
			}
		}	

		// Return all filled updates
		return std::vector<Coloring>(update.begin(), update.begin() + updates_count);
	}
	
	/**
	 * From the source distribute its parameters and newly colored neighbours shedule for update.
	 *
	 * @param ID	ID of the source state in the product
	 * @param parameters	parameters that will be distributed
	 */
	void transferUpdates(const StateID ID, const Parameters parameters) {
		// Get passed colors, unique for each sucessor
		std::vector<Coloring> update = std::move(broadcastParameters(ID, parameters));

		// For all passed values make update on target
		for (auto update_it = update.begin(); update_it != update.end(); update_it++) {
			if (user_options.timeSerie()) {
				// Do not allow to pass between initial states and final states
				if (product.isInitial(update_it->first) || product.isFinal(ID))
					continue;
			}
			if (!none(update_it->second)) {
				// If something new is added to the target, schedule it for an update
				if (witness_use == none_wit) {
					if (storage.update(update_it->second, update_it->first)) {
						updates.insert(update_it->first);
					}
				}
				else {
					if (storage.update(ID, update_it->second, update_it->first)) {
						updates.insert(update_it->first);
					}
				}
			}
		}
	}

	/**
	 * Distribute updates and store the new ones.
	 */
	void doColoring() {
		// While there are updates, pass them to succesing vertices
		while (!updates.empty()) {
			// Within updates, find the one with most bits
			StateID ID = getStrongestUpdate();
			// Pass data from updated vertex to its succesors
			transferUpdates(ID, storage.getColor(ID));
			// Erase completed update from the set
			updates.erase(ID);
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTING FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Sets/resets all coloring reference values;
	 *
	 * @param _range	range of parameters for this coloring round
	 * @param _witness_use	how to manage witnesses in this coloring round
	 */
	void prepareCheck(const Range & _range, const WitnessUse _witness_use) {
		witness_use = _witness_use;
		synthesis_range = _range;
		BFS_level = 0;
		witness_found = false;	
	}

	ModelChecker(const ModelChecker & other);            // Forbidden copy constructor.
	ModelChecker& operator=(const ModelChecker & other); // Forbidden assignment operator.

public:
	/**
	 * Constructor, passes the data
	 */
	ModelChecker(const ProductStructure & _product, ColorStorage & _storage) : structure(_product.getKS()), automaton(_product.getBA()), product(_product), storage(_storage) { }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STARTERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Start a new coloring round for cycle detection from a single state.
	 *
	 * @param ID	ID of the state to start cycle detection from
	 * @param parameters	starting parameters for the cycle detection
	 * @param _range	range of parameters for this coloring round
	 * @param _witness_use	how to manage witnesses in this coloring round
	 */
	const std::size_t startColoring(const StateID ID, const Parameters parameters, const Range & _range, const WitnessUse _witness_use = none_wit) {
		prepareCheck(_range, _witness_use);
		updates.clear();
		transferUpdates(ID, parameters);
		doColoring();
		return BFS_level;
	}

	/**
	 * Start a new coloring round for cycle detection from a single state.
	 *
	 * @param _updates	states that are will be scheduled for an update in this round
	 * @param _range	range of parameters for this coloring round
	 * @param _witness_use	how to manage witnesses in this coloring round
	 */
	const std::size_t startColoring(const std::set<StateID> & _updates, const Range & _range, const WitnessUse _witness_use = none_wit) {
		prepareCheck(_range, _witness_use);
		updates = _updates;
		doColoring();
		return BFS_level;
	}
};


#endif