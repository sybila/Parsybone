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
#include "automaton_interface.hpp"

class AutomatonBuilder;

class AutomatonStructure : public AutomatonInterface {
	friend class AutomatonBuilder;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Single labelled transition from one state to another
	struct Transition : public TransitionProperty {
		std::vector<std::set<std::size_t> > allowed_values; // Allowed values of species for this transition

		Transition(const StateID target_ID, std::vector<std::set<std::size_t> > && _allowed_values)
			: TransitionProperty(target_ID), allowed_values(std::move(_allowed_values)) {}
	};

	// Storing a single state - its activation levels of each of the species and IDs of states that are neighbours (differ only in single step of single value)
	struct State : public StateProperty<Transition> {
		bool final; // true if this state is final

		State(const StateID ID, const bool _final) : StateProperty<Transition>(ID), final(_final) {}
	};

	// Storage of the actuall states
	std::vector<State> states;

	// Information
	std::vector<StateID> initial_states;
	std::vector<StateID> final_states;
		
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FILLING FUNCTIONS (can be used only from AutomatonStructureBuilder)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Add a new transition - having a source, target and permitted values for each specie
	 */
	inline void addTransition(const StateID source_state, const StateID target_state, std::vector<std::set<std::size_t> > && allowed_values) {
		states[source_state].transitions.push_back(std::move(Transition(target_state, std::move(allowed_values))));
	}

	/**
	 * @param final	if true than state with index equal to the one of this vector is final
	 */
	inline void addState(const StateID ID, const bool final) {
		states.push_back(std::move(State(ID, final)));
		if (ID == 0) 
			initial_states.push_back(ID);
		if (final)
			final_states.push_back(ID);
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
	 * @override
	 */
	inline const std::size_t getStateCount() const {
		return states.size();
	}

	/**
	 * @override
	 */
	inline const std::size_t getTransitionCount(const StateID ID) const {
		return states[ID].transitions.size();
	}
	
	/**
	 * @override
	 */
	inline const std::size_t getTargetID(const StateID ID, const std::size_t transition_num) const {
		return states[ID].transitions[transition_num].target_ID;
	}

	/**
	 * Return string representing the state in the form: (ID).
	 * @override
	 */
	const std::string getString(const StateID ID) const {
		std::string state_string;
		state_string = "(";
		state_string += boost::lexical_cast<std::string, std::size_t>(ID);
		state_string += ")";
		return std::move(state_string);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BUCHI AUTOMATON FUNCTIONS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @override
	 */
	virtual inline const bool isFinal(const StateID ID) const {
		return states[ID].final;
	}

	/**
	 * Only the first state is considered initial.
	 * @override
	 */
	virtual inline const bool isInitial(const StateID ID) const {
		return (ID == 0);
	}

	/**
	 * @override
	 */
	virtual inline const std::vector<StateID> & getFinalStates() const {
		return final_states;
	}

	/**
	 * Only the first state is considered initial.
	 * @override
	 */
	virtual inline const std::vector<StateID> & getInitialStates() const {
		return initial_states;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OTHER CONSTANT GETTERS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @param transition_num	number of transition to get the data from
	 *
	 * @return	ID of the target state of this transition
	 */
	inline const std::vector<std::set<std::size_t> > & getAllowedValues(const StateID ID, const std::size_t transition_num) const {
		return states[ID].transitions[transition_num].allowed_values;
	}
};

#endif
