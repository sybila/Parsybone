/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_AUTOMATON_STRUCTURE_INCLUDED
#define PARSYBONE_AUTOMATON_STRUCTURE_INCLUDED

#include "../auxiliary/common_functions.hpp"
#include "../auxiliary/output_streamer.hpp"
#include "automaton_interface.hpp"

/// Single labelled transition from one state to another
struct AutTransitionion : public TransitionProperty {
	std::vector<std::set<std::size_t> > allowed_values; ///< Allowed values of species for this transition

	AutTransitionion(const StateID target_ID, std::vector<std::set<std::size_t> > && _allowed_values)
		: TransitionProperty(target_ID), allowed_values(std::move(_allowed_values)) {}  ///< Simple filler, assigns values to all the variables
};

/// Storing a single state of the B\"uchi automaton. This state is extended with a value saying wheter the states is final.
struct AutState : public AutomatonStateProperty<AutTransitionion> {

	/// Fills data and checks if the state has value  -> is initial
    AutState(const StateID ID, const bool final, std::string && label)
        : AutomatonStateProperty<AutTransitionion>((ID == 0), final, ID, std::move(label)) { }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// AutomatonStructure stores B\"uchi automaton with edges labelled by values the KS can be in for the transition to be allowed.
/// AutomatonStructure data can be set only form the AutomatonStructureBuilder object. Rest of the code can access the data only via constant getters.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class AutomatonStructure : public AutomatonInterface<AutState> {
	friend class AutomatonBuilder;
		
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FILLING FUNCTIONS (can be used only from AutomatonStructureBuilder)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Add a new transition - having a source, target and permitted values for each specie
	 */
	inline void addTransition(const StateID source_state, const StateID target_state, std::vector<std::set<std::size_t> > && allowed_values) {
		states[source_state].transitions.push_back(std::move(AutTransitionion(target_state, std::move(allowed_values))));
	}

	/**
	 * @param final	if true than state with index equal to the one of this vector is final
	 */
	inline void addState(const StateID ID, const bool final) {
		std::string label("(");
		label.append(toString(ID)).append(")");
        states.push_back(std::move(AutState(ID, final, std::move(label))));
		if (ID == 0) 
			initial_states.push_back(ID);
		if (final)
			final_states.push_back(ID);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OTHER FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	AutomatonStructure(const AutomatonStructure & other); ///< Forbidden copy constructor.
	AutomatonStructure& operator=(const AutomatonStructure & other); ///< Forbidden assignment operator.

public:
	AutomatonStructure() {} ///< Default empty constructor

	/**
	 * @param ID	source state of the transition
	 * @param transition_num	ordinal number of the transition
	 * @param levels	current levels of species i.e. the state of the KS
	 *
	 * @return	true if the transition is feasible
	 */
	bool isTransitionFeasible(const StateID ID, const std::size_t transition_num, const Levels & levels) const {
		const AutTransitionion & transition = states[ID].transitions[transition_num];
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
	 * Get a vector of values the KS can be in for the transition to be active
	 *
	 * @param ID	source state
	 * @param transition_num	number of transition to get the data from
	 *
	 * @return	vector of values the KS can occur in for the transition to be allowed
	 */
	inline const std::vector<std::set<std::size_t> > & getAllowedValues(const StateID ID, const std::size_t transition_num) const {
		return states[ID].transitions[transition_num].allowed_values;
	}
};

#endif
