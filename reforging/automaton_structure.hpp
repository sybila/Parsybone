/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_AUTOMATON_STRUCTURE_INCLUDED
#define PARSYBONE_AUTOMATON_STRUCTURE_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AutomatonStructure transitions of the BA into implicit form with dependencies on the model.
// Each transition knows the levels of the genes that are required for it to be feasible.
// Transitions are ordered by order of their source states - precise positions are stored in states_begin vector.
// AutomatonStructure data can be set only form the AutomatonStructureBuilder object.
// Rest of the code can access the data only via constant getters.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../auxiliary/output_streamer.hpp"

class AutomatonBuilder;

class AutomatonStructure {
	friend class AutomatonBuilder;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Single labelled transition from one state to another
	struct Transition { 
		std::size_t target_state; // To where
		std::vector<std::set<std::size_t> > allowed_values; // Allowed values of species for this transition

		Transition(const std::size_t _target_state, std::vector<std::set<std::size_t> > && _allowed_values) 
			: target_state(_target_state), allowed_values(std::move(_allowed_values)) {}
	};

	// Storing a single state - its activation levels of each of the species and IDs of states that are neighbours (differ only in single step of single value)
	struct State {
		std::size_t ID; // unique ID of the state
		bool final; // true if this state is final
		std::vector<Transition> transitions; // Transitions from this state

		State(const std::size_t _ID, const bool _final) : ID(_ID), final(_final) {}
	};

	std::vector<State> states;
		
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FILLING FUNCTIONS (can be used only from AutomatonStructureBuilder)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Add a new transition - having a source, target and permitted values for each specie
	 */
	inline void addTransition(const std::size_t source_state, const std::size_t target_state, std::vector<std::set<std::size_t> > && allowed_values) {
		states[source_state].transitions.push_back(std::move(Transition(target_state, std::move(allowed_values))));
	}

	/**
	 * @param final	if true than state with index equal to the one of this vector is final
	 */
	inline void addState(const std::size_t ID, const bool final) {
		states.push_back(std::move(State(ID, final)));
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OTHER FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	AutomatonStructure(const AutomatonStructure & other);            // Forbidden copy constructor.
	AutomatonStructure& operator=(const AutomatonStructure & other); // Forbidden assignment operator.

public:
	AutomatonStructure() {} // Default empty constructor, needed to create an empty object that will be filled

	/**
	 * @param levels	current levels of species i.e. the state of the KS
	 *
	 * @return	true if the transition is feasible
	 */
	bool isTransitionFeasible(const std::size_t state_num, const std::size_t transition_num, const Levels & levels) const {
		const Transition & transition = states[state_num].transitions[transition_num];
		// Cycle through the sates
		for (std::size_t specie_num = 0; specie_num < transition.allowed_values.size(); specie_num++) {
			// If you do not find current specie level between allowed, return false
			if (transition.allowed_values[specie_num].find(levels[specie_num]) == transition.allowed_values[specie_num].end())
				return false;
		}
		return true;
	}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// KRIPKE STRUCTURE FUNCTIONS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @return	number of the states
	 */
	inline const std::size_t getStatesCount() const {
		return states.size();
	}

	/**
	 * @param state_ID	ID of the state to get the data from
	 *
	 * @return	number of the transitions
	 */
	inline const std::size_t getTransitionsCount(const std::size_t state_ID) const {
		return states[state_ID].transitions.size();
	}
	
	/**
	 * @param transition_num	number of transition to get the data from
	 *
	 * @return	ID of the target state of this transition
	 */
	inline const std::size_t getTarget(const std::size_t state_ID, const std::size_t transition_num) const {
		return states[state_ID].transitions[transition_num].target_state;
	}

	/**
	 * @param state_ID	ID of the state to get the data from
	 *
	 * @return	give state as a string
	 */
	const std::string getString(const std::size_t state_ID) const {
		std::string state_string;
		state_string = "(";
		state_string += boost::lexical_cast<std::string, std::size_t>(state_ID);
		state_string += ")";
		return std::move(state_string);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OTHER CONSTANT GETTERS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @param state_ID	ID of the state to get the data from
	 *
	 * @return	true if the state is final
	 */
	inline const bool isFinal(const std::size_t state_ID) const {
		return states[state_ID].final;
	}

	/**
	 * @param transition_num	number of transition to get the data from
	 *
	 * @return	ID of the target state of this transition
	 */
	inline const std::vector<std::set<std::size_t> > & getAllowedValues(const std::size_t state_ID, const std::size_t transition_num) const {
		return states[state_ID].transitions[transition_num].allowed_values;
	}
};

#endif