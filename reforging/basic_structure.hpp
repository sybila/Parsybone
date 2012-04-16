/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_BASIC_STRUCTURE_INCLUDED
#define PARSYBONE_BASIC_STRUCTURE_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BasicStructure stores states of the Kripke structure created from the model - each state knows its levels and indexes of all the neighbours.
// Order of neighbours of state is (specie 1 down, specie 1 stay, specie 1 up, specie 2 down, ... )
// BasicStructure data can be set only form the BasicStructureBuilder object.
// Rest of the code can access the data only via constant getters.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../auxiliary/output_streamer.hpp"
#include "../reforging/graph_interface.hpp"

class BasicStructureBuilder;

class BasicStructure : public GraphInterface {
	friend class BasicStructureBuilder;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Stores an unlabelled transition to next state 
	struct Transition {
		std::size_t target_ID; // index in the vector of states
		std::size_t changed_specie; // ID of specie that differs between this and neighbour
		Direction change_direction; // way the specie's value is changed

		Transition(std::size_t _target_ID, std::size_t _changed_specie, Direction _change_direction)
			: target_ID(_target_ID), changed_specie(_changed_specie), change_direction(_change_direction) { }
	};
	
	// Storing a single state - its activation levels of each of the species and IDs of states that are neighbours (differ only in single step of single value)
	struct State { 
		std::size_t ID; // unique ID of the state
		Levels species_level; // species_level[i] = activation level of specie i

		std::vector<Transition> transitions; // Indexes of the neigbourging BasicStates - all those whose levels change only in one step of a single value

		State(std::size_t _ID, Levels _species_level) 
			: ID(_ID), species_level(_species_level) { }
	};
	
	std::vector<State> states;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FILLING FUNCTIONS (can be used only from BasicStructureBuilder)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	/**
	 * Add a new state to the structure - structure consist of ID, its levels and IDs of neighbours.
	 */
	inline void addState(const std::size_t _ID, const  Levels _species_level) {
		states.push_back(State(_ID, _species_level));
	}

	/**
	 * Add a new neighbour to the target state
	 */
	inline void addNeighbour(const std::size_t state_ID, std::size_t target_ID, const std::size_t changed_specie, const Direction change_direction) {
		states[state_ID].transitions.push_back(std::move(Transition(target_ID, changed_specie, change_direction)));
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OTHER FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	BasicStructure(const BasicStructure & other);            // Forbidden copy constructor.
	BasicStructure& operator=(const BasicStructure & other); // Forbidden assignment operator.

public:
	BasicStructure() {} // Default empty constructor, needed to create an empty object that will be filled

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// KRIPKE STRUCTURE FUNCTIONS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @return	size of the state space
	 */
	const std::size_t getStateCount() const { 
		return states.size(); 
	}

	/**
	 * @param ID	ID of the state to get value from
	 *
	 * @return	levels of the state
	 */
	const std::size_t getTransitionCount(const std::size_t ID) const { 
		return states[ID].transitions.size(); 
	}

	/**
	 * @param this_ID	ID of the state to get the neighbour from
	 * @param trans_number	index in the vector of transitions
	 *
	 * @return	ID of the requested neighbour
	 */
	const std::size_t getTargetID(const std::size_t this_ID, const std::size_t trans_number) const {
		return states[this_ID].transitions[trans_number].target_ID;
	}

	const std::string getString(const std::size_t state_ID) const {
		return "";
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OTHER CONSTANT GETTERS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @param ID	ID of the state to get
	 *
	 * @return	levels of the state
	 */
	inline const Levels & getStateLevels(const std::size_t ID) const { 
		return states[ID].species_level; 
	}

	/**
	 * @param this_ID	ID of the state to get the neighbour from
	 * @param neighbour_index	index in the vector of neighbours
	 *
	 * @return	ID of the specie that vary between the two states
	 */
	inline const std::size_t getSpecieID(const std::size_t this_ID, const std::size_t neighbour_index) const {
		return states[this_ID].transitions[neighbour_index].changed_specie;
	}

	/**
	 * @param this_ID	ID of the state to get the neighbour from
	 * @param neighbour_index	index in the vector of neighbours
	 *
	 * @return	Direction in which the specie changes
	 */
	inline const Direction getDirection(const std::size_t this_ID, const std::size_t neighbour_index) const {
		return states[this_ID].transitions[neighbour_index].change_direction;
	}
};

#endif