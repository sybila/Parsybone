/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_AUTOMATON_INTERFACE_INCLUDED
#define PARSYBONE_AUTOMATON_INTERFACE_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file This file holds abstract basis for a finite automaton described as a graph with states and transitions between the states.
/// Automaton is parametrized by state and state is parametrized by transition. Automaton state is also defined here.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "graph_interface.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief A state structure enhanced with information whether the state is final and/or initial.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename Transition>
struct AutomatonStateProperty : public StateProperty<Transition> {
   bool initial; ///< True if the state is initial.
   bool final; ///< True if this state is final.

   /**
    * Adds information if the state is final or initial, passes the rest.
    */
   AutomatonStateProperty<Transition>(const bool _initial, const bool _final, const StateID ID)
      : StateProperty<Transition>(ID), initial(_initial), final(_final) { }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  \brief Interface for all the classes that represent a Buchi automaton. Buchi automaton is based on a GraphInterface.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename StateT>
class AutomatonInterface : public GraphInterface<StateT> {
protected:
   vector<StateID> initial_states; ///< Vector with indexes of initial states (in this case only the first state).
   vector<StateID> final_states; ///< Vector with indexes of final states of the BA.

public:
	/**
    * For a given state find out whether it is marked as final.
	 *
	 * @param ID	state to test
	 *
	 * @return	true if the state is final
	 */
	virtual inline bool isFinal(const StateID ID) const {
        return GraphInterface<StateT>::states[ID].final;
	}

	/**
	 * For given state find out if it is marked as initial.
	 *
	 * @param ID	state to test
	 *
	 * @return	true if the state is initial
	 */
	virtual inline bool isInitial(const StateID ID) const {
        return GraphInterface<StateT>::states[ID].initial;
	}

	/**
	 * Get IDs of all states that are marked as final.
	 *
	 * @return vector of final states' IDs
	 */
	virtual inline const vector<StateID> & getFinalStates() const {
		return final_states;
	}

	/**
	 * Get IDs of all states that are marked as initial.
	 *
	 * @return vector of initial states' IDs
	 */
	virtual inline const vector<StateID> & getInitialStates() const {
		return initial_states;
	}
};

#endif // PARSYBONE_AUTOMATON_INTERFACE_INCLUDED
