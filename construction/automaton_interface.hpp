/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_AUTOMATON_INTERFACE_INCLUDED
#define PARSYBONE_AUTOMATON_INTERFACE_INCLUDED

#include "graph_interface.hpp"

/// A state structure enhanced with information whether it is final and/or initial.
template <typename Transition>
struct AutomatonStateProperty : public StateProperty<Transition> {
	bool initial; ///< True if the state is initial
	bool final; ///< true if this state is final, false otherwise

   /**
    * Adds information if the state is final or initial, passes the rest
    */
   AutomatonStateProperty<Transition>(const bool _initial, const bool _final, const StateID ID, const std::string && label)
      : StateProperty<Transition>(ID, std::move(label)), initial(_initial), final(_final) { }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Interface for all the classes that represent a Buchi automaton.
/// Buchi automaton is based on a directed graph.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename StateT>
class AutomatonInterface : public GraphInterface<StateT> {
protected:
	std::vector<StateID> initial_states; ///< Vector with indexes of initial states (in this case only the first state)
	std::vector<StateID> final_states; ///< Vector with indexes of final states of the BA

public:
	/**
	 * For given state find out if it is marked as final.
	 *
	 * @param ID	state to test
	 *
	 * @return	true if the state is final
	 */
	virtual inline const bool isFinal(const StateID ID) const {
		return states[ID].final;
	}

	/**
	 * For given state find out if it is marked as initial.
	 *
	 * @param ID	state to test
	 *
	 * @return	true if the state is initial
	 */
	virtual inline const bool isInitial(const StateID ID) const {
		return states[ID].initial;
	}

	/**
	 * Get IDs of all states that are marked as final.
	 *
	 * @return vector of final states' IDs
	 */
	virtual inline const std::vector<StateID> & getFinalStates() const {
		return final_states;
	}

	/**
	 * Get IDs of all states that are marked as initial.
	 *
	 * @return vector of initial states' IDs
	 */
	virtual inline const std::vector<StateID> & getInitialStates() const {
		return initial_states;
	}
};

#endif
