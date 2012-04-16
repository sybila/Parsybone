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
	struct Transition {
	};
	
	struct State {
		std::size_t ID;
		std::size_t BA_ID;
		std::size_t KS_ID;

		std::vector<Transition> transitions;
	};
	
	// References to data structures
	const FunctionsStructure & functions; // Implicit reprezentation of functions - used as reference
	const ParametrizedStructure & structure; // Stores info about KS states
	const AutomatonStructure & automaton; // Stores info about BA states

	// Information about states
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
		return (ks_index * automaton.getStateCount() + ba_index);
	}

	/**
	 * @return index of this combination of states in the product in the form (KS_state, BA_state)
	 */
	const std::pair<std::size_t, std::size_t> getStateIndexes(const std::size_t product_index) const {
		const std::size_t KS_state = product_index / automaton.getStateCount();
		const std::size_t BA_state = product_index % automaton.getStateCount();
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
	 * @return vector of the initial states
	 */
	inline const std::vector<std::size_t> & getInitialStates() const {
		return initial_states;
	}

	/**
	 * @return set of final states
	 */ 
	inline const std::vector<std::size_t> & getFinalStates() const {
		return final_states;
	}

	/**
	 * @return set with initial states (instead of vector)
	 */
	std::set<std::size_t> getInitialUpdates() const {
		return std::set<std::size_t>(initial_states.begin(), initial_states.end());
	}
};

#endif