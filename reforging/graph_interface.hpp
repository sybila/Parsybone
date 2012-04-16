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
// Interface for all the classes that represent a directed graph.
// Transitions are stored with their source state.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class GraphInterface {
public:
	/**
	 * Obtains number of states of the graph.
	 *
	 * @return integer with size of the graph
	 */
	virtual inline const std::size_t getStateCount() const = 0;

	/**
	 * Obtains number of transitions for given state.
	 *
	 * @param ID	ID of the state to get the number from
	 *
	 * @return	integer with number of outcoming transitions 
	 */
	virtual inline const std::size_t getTransitionCount(const StateID ID) const = 0;

	/**
	 * Obtains ID of the target of given transition for given state.
	 *
	 * @param ID	ID of the state to get the neighbour from
	 * @param trans_number	index in the vector of transitions
	 *
	 * @return	ID of the requested target
	 */
	virtual inline const StateID getTargetID(const StateID ID, const std::size_t transition_number) const = 0;

	/**
	 * Returns given state as a string.
	 *
	 * @param ID	ID of the state to turn into the string
	 *
	 * @return	given state as a string
	 */
	virtual inline const std::string getString(const std::size_t StateID) const = 0;

	/**
	 * Virtual destructor.
	 */
	virtual ~GraphInterface() {}
};

#endif