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
#include "../auxiliary/common_functions.hpp"

class ProductBuilder;

class ColorStorage {
	friend class ProductBuilder;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	struct ColorData {
		std::size_t color_num; // Relative number of the color in this round
		std::vector<StateID> predecessors; // Predecessors only for this color
		std::vector<StateID> succesors; // Succesors only for this color
	};
	
	struct State {
		StateID ID;
		Parameters parameters; // 32 bits for each color in this round marking its presence or absence
		std::map<StateID, Parameters> predecessors; // Stores a predeccesor in the form (product_ID, parameters)
		std::map<StateID, Parameters> successors; // Stores succesors in the same form

		State(const StateID _ID) : ID(_ID), parameters(0) { }
	};
	
	WitnessUse current_mode; // If set to none_wit, stores only parameters

	// DATA STORAGE
	std::vector<State> states;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Add a new storage for a single state
	 *
	 * @param predecessors	vector with IDs of all predecessors of this state
	 */
	void addState(const StateID ID) {
		states.push_back(State(ID));
	}


	/**
	 * Add a new predecessor to the state
	 *
	 * @param ID	target of the transition that will be upgraded
	 * @param source	source of the transition
	 */
	void addPredecessor(const StateID ID, const StateID source) {
		states[ID].predecessors.insert(std::make_pair(source, 0));
	}

	/**
	 * Add a new successor to the state
	 *
	 * @param ID	source of the transition that will be upgraded
	 * @param target	target of the transition
	 */
	void addSuccessor(const StateID ID, const StateID target) {
		states[ID].successors.insert(std::make_pair(target, 0));
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
		states[target_ID].predecessors.find(source_ID)->second |= parameters;
		states[source_ID].successors.find(target_ID)->second |= parameters;
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
	 * Get all the predecessors for this color from this state.
	 *
	 * @param ID	index of the state to ask for predecessors
	 * @param successors	true if successors are required, false if predecessors
	 * @param color_mask	bitmask for a given color, if it is not specified, all colors are required
	 *
	 * @return predecessors for given state and color
	 */
	inline const Neighbours getNeighbours(const StateID ID, const bool successors, const Parameters color_mask = ~0) const {
		// reference
		auto neigbours = successors ? states[ID].successors : states[ID].predecessors;
		// Data to fill
		Neighbours color_neigh;

		// Add these from the color
		forEach(neigbours, [&color_neigh, color_mask](const std::pair<StateID, Parameters> & neighbour) {
			// Test if the color is present
			if ((neighbour.second & color_mask) != 0)
				color_neigh.push_back(neighbour.first);
		});

		return color_neigh;
	}
};

#endif