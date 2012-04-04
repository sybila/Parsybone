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

#ifndef PARSYBONE_PRODUCT_INCLUDED
#define PARSYBONE_PRODUCT_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product stores product of BA and PKS
// States are indexed as (BA_state_count * KS_state_ID + BA_state_ID) - e.g. if 3-state BA state ((1,0)x(1)) would be at position 3*1 + 1 = 4
// In other words, first iterate through BA then through KS
// Product data can be set only form the ProductBuilder object.
// Product is used for computation - meaning it has also setter / computation functions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <set>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <queue>
#include <utility>

#include "../auxiliary/data_types.hpp"
#include "../auxiliary/output_streamer.hpp"
#include "../reforging/automaton_structure.hpp"
#include "../reforging/parametrized_structure.hpp"

class Product {
	friend class ProductBuilder;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// typedef std::vector<std::size_t> Predecessors; // Where to store predecessors
	
	// References to data structures
	const UserOptions & user_options; // Values provided as parameters
	const ParametrizedStructure & structure; // Stores info about KS states
	const AutomatonStructure & automaton; // Stores info about BA states
	
	// pointer used to access all the parameters
	std::vector<Parameters> states;
	// std::vector<Predecessors> state_predecesors;

	// Information
	std::set<std::size_t> initial_states;
	std::set<std::size_t> final_states;
	// WitnessUse witness_use;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	Product(const Product & other);            // Forbidden copy constructor.
	Product& operator=(const Product & other); // Forbidden assignment operator.

public:
	Product(const UserOptions &_user_options, const ParametrizedStructure & _structure, const AutomatonStructure & _automaton) 
		: user_options(_user_options), structure(_structure), automaton(_automaton) { }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA HANDLING FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Sets all to zero
	 */ 
	void resetProduct() {
		std::for_each(states.begin(), states.end(),[](Parameters & parameters) {
			parameters = 0;
		});
	}

	/**
	 * @param state_num	index of the state to fill
	 * @param parameters to add - if empty, add all, otherwise use bitwise or
	 * 
	 * @return true if there was an actuall update
	 */
	inline bool updateParameters(const Parameters parameters, const std::size_t state_num) {
		if (states[state_num] == (parameters | states[state_num]))
			return false;
		states[state_num] |= parameters;
		return true;
	}

	/**
	 * Color initial states of the product with given color
	 *
	 * @param color	color to use for the initial coloring
	 *
	 * @return set of initial vertices
	 */
	const std::set<std::size_t> & colorInitials(const Parameters color) {
		std::for_each(initial_states.begin(), initial_states.end(), [&](std::size_t state_index) {
			states[state_index] = color;
		});
		return initial_states;
	}

	/**
	 * Pick final states from the product and store them with their parameters in the queue of colorings
	 *
	 * @return queue with all colorings of final states
	 */
	std::queue<Coloring> storeFinalStates() {
		// Queue tates colored in basic coloring
		std::queue<Coloring> final_colorings; 

		// Get the states and their colors
		std::for_each(final_states.begin(), final_states.end(), [&](std::size_t state_index) {
			final_colorings.push(Coloring(state_index, states[state_index]));
		});

		// Return final vertices with their positions
		return final_colorings;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	 * @return set of the initial states
	 */
	inline const std::set<std::size_t> & getInitials() {
		return initial_states;
	}

	/**
	 * @return set of final states
	 */ 
	inline const std::set<std::size_t> & getFinals() {
		return final_states;
	}

	/**
	 * @return	number of states of the product structure
	 */
	inline const std::size_t getStatesCount() const {
		return states.size();
	}

	/**
	 * @return index of this combination of states in the product
	 */
	inline const std::size_t getProductIndex(const std::size_t ks_index, const std::size_t ba_index) const {
		return (ks_index * automaton.getStatesCount() + ba_index);
	}

	/**
	 * @return index of this combination of states in the product in the form (KS_state, BA_state)
	 */
	inline const std::pair<std::size_t, std::size_t> getStateIndexes(const std::size_t product_index) const {
		const std::size_t KS_state = product_index / automaton.getStatesCount();
		const std::size_t BA_state = product_index % automaton.getStatesCount();
		return std::make_pair(KS_state, BA_state);
	}

	/**
	 * @param state_num	index of the state to ask
	 * 
	 * @return parameters assigned to the state
	 */
	inline const Parameters & getParameters(const std::size_t state_num) {
		return states[state_num];
	}
};

#endif