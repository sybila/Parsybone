/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_PRODUCT_INCLUDED
#define PARSYBONE_PRODUCT_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ProductStructure stores product of BA and PKS
// States are indexed as (BA_state_count * KS_state_ID + BA_state_ID) - e.g. if 3-state BA state ((1,0)x(1)) would be at position 3*1 + 1 = 4
// In other words, first iterate through BA then through KS
// ProductStructure data can be set only form the ProductBuilder object.
// ProductStructure is used for computation - meaning it has also setter / computation functions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../auxiliary/output_streamer.hpp"
#include "../coloring/parameters_functions.hpp"
#include "../reforging/automaton_structure.hpp"
#include "../reforging/functions_structure.hpp"
#include "../reforging/parametrized_structure.hpp"

class ProductStructure {
	friend class ProductBuilder;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	struct State {
		Parameters parameters; // 32 bits for each color in this round marking its presence or absence
		std::vector<Predecessors> predecessors; // Vector of states for each color - predecessors in this color
	};
	
	// References to data structures
	const FunctionsStructure & functions; // Implicit reprezentation of functions - used as reference
	const ParametrizedStructure & structure; // Stores info about KS states
	const AutomatonStructure & automaton; // Stores info about BA states

	// Information
	std::vector<std::size_t> initial_states;
	std::vector<std::size_t> final_states;

	std::vector<State> states;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ProductStructure(const ProductStructure & other);            // Forbidden copy constructor.
	ProductStructure& operator=(const ProductStructure & other); // Forbidden assignment operator.

public:
	ProductStructure(const FunctionsStructure & _functions, const ParametrizedStructure & _structure, const AutomatonStructure & _automaton) 
		: functions(_functions), structure(_structure), automaton(_automaton) { }

	/**
	 * Sets all to zero
	 */ 
	void resetProduct() {
		const std::size_t color_count = getParamsetSize();
		// Clear each state
		std::for_each(states.begin(), states.end(),[color_count](State & state) {
			state.parameters = 0;
			if (user_options.witnesses()) {
				for(auto color_it = state.predecessors.begin(); color_it != state.predecessors.end(); color_it++) {
					color_it->clear();
				}
			}
		});
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
	inline bool updateParameters(const Parameters parameters, const std::size_t state_num) {
		if (states[state_num].parameters == (parameters | states[state_num].parameters))
			return false;
		states[state_num].parameters |= parameters;
		return true;
	}

	/**
	 * Color initial states of the product with given color
	 *
	 * @param color	color to use for the initial coloring
	 *
	 * @return set of initial vertices
	 */
	const std::vector<std::size_t> & colorInitials(const Parameters color) {
		std::for_each(initial_states.begin(), initial_states.end(), [&](std::size_t state_num) {
			states[state_num].parameters = color;
		});
		return initial_states;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WITNESSES HANDLING
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * For all transitive parameters, add source as target's predecessor
	 *
	 * @param source	ID of the source state
	 * @param target	ID of the target state
	 * @param passed	mask of parameters that are passed from source to target
	 */
	void addPredecessor(const std::size_t source, const std::size_t target, Parameters passed) {
		// For each color
		for (std::size_t color_index = 0; color_index < getParamsetSize(); color_index++) {
			// If the color is present, add predecessor
			if (passed % 2)
				states[target].predecessors[color_index].insert(source);
			// Iterate color
			passed >>= 1;
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// KRIPKE STRUCTURE FUNCTIONS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @return	number of states of the product structure
	 */
	inline const std::size_t getStatesCount() const {
		return states.size();
	}

	/**
	 * @param state_ID	ID of the state to get the data from
	 *
	 * @return	give state as a string
	 */
	const std::string getString(const std::size_t state_ID) const {
		// Get states numbers
		const std::size_t KS_state = getStateIndexes(state_ID).first;
		const std::size_t BA_state = getStateIndexes(state_ID).second;
		// Concat strings of subparts
		std::string state_string = std::move(structure.getString(KS_state));
		if (user_options.BA())
			state_string += std::move(automaton.getString(BA_state));

		return std::move(state_string);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// REFORMING GETTERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @return index of this combination of states in the product
	 */
	inline const std::size_t getProductIndex(const std::size_t ks_index, const std::size_t ba_index) const {
		return (ks_index * automaton.getStatesCount() + ba_index);
	}

	/**
	 * @return index of this combination of states in the product in the form (KS_state, BA_state)
	 */
	const std::pair<std::size_t, std::size_t> getStateIndexes(const std::size_t product_index) const {
		const std::size_t KS_state = product_index / automaton.getStatesCount();
		const std::size_t BA_state = product_index % automaton.getStatesCount();
		return std::make_pair(KS_state, BA_state);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @return constant reference to Kripke structure stored within the product
	 */
	const ParametrizedStructure & getKS() const {
		return structure;
	}

	/**
	 * @return constant reference to Buchi automaton stored within the product
	 */
	const AutomatonStructure & getBA() const {
		return automaton;
	}

	/**
	 * @return constant reference to structure with regulatory functions
	 */
	const FunctionsStructure & getFunc() const {
		return functions;
	}

	/**
	 * @return set of the initial states
	 */
	inline const std::vector<std::size_t> & getInitials() const {
		return initial_states;
	}

	/**
	 * @return set of final states
	 */ 
	inline const std::vector<std::size_t> & getFinals() const {
		return final_states;
	}

	/**
	 * @return set with initial states (instead of vector)
	 */
	std::set<std::size_t> getInitialUpdates() const {
		return std::set<std::size_t>(initial_states.begin(), initial_states.end());
	}

	/**
	 * @param state_num	index of the state to ask for parameters
	 * 
	 * @return parameters assigned to the state
	 */
	inline const Parameters & getParameters(const std::size_t state_num) const {
		return states[state_num].parameters;
	}

	/** 
	 * @param state_num	index of the state to ask for predecessors
	 * @param color_index	index of the color in this round (0 .. sizeof(Parameters)*8)
	 *
	 * @return predecessors for given state and color
	 */
	inline const Predecessors & getPredecessors(const std::size_t state_num, const std::size_t color_index) const {
		return states[state_num].predecessors[color_index];
	}

	/** 
	 * @param state_num	index of the state to ask for predecessors
	 *
	 * @return predecessors for given state
	 */
	inline const std::vector<Predecessors> & getPredecessors(const std::size_t state_num) const {
		return states[state_num].predecessors;
	}
};

#endif