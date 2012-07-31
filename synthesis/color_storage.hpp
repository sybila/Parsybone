/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_COLOR_STORAGE_INCLUDED
#define PARSYBONE_COLOR_STORAGE_INCLUDED

#include "../auxiliary/data_types.hpp"
#include "../auxiliary/common_functions.hpp"
#include "paramset_helper.hpp"

class ProductBuilder;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ColorStorage is auxiliary class to the product and stores colors and possibly predecessors for individual states of the product during the computation.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ColorStorage {
	struct State {
		StateID ID;
		Parameters parameters; ///< 32 bits for each color in this round marking its presence or absence
		std::vector<Parameters> predecessors; ///< Stores a predeccesor in the form (product_ID, parameters)
		std::vector<Parameters> successors; ///< Stores succesors in the same form

		State(const StateID _ID, const std::size_t states_num) : ID(_ID), parameters(0) {
			parameters = 0;
			predecessors.resize(states_num, 0);
			successors.resize(states_num, 0);
		}
	};
	
	std::vector<State> states; ///< Vector of states that correspond to those of Product Structure and store coloring data
	/// This vector stores so-called COST value i.e. number of steps required to reach the final state in TS.
	/// If it is not reachable, cost is set to ~0.
	std::vector<std::size_t> cost_val;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Add a new storage for a single state
	 *
	 * @param predecessors	vector with IDs of all predecessors of this state
	 */
	void addState(const StateID ID, const std::size_t states_num) {
		states.push_back(State(ID, states_num));
	}

public:
	/**
	 * Constructor allocates necessary memory for further usage (this memory is not supposed to be freed until endo of the computation)
	 * Every state has predecessors and succesors allocated for EVERY other state, this consumes memory but benefits the complexity of operations.
	 *
	 * @param states_count	number of states the structure the data will be saved for has
	 */
	ColorStorage(std::size_t states_count) {
		for (StateID ID = 0; ID < states_count; ID++) {
			addState(ID, states_count);
		}
		cost_val = std::vector<std::size_t>(paramset_helper.getParamsetSize(), ~0); // Set all to max. value
	}

	ColorStorage() {} ///< Empty constructor for an empty storage

	/**
	 * Sets all values for all the states to zero. Allocated memory remains.
	 */ 
	void reset() {
		// Clear each state
		std::for_each(states.begin(), states.end(),[&](State & state) {
			// Reset merged parameters
			state.parameters = 0;
			// Reset parameters from predecessors, if there were new values
			if (user_options.witnesses()) {
				for(auto pred_it = state.predecessors.begin(); pred_it != state.predecessors.end(); pred_it++) {
					*pred_it = 0;
				}
				for(auto succ_it = state.successors.begin(); succ_it != state.successors.end(); succ_it++) {
					*succ_it = 0;
				}
			}
		});
	}

	/**
	 * Add all values from one coloring structure to another.
	 * @attention	This will only sum, not replace original values
	 *
	 * @param other	structure to copy from
	 */
	void addFrom(const ColorStorage & other) {
		// For all states
		auto this_state_it = states.begin();
		for (StateID ID = 0; ID < states.size(); ID++, this_state_it++) {
			// Copy params
			this_state_it->parameters |= other.getColor(ID);

			// Copy succesors parameters
			auto this_succ_it = this_state_it->successors.begin();
			std::vector<Parameters> other_succs = std::move(other.getMarking(ID, true));
			for (auto other_succ_it = other_succs.begin(); other_succ_it != other_succs.end(); this_succ_it++, other_succ_it++) {
				*this_succ_it |= *other_succ_it;
			}

			// Copy predecessors parameters
			auto this_pred_it = this_state_it->predecessors.begin();
			std::vector<Parameters> other_preds = std::move(other.getMarking(ID, false));
			for (auto other_pred_it = other_preds.begin(); other_pred_it != other_preds.end(); this_pred_it++, other_pred_it++) {
				*this_pred_it |= *other_pred_it;
			}
		}
	}

	/**
	 * Just passes a new cost vector
	 *
	 * @param new_cost	a vector of lenght |parameter_set| containing cost values. If the value does not exist (state is not reachable), use ~0
	 */
	void setCost(const std::vector<std::size_t> & new_cost) {
		cost_val = new_cost;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PARAMTERS HANDLING
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Add passed colors to the state
	 *
	 * @param ID	index of the state to fill
	 * @param parameters to add - if empty, add all, otherwise use bitwise or
	 * 
	 * @return true if there was an actuall update
	 */
	inline bool update(const Parameters parameters, const StateID ID) {
		// If nothing is new return false
		if (states[ID].parameters == (parameters | states[ID].parameters))
			return false;
		// Add new parameters and return true
		states[ID].parameters |= parameters;
		return true;
	}

	/**
	 * Return true if the state would be updated, false otherwise.
	 *
	 * @param ID	index of the state to fill
	 * @param parameters to add - if empty, add all, otherwise use bitwise or
	 *
	 * @return true if there would be an update
	 */
	inline bool soft_update(const Parameters parameters, const StateID ID) {
		if (states[ID].parameters == (parameters | states[ID].parameters))
			return false;
		else
			return true;
	}

	/**
	 * Add passed colors to the state
	 *
	 * @param source_ID	index of the state that passed this update
	 * @param target_ID	index of the state to fill
	 * @param parameters to add - if empty, add all, otherwise use bitwise or
	 * 
	 * @return true if there was an actuall update
	 */
	inline bool update(const StateID source_ID, const Parameters parameters, const StateID target_ID) {
		// Mark parameters source and target
		states[target_ID].predecessors[source_ID] |= parameters;
		states[source_ID].successors[target_ID] |= parameters;
		// Make an actuall update
		return update(parameters, target_ID);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @param ID	index of the state to ask for parameters
	 * 
	 * @return parameters assigned to the state
	 */
	inline const Parameters & getColor(const StateID ID) const {
		return states[ID].parameters;
	}

	/**
	 * @param states	indexes of states to ask for parameters
	 *
	 * @return queue with all colorings of states
	 */
	const std::vector<Coloring> getColor(const std::vector<StateID> & states) const {
		// Queue tates colored in basic coloring
		std::vector<Coloring> colors; 

		// Get the states and their colors
		std::for_each(states.begin(), states.end(), [&](const StateID ID) {
			colors.push_back(Coloring(ID, getColor(ID)));
		});

		// Return final vertices with their positions
		return colors;
	}

	/**
	 * @return vector of states that have any color attached to them
	 */
	std::set<StateID> getColored() const {
		std::set<StateID> new_updates;
		for (auto state_it = states.begin(); state_it != states.end(); state_it++) {
			if (!paramset_helper.none(state_it->parameters)) {
				new_updates.insert(state_it->ID);
			}
		}
		return new_updates;
	}

	/** 
	 * Get all the neigbours for this color from this state.
	 *
	 * @param ID	index of the state to ask for predecessors
	 * @param successors	true if successors are required, false if predecessors
	 * @param color_mask	bitmask for a given color, if it is not specified, all colors are required
	 *
	 * @return neigbours for given state
	 */
	inline const Neighbours getNeighbours(const StateID ID, const bool successors, const Parameters color_mask = ~0) const {
		// reference
		auto neigbours = successors ? states[ID].successors : states[ID].predecessors;
		// Data to fill
		Neighbours color_neigh;

		// Add these from the color
		StateID neigh_num = 0;
		forEach(neigbours, [&neigh_num, &color_neigh, color_mask](const Parameters neighbour) {
			// Test if the color is present
			if ((neighbour & color_mask) != 0)
				color_neigh.push_back(neigh_num);
			neigh_num++;
		});

		return color_neigh;
	}

	/** 
	 * Get all the neigbours for this color from this state.
	 *
	 * @param ID	index of the state to ask for predecessors
	 * @param successors	true if successors are required, false if predecessors
	 *
	 * @return neigbours for given state and their color
	 */
	inline const std::vector<Parameters> getMarking(const StateID ID, const bool successors) const {
		// reference
		auto neigbours = successors ? states[ID].successors : states[ID].predecessors;

		return neigbours;
	}

	/**
	 *
	 */
	const std::size_t getCost(std::size_t position) const {
		return cost_val[position];
	}

	/**
	 *
	 */
	const std::vector<std::size_t> & getCost() const {
		return cost_val;
	}
};

#endif // PARSYBONE_COLOR_STORAGE_INCLUDED
