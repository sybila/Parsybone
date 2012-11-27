/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_GRAPH_INTERFACE_INCLUDED
#define PARSYBONE_GRAPH_INTERFACE_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file This file holds abstract basis for a graph with states and transitions between the states.
/// Graph is parametrized by state and state is parametrized by transition. Basic state and transition are also defined here.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief This is just a very simple basis for a transition in a graph.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct TransitionProperty {
   /// Unique ID of the state.
   StateID target_ID;

   /**
    * Basic constructor fills in the ID.
    */
   TransitionProperty(StateID _target_ID) : target_ID(_target_ID) { }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief This is just a very simple basis for a state of any graph.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename Transition>
struct StateProperty {
   /// Unique ID of the state.
   StateID ID;
   /// Label of the state (usually a number or series of numbers) descibing the state.
   string label;
   /// Graph or automaton transitions, basically it is an edge with a label.
   vector<Transition> transitions;

   /**
    * Basic constructor that fills in the ID and label.
    */
   StateProperty<Transition>(const StateID _ID, const string && _label) : ID(_ID), label(move(_label)) { }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Interface for all the classes that represent a directed graph. Transitions are expected to be stored within their source state structure.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename StateT>
class GraphInterface {
protected:
   vector<StateT> states; ///< Vector holding states of the graph.

public:
	/**
	 * Obtains number of states of the graph.
	 *
	 * @return integer with size of the graph
	 */
	inline size_t getStateCount() const {
		return states.size();
	}

	/**
	 * Obtains number of outcoming transitions for given state.
	 *
	 * @param ID	ID of the state to get the number from
	 *
	 * @return	integer with number of outcoming transitions 
	 */
	inline size_t getTransitionCount(const StateID ID) const {
		return states[ID].transitions.size();
	}

	/**
	 * Obtains ID of the target of given transition for given state.
	 *
	 * @param ID	ID of the state to get the neighbour from
	 * @param trans_number	index in the vector of transitions
	 *
	 * @return	ID of the requested target
	 */
	inline StateID getTargetID(const StateID ID, const size_t transition_number) const {
		return states[ID].transitions[transition_number].target_ID;
	}

	/**
	 * Returns given state as a string.
	 *
	 * @param ID	ID of the state to turn into the string
	 *
	 * @return	given state as a string
	 */
	inline const string & getString(const StateID ID) const {
		return states[ID].label;
	}
};

#endif // PARSYBONE_GRAPH_INTERFACE_INCLUDED
