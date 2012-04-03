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

#ifndef PARSYBONE_AUTOMATON_STRUCTURE_INCLUDED
#define PARSYBONE_AUTOMATON_STRUCTURE_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AutomatonStructure transitions of the BA into implicit form with dependencies on the model.
// Each transition knows the levels of the genes that are required for it to be feasible.
// Transitions are ordered by order of their source states - precise positions are stored in states_begin vector.
// AutomatonStructure data can be set only form the AutomatonStructureBuilder object.
// Rest of the code can access the data only via constant getters.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../auxiliary/data_types.hpp"
#include "../auxiliary/output_streamer.hpp"

class AutomatonBuilder;

class AutomatonStructure {
	friend class AutomatonBuilder;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Storing a single state - its activation levels of each of the species and IDs of states that are neighbours (differ only in single step of single value)
	struct Transition { 
		std::size_t source_state; // From where
		std::size_t target_state; // To where
		std::vector<std::set<std::size_t> > allowed_values; // Allowed values of species for this transition

		Transition(std::size_t _source_state, std::size_t _target_state, std::vector<std::set<std::size_t> > && _allowed_values) 
			: source_state(_source_state), target_state(_target_state), allowed_values(std::move(_allowed_values)) {}
	};
	
	std::vector<Transition> transitions;

	// Auxiliary data
	std::vector<bool> final_states; // For each state stores whether it is final or not
	std::vector<std::size_t> states_begin; // states_begin[i] = Position in transitions vector at which transition from the state i begins
	
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FILLING FUNCTIONS (can be used only from AutomatonStructureBuilder)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Add a new transition - having a source, target and permitted values for each specie
	 */
	inline void addTransition(std::size_t _source_state, std::size_t _target_state, std::vector<std::set<std::size_t> > && _allowed_values) {
		transitions.push_back(Transition(_source_state, _target_state, std::move(_allowed_values)));
	}
	
	/**
	 * @param position	number of the first transition that leads from the state with index equal to that of the vector
	 */
	inline void addStateBegin(const std::size_t position) {
		states_begin.push_back(position);
	}

	/**
	 * @param final	if true than state with index equal to the one of this vector is final
	 */
	inline void addFinality(const bool final) {
		final_states.push_back(final);
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
	bool isTransitionFeasible(const std::size_t transition_num, const Levels & levels) const {
		const Transition & transition = transitions[transition_num];
		// Cycle through the sates
		for (std::size_t specie_num = 0; specie_num < transition.allowed_values.size(); specie_num++) {
			// If you do not find current specie level between allowed, return false
			if (transition.allowed_values[specie_num].find(levels[specie_num]) == transition.allowed_values[specie_num].end())
				return false;
		}
		return true;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @return	number of the states
	 */
	inline const std::size_t getStatesCount() const {
		return final_states.size();
	}

	/**
	 * @param state_ID	ID of the state to get the data from
	 *
	 * @return	true if the state is final
	 */
	inline const bool isFinal(const std::size_t state_ID) const {
		return final_states[state_ID];
	}

	/**
	 * @param state_ID	ID of the state to get the data from
	 *
	 * @return	index in transitions where transitions with the source with this ID start
	 */
	inline const std::size_t getBeginIndex(const std::size_t state_ID) const {
		return states_begin[state_ID];
	}

	/**
	 * @return	number of the transitions
	 */
	inline const std::size_t getTransitionsCount() const {
		return transitions.size();
	}

	/**
	 * @param transition_num	number of transition to get the data from
	 *
	 * @return	ID of the source state of this transition
	 */
	inline const std::size_t getSource(const std::size_t transition_num) const {
		return transitions[transition_num].source_state;
	}

	/**
	 * @param transition_num	number of transition to get the data from
	 *
	 * @return	ID of the target state of this transition
	 */
	inline const std::size_t getTarget(const std::size_t transition_num) const {
		return transitions[transition_num].target_state;
	}

	/**
	 * @param transition_num	number of transition to get the data from
	 *
	 * @return	ID of the target state of this transition
	 */
	inline const std::vector<std::set<std::size_t> > & getAllowedValues(const std::size_t transition_num) const {
		return transitions[transition_num].allowed_values;
	}
};

#endif