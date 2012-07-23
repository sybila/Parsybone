/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_MODEL_CHECKER_INCLUDED
#define PARSYBONE_MODEL_CHECKER_INCLUDED

#include "../auxiliary/common_functions.hpp"
#include "../construction/construction_holder.hpp"
#include "color_storage.hpp"
#include "parameters_functions.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ModelChecker class solves the parameter synthesis problem by iterative transfer parameters from initial states to final ones.
/// Functions in model checker use many supporting variables and therefore are quite long, it would not make sense to split them, though.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ModelChecker {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	// Provided with constructor
    const ProductStructure & product; // Product on which the computation will be conducted
	ColorStorage & storage; // Auxiliary product storage
	ColorStorage next_round_storage; // Storing values passed in this round

	// Used for computation
	std::set<StateID> updates; // Set of states that need to spread their updates
	std::set<StateID> next_updates;
	Range synthesis_range; // First and one beyond last color to be computed in this round
	WitnessUse witness_use; // How wintesses will be held in this computation

	// Used for self loops
	std::vector<bool> BA_presence;

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
		ColorNum param_num = synthesis_range.first;
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
				std::size_t bits_in_step = std::min<std::size_t>(step_size, static_cast<std::size_t>(synthesis_range.second - param_num));
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
	 * @return	index of the state to start an update from
	 */
	const StateID getStrongestUpdate() const {
		// Reference value
		StateID ID = 0;
		Parameters current_color = 0;
		// Cycle throught the updates
		for (auto update_it = updates.begin(); update_it != updates.end(); update_it++) {
			Parameters test_color = storage.getColor(*update_it);
			// Compare with current data - if better, replace
			if (test_color != current_color) {
				if (test_color == (current_color | test_color)) {
					ID = *update_it;
					current_color = test_color;
				}
			}
		}
		return ID;
	}

	/**
	 * For each found color add this BFS level as a first when the state was updated, if it was not already found.
	 *
	 * @param colors	current coloring
	 */
	void markLevels(const Parameters colors) {
		// If all is found, end
		if (!to_find)
			return;

		// Which are new
		Parameters store = to_find & colors;
		// Remove currently found
		to_find &= ~colors;

		// Store those that were found in this round
		for (int color_pos = static_cast<int>(getParamsetSize() - 1); color_pos >= 0 ; store >>= 1, color_pos--) {
			if (store % 2)
				BFS_reach[color_pos] = BFS_level;		
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
	std::vector<Coloring> broadcastParameters(const StateID ID, const Parameters parameters) {
		// To store parameters that passed the transition but were not yet added to the target
		std::vector<Coloring> param_updates;
		param_updates.reserve(product.getTransitionCount(ID));

		std::size_t KS_state = product.getKSID(ID);
		Parameters self_loop = ~0;


		// Cycle through all the transition
		for (std::size_t trans_num = 0; trans_num < product.getTransitionCount(ID); trans_num++) {
			// Parameters to pass through the transition
			Parameters passed = parameters;
			// From an update strip all the parameters that can not pass through the transition - color intersection on the transition
			passParameters(passed, product.getStepSize(ID, trans_num), product.getTransitive(ID, trans_num));

			StateID target_ID = product.getTargetID(ID, trans_num);

			// Test if it is a self loop, if there is nothing outcoming, add to self-loop (if it is still possible)
			if (self_loop && (KS_state == product.getKSID(target_ID)) ) {
				self_loop &= passed;
				StateID BA_ID = product.getBAID(target_ID);
				BA_presence[BA_ID] = true;
			}
			// Else add normally and remove from the self_loop
			else if (passed) {
				self_loop &= ~passed;
				param_updates.push_back(std::make_pair(target_ID, passed));
			}
		}	

		// If there is a self-loop, add it for all the BA_states
		for(StateID BA_state = 0; BA_state < BA_presence.size(); BA_state++) {
			if (BA_presence[BA_state]) {
				if (self_loop) {
					StateID target = product.getProductID(KS_state, BA_state) ;
					param_updates.push_back(Coloring(target, self_loop));
				}
				// Null the value
				BA_presence[BA_state] = false;
			}
		}

		// Return all filled updates
		return param_updates;
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
			// Skip empty updates
			if (none(update_it->second))
				continue;

			// Do not allow to pass between initial states and final states in the time serie
			if (user_options.timeSerie()) {	
				if (product.isInitial(update_it->first) || product.isFinal(ID))
					continue;
			}

			// If something new is added to the target, schedule it for an update
			if ((witness_use == none_wit && storage.update(update_it->second, update_it->first)))
				updates.insert(update_it->first);
			else if (witness_use == all_wit && storage.update(ID, update_it->second, update_it->first))
				updates.insert(update_it->first);
			else if (witness_use == short_wit && storage.soft_update(update_it->second, update_it->first)) // Only test
				next_round_storage.update(ID, update_it->second, update_it->first); // If something is present, schedule it for next round
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
			if (updates.empty() && witness_use == short_wit && to_find ) {
				updates = std::move(next_round_storage.getColored()); // Get updates from this level coloring
				storage.addFrom(next_round_storage); // Copy updated
				next_round_storage.reset(); // Clean storage
				BFS_level++; // Increase level
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
		to_find = parameters; // Store which parameters are we searching for
		updates = start_updates; // Copy starting updates
		witness_use = _witness_use; // Store witness use info
		synthesis_range = _range; // Copy range of this round

		if (witness_use == short_wit) {
			BFS_level = 1; // Set sterting number of BFS
			next_updates.clear(); // Ensure emptiness of the next round
			BFS_reach.resize(getParamsetSize(), 0); // Recreate reach values
			next_round_storage = storage; // Copy starting values
		}
	}

	ModelChecker(const ModelChecker & other);            // Forbidden copy constructor.
	ModelChecker& operator=(const ModelChecker & other); // Forbidden assignment operator.

public:
	/**
	 * Constructor, passes the data
	 */
    ModelChecker(const ConstructionHolder & holder, ColorStorage & _storage) : product(holder.getProduct()), storage(_storage) {
        BA_presence.resize(holder.getAutomatonStructure().getStateCount(), false);
	}

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
		transferUpdates(ID, parameters); // Transfer updates from the start of the detection
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
