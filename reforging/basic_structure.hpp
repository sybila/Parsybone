/*
 * Copyright (C) 2012 - Adam Streck
 *
 * This file is part of PoSeIDoN (Parameter Synthetizer for Discrete Networks) verification tool
 *
 * Poseidon is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Poseidon is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 *
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno.
 * See http://sybila.fi.muni.cz/ .
 */

#ifndef POSEIDON_BASIC_STRUCTURE_INCLUDED
#define POSEIDON_BASIC_STRUCTURE_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BasicStructure stores states of the Kripke structure created from the model - each state knows its levels and indexes of all the neighbours.
// Order of neighbours of state is (specie 1 down, specie 1 stay, specie 1 up, specie 2 down, ... )
// BasicStructure data can be set only form the BasicStructureBuilder object.
// Rest of the code can access the data only via constant getters.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../auxiliary/data_types.hpp"

class BasicStructureBuilder;

class BasicStructure {
	friend class BasicStructureBuilder;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Stores a state that can be possible succesor of this state
	struct Neighbour {
		std::size_t target_ID; // index in the vector of states
		std::size_t changed_specie; // ID of specie that differs between this and neighbour
		Direction change_direction; // way the specie's value is changed

		Neighbour(std::size_t _target_ID, std::size_t _changed_specie, Direction _change_direction)
			: target_ID(_target_ID), changed_specie(_changed_specie), change_direction(_change_direction) { }
	};
	
	// Storing a single state - its activation levels of each of the species and IDs of states that are neighbours (differ only in single step of single value)
	struct BasicState { 
		std::size_t ID; // unique ID of the node
		Levels species_level; // species_level[i] = activation level of specie i
		std::vector<Neighbour> neighbours; // Indexes of the neigbourging BasicStates - all those whose levels change only in one step of a single value

		BasicState(std::size_t _ID, Levels _species_level) 
			: ID(_ID), species_level(_species_level) { }
	};
	
	std::vector<BasicState> states;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FILLING FUNCTIONS (can be used only from BasicStructureBuilder)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	/**
	 * Add a new state to the structure - structure consist of ID, its levels and IDs of neighbours.
	 */
	inline void addState(std::size_t _ID, Levels _species_level) {
		states.push_back(BasicState(_ID, _species_level));
	}

	/**
	 * Add a new neighbour to the target state
	 */
	inline void addNeighbour(std::size_t state_ID, std::size_t target_ID, std::size_t changed_specie, Direction change_direction) {
		states[state_ID].neighbours.push_back(std::move(Neighbour(target_ID, changed_specie, change_direction)));
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OTHER FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	BasicStructure(const BasicStructure & other);            // Forbidden copy constructor.
	BasicStructure& operator=(const BasicStructure & other); // Forbidden assignment operator.

public:
	BasicStructure() {} // Default empty constructor, needed to create an empty object that will be filled
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	 * @return	size of the state space
	 */
	inline const std::size_t getStateCount() const { 
		return states.size(); 
	}

	/**
	 * @param ID	ID of the state to get
	 *
	 * @return	levels of the state
	 */
	inline const Levels & getStateLevels(const std::size_t ID) const { 
		return states[ID].species_level; 
	}

	/**
	 * @param ID	ID of the state to get value from
	 *
	 * @return	levels of the state
	 */
	inline const std::size_t getNeighboursCount(const std::size_t ID) const { 
		return states[ID].neighbours.size(); 
	}


	/**
	 * @param this_ID	ID of the state to get the neighbour from
	 * @param neighbour_index	index in the vector of neighbours
	 *
	 * @return	ID of the requested neighbour
	 */
	inline const std::size_t getNeighbourID(const std::size_t this_ID, const std::size_t neighbour_index) const {
		return states[this_ID].neighbours[neighbour_index].target_ID;
	}

	/**
	 * @param this_ID	ID of the state to get the neighbour from
	 * @param neighbour_index	index in the vector of neighbours
	 *
	 * @return	ID of the specie that vary between the two states
	 */
	inline const std::size_t getSpecieID(const std::size_t this_ID, const std::size_t neighbour_index) const {
		return states[this_ID].neighbours[neighbour_index].changed_specie;
	}

	/**
	 * @param this_ID	ID of the state to get the neighbour from
	 * @param neighbour_index	index in the vector of neighbours
	 *
	 * @return	Direction in which the specie changes
	 */
	inline const Direction getDirection(const std::size_t this_ID, const std::size_t neighbour_index) const {
		return states[this_ID].neighbours[neighbour_index].change_direction;
	}
};

#endif