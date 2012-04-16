/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_PARAMETRIZED_STRUCTURE_INCLUDED
#define PARSYBONE_PARAMETRIZED_STRUCTURE_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ParametrizedStructure stores states of the Kripke structure created from the model together with labelled transitions.
// Each transition contains a function that causes it with explicit enumeration of values from the function that are transitive.
// To easily search for the values in the parameter bitmask, step_size of the function is added 
// - that is the value saying how many bits of mask share the the same value for the function.
// ParametrizedStructure data can be set only form the ParametrizedStructureBuilder object.
// Rest of the code can access the data only via constant getters - once the data are parse, model remains constant.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../auxiliary/output_streamer.hpp"
#include "../reforging/graph_interface.hpp"

class ParametrizedStructureBuilder;

class ParametrizedStructure : public GraphInterface {
	friend class ParametrizedStructureBuilder;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Storing a single transition to neighbour state together with its transition function
	struct Transition {
		std::size_t target_ID; // ID of the state the transition leads to
		std::size_t function_ID; // ID of the function active in this transition - not used in the program itself
		std::size_t step_size; // How many bits of a parameter space bitset is needed to get from one targe value to another
		std::vector<bool> transitive_values; // Which values from the original set does not allow a trasition and therefore removes bits from the mask.

		Transition(const std::size_t _target_ID, const std::size_t _function_ID, const std::size_t _step_size, std::vector<bool>&& _transitive_values)
			: target_ID(_target_ID), function_ID(_function_ID), step_size(_step_size), transitive_values(std::move(_transitive_values)) {}
	};
	
	// Simple state enriched with transition functions
	struct State {
		std::size_t ID; // unique ID of the state
		Levels species_level; // species_level[i] = activation level of specie i
		std::vector<Transition> transitions; // Indexes of the neigbourging BasicStates - all those whose levels change only in one step of a single value

		State(const std::size_t _ID, const Levels& _species_level) 
			: ID(_ID), species_level(_species_level) { }
	};

	std::vector<State> states;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FILLING FUNCTIONS (can be used only from ParametrizedStructureBuilder)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	 * Add a new state, only with ID and levels
	 */
	inline void addState(const std::size_t _ID, const Levels& _species_level) {
		states.push_back(State(_ID, _species_level));
	}

	/**
	 * @param ID	add data to the state with this IS
	 *
	 * Add a new transition with all its values
	 */
	inline void addTransition(const std::size_t ID, const std::size_t _target_ID, const std::size_t _function_ID, const std::size_t _step_size, std::vector<bool>&& _transitive_values) {
		states[ID].transitions.push_back(Transition(_target_ID, _function_ID, _step_size, std::move(_transitive_values)));
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OTHER FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ParametrizedStructure(const ParametrizedStructure & other);            // Forbidden copy constructor.
	ParametrizedStructure& operator=(const ParametrizedStructure & other); // Forbidden assignment operator.

public:
	ParametrizedStructure() {} // Default empty constructor

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// KRIPKE STRUCTURE FUNCTIONS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @override
	 */
	inline const std::size_t getStateCount() const {
		return states.size();
	}

	/**
	 * @override
	 */
	inline const std::size_t getTransitionCount(const std::size_t state_ID) const {
		return states[state_ID].transitions.size();
	}

	/**
	 * @override
	 */
	inline const std::size_t getTargetID(const std::size_t state_ID, const std::size_t transtion_num) const {
		return states[state_ID].transitions[transtion_num].target_ID;
	}

	/**
	 * @override
	 * Return string representing given state in the form (specie1_val, specie2_val, ...)
	 */
	const std::string getString(const std::size_t state_ID) const {
		std::string state_string = "(";
		// Add species levels
		for (auto spec_it = getStateLevels(state_ID).begin(); spec_it != getStateLevels(state_ID).end() - 1; spec_it++) {
			state_string += boost::lexical_cast<std::string, std::size_t>(*spec_it);
			state_string += ",";
		}
		// Add the last species level
		state_string += boost::lexical_cast<std::string, std::size_t>(getStateLevels(state_ID).back());
		// End the state
		state_string += ")";
		return std::move(state_string);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OTHER CONSTANT GETTERS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @param state_ID	ID of the state to get the data from
	 *
	 * @return	species level 
	 */
	inline const Levels & getStateLevels(const std::size_t state_ID) const {
		return states[state_ID].species_level;
	}

	/**
	 * @param state_ID	ID of the state to get the data from
	 * @param transition_num index of the transition to get the data from
	 *
	 * @return	ID of the target of the transition
	 */
	inline const std::size_t getFunctionID(const std::size_t state_ID, const std::size_t transtion_num) const {
		return states[state_ID].transitions[transtion_num].function_ID;
	}

	/**
	 * @param state_ID	ID of the state to get the data from
	 * @param transition_num index of the transition to get the data from
	 *
	 * @return	number of neighbour parameters that share the same value of the function
	 */
	inline const std::size_t getStepSize(const std::size_t state_ID, const std::size_t transtion_num) const {
		return states[state_ID].transitions[transtion_num].step_size;
	}

	/**
	 * @param state_ID	ID of the state to get the data from
	 * @param transition_num index of the transition to get the data from
	 *
	 * @return	target values that are includete in non-transitive parameters that have to be removed
	 */
	inline const std::vector<bool> & getTransitive(const std::size_t state_ID, const std::size_t transtion_num) const {
		return states[state_ID].transitions[transtion_num].transitive_values;
	}
};

#endif