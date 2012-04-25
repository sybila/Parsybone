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

#include "../auxiliary/common_functions.hpp"
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
	Range synthesis_range; // First and one beyond last color to be computed in this round
	WitnessUse witness_use; // How wintesses will be held in this computation

	// Bounded check variables
	Parameters to_find;
	std::vector<std::size_t> BFS_reach; // In which round this color was found
	std::size_t BFS_level; // Number of current BFS level during coloring

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

	/**
	 * For each found color add this BFS level as a first when the state was updated, if it was not already found.
	 *
	 * @param colors	current coloring
	 */
	void markLevels(Parameters colors) {
		if (!to_find)
			return;
		// Remove currently found
		to_find &= ~colors;

		std::size_t color_pos = getParamsetSize() - 1;
		while (color_pos < (synthesis_range.second - synthesis_range.first)) {
			if (colors % 2)
				BFS_reach[color_pos] = my_min(BFS_reach[color_pos], BFS_level);
			
			colors >>= 1;
			color_pos--;
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COLORING FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	 * Get stripped parameters for each unique edge (if there are multi-edges, intersect their values)
	 *
	 * @param ID	ID of the source state in the product
	 * @param parameters	parameters that will be distributed
	 *
	 * @return vector of passed parameters together with their targets
	 */
	std::vector<Coloring> broadcastParameters(const StateID ID, const Parameters parameters) const {
		// To store parameters that passed the transition but were not yet added to the target
		std::vector<Coloring> updates;
		updates.reserve(product.getTransitionCount(ID));
		// To store info about self-loops
		std::set<StateID> BA_set;
		Parameters self_loop = ~0;

		// Cycle through all the transition
		for (std::size_t trans_num = 0; trans_num < product.getTransitionCount(ID); trans_num++) {
			// Parameters to pass through the transition
			Parameters passed = parameters;
			// From an update strip all the parameters that can not pass through the transition - color intersection on the transition
			passParameters(passed, product.getStepSize(ID, trans_num), product.getTransitive(ID, trans_num));

			StateID target_ID = product.getTargetID(ID, trans_num);

			// Test if it is a self loop, if there is nothing outcoming, add to self-loop (if it is still possible)
			if (product.getKSID(ID) == product.getKSID(target_ID) && self_loop) {
				self_loop &= passed;
				BA_set.insert(target_ID);
			}
			// Else add normally and remove from the self_loop
			else if (passed) {
				self_loop &= ~passed;
				updates.push_back(std::make_pair(target_ID, passed));
			}
		}	

		// If there is a self-loop, add it for all the BA_states
		if (self_loop) {
			for (auto BA_it = BA_set.begin(); BA_it != BA_set.end(); BA_it++) {
				updates.push_back(std::make_pair(*BA_it, self_loop));
			}
		}

		// Return all filled updates
		return updates;
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
				if ((witness_use == none_wit && storage.update(update_it->second, update_it->first)) || storage.update(ID, update_it->second, update_it->first)) {
					next_updates.insert(update_it->first);
				}		
			}
		}
	}

	/**
	 * Distribute updates and store the new ones.
	 */
	void doColoring() {
		// While there are updates, pass them to succesing vertices
		do  {
			// Within updates, find the one with most bits
			StateID ID = getStrongestUpdate();
			// Check if this is not the last round
			if (user_options.witnesses() == short_wit && product.isFinal(ID)) 
				markLevels(storage.getColor(ID));
			// Pass data from updated vertex to its succesors
			transferUpdates(ID, storage.getColor(ID));
			// Erase completed update from the set
			updates.erase(ID);
			// If witness has not been found and 
			if ((updates.empty()) && (to_find || (witness_use != short_wit))) {
				updates = std::move(next_updates);
				BFS_level++;
			}
		} while (!updates.empty());
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
	void prepareCheck(const Parameters parameters, const Range & _range, const WitnessUse _witness_use, std::set<StateID> start_updates = std::set<StateID>()) {
		to_find = parameters;
		next_updates.clear();
		updates = start_updates;
		witness_use = _witness_use;
		BFS_reach.resize(getParamsetSize(), ~0);
		synthesis_range = _range;
		BFS_level = 1;
	}

	ModelChecker(const ModelChecker & other);            // Forbidden copy constructor.
	ModelChecker& operator=(const ModelChecker & other); // Forbidden assignment operator.

public:
	/**
	 * Constructor, passes the data
	 */
	ModelChecker(const ProductStructure & _product, ColorStorage & _storage) : structure(_product.getKS()), automaton(_product.getBA()), product(_product), storage(_storage) {	}

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
	const std::vector<std::size_t> startColoring(const StateID ID, const Parameters parameters, const Range & _range, const WitnessUse _witness_use = none_wit) {
		prepareCheck(parameters, _range, _witness_use);
		transferUpdates(ID, parameters);
		doColoring();
		return std::move(BFS_reach);
	}

	/**
	 * Start a new coloring round for cycle detection from a single state.
	 *
	 * @param _updates	states that are will be scheduled for an update in this round
	 * @param _range	range of parameters for this coloring round
	 * @param _witness_use	how to manage witnesses in this coloring round
	 */
	const std::vector<std::size_t> startColoring(const Parameters parameters, const std::set<StateID> & _updates, const Range & _range, const WitnessUse _witness_use = none_wit){
		prepareCheck(parameters, _range, _witness_use, _updates);
		doColoring();
		return std::move(BFS_reach);
	}
};


#endif