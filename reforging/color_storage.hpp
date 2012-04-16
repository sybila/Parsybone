/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_COLOR_STORAGE_INCLUDED
#define PARSYBONE_COLOR_STORAGE_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ColorStorage is auxiliary class to the product and stores colors and possibly predecessors for individual states of the product during the computation.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../auxiliary/data_types.hpp"

class ProductBuilder;

class ColorStorage {
	friend class ProductBuilder;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	struct State {
		Parameters parameters; // 32 bits for each color in this round marking its presence or absence
		std::vector<std::pair<std::size_t, Parameters> > predecessors; // Stores a predeccesor in the form (product_ID, parameters)

		State(const std::vector<std::size_t> _predecessors) {
			parameters = 0;
			for (auto pred_it = _predecessors.begin(); pred_it != _predecessors.end(); pred_it++) {
				predecessors.push_back(std::make_pair(*pred_it, 0));
			}
		}
	};
	
	WitnessUse current_mode; // If set to none_wit, stores only parameters

	std::vector<State> states;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Add a new storage for a single state
	 *
	 * @param predecessors	vector with IDs of all predecessors of this state
	 */
	void addState(const std::vector<std::size_t> && predecessors) {
		states.push_back(State(std::move(predecessors)));
	}


	ColorStorage(const ColorStorage & other);            // Forbidden copy constructor.
	ColorStorage& operator=(const ColorStorage & other); // Forbidden assignment operator.

public:
	ColorStorage() {
		// Set to all for the beginning - needed for clean reset
		current_mode = all_wit;
	}

	/**
	 * Sets all values for all the states to zero
	 */ 
	void reset() {
		// Clear each state
		std::for_each(states.begin(), states.end(),[&](State & state) {
			// Reset merged parameters
			state.parameters = 0;
			// Reset parameters from predecessors, if there were new values
			if (current_mode == all_wit) {
				for(auto pred_it = state.predecessors.begin(); pred_it != state.predecessors.end(); pred_it++) {
					pred_it->second = 0;
				}
			}
		});
	}

	/**
	 * Set mode in which data will be stored in this round
	 *
	 * @param new_mode	mode to use - either store only all parameters or store them together with their sources
	 */
	void setMode(const WitnessUse new_mode) {
		current_mode = new_mode;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PARAMTERS HANDLING
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @param state_num	index of the state to fill
	 * @param parameters to add - if empty, add all, otherwise use bitwise or
	 * 
	 * @return true if there was an actuall update
	 */
	inline bool update(const Parameters parameters, const std::size_t state_num) {
		// If nothing is new return false
		if (states[state_num].parameters == (parameters | states[state_num].parameters))
			return false;
		// Add new parameters and return true
		states[state_num].parameters |= parameters;
		return true;
	}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @param state_num	index of the state to ask for parameters
	 * 
	 * @return parameters assigned to the state
	 */
	inline const Parameters & getParameters(const std::size_t state_num) const {
		return states[state_num].parameters;
	}

	///** 
	// * @param state_num	index of the state to ask for predecessors
	// * @param color_index	index of the color in this round (0 .. sizeof(Parameters)*8)
	// *
	// * @return predecessors for given state and color
	// */
	//inline const Predecessors & getPredecessors(const std::size_t state_num, const std::size_t color_index) const {
	//	return states[state_num].predecessors[color_index];
	//}

	///** 
	// * @param state_num	index of the state to ask for predecessors
	// *
	// * @return predecessors for given state
	// */
	//inline const std::vector<Predecessors> & getPredecessors(const std::size_t state_num) const {
	//	return states[state_num].predecessors;
	//}
};

#endif