/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_BASIC_STRUCTURE_INCLUDED
#define PARSYBONE_BASIC_STRUCTURE_INCLUDED

#include "../auxiliary/output_streamer.hpp"
#include "../auxiliary/common_functions.hpp"
#include "graph_interface.hpp"

/// Stores an unlabelled transition to next state
struct BasTransition : public TransitionProperty {
   std::size_t changed_specie; ///< ID of specie that differs between this and neighbour
   Direction change_direction; ///< way the specie's value is changed

   BasTransition(const StateID target_ID, const std::size_t _changed_specie, const Direction _change_direction)
      : TransitionProperty(target_ID), changed_specie(_changed_specie), change_direction(_change_direction) { } ///< Simple filler, assigns values to all the variables
};

/// Storing a single state - its activation levels of each of the species and IDs of states that are neighbours (differ only in single step of single value)
struct BasState : public StateProperty<BasTransition> {
	Levels species_level; ///< species_level[i] = activation level of specie i

	BasState(const StateID ID, const Levels _species_level, const std::string && label)
		: StateProperty<BasTransition>(ID, std::move(label)), species_level(_species_level) { }  ///< Simple filler, assigns values to all the variables
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// BasicStructure stores states of the Kripke structure created from the model - each state knows its levels and indexes of all the neighbours.
/// Order of neighbours of state is (specie 1 down, specie 1 stay, specie 1 up, specie 2 down, ... )
/// BasicStructure data can be set only form the BasicStructureBuilder object.
/// Rest of the code can access the data only via constant getters.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class BasicStructure : public GraphInterface<BasState> {
	friend class BasicStructureBuilder;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FILLING FUNCTIONS (can be used only from BasicStructureBuilder)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Add a new state, with its ID, levels and label
	 */
	inline void addState(const StateID ID, const Levels& species_level, const std::string && label) {
		states.push_back(BasState(ID, species_level, std::move(label)));
	}

	/**
	 * Add a new neighbour to the target state
	 */
	inline void addNeighbour(const StateID ID, StateID target_ID, const std::size_t changed_specie, const Direction change_direction) {
		states[ID].transitions.push_back(std::move(BasTransition(target_ID, changed_specie, change_direction)));
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OTHER FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	BasicStructure(const BasicStructure & other); ///< Forbidden copy constructor.
	BasicStructure& operator=(const BasicStructure & other); ///< Forbidden assignment operator.

public:
	BasicStructure() {} ///< Default empty constructor, needed to create an empty object that will be filled

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OTHER CONSTANT GETTERS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @param ID	ID of the state to get
	 *
	 * @return	levels of the state
	 */
	inline const Levels & getStateLevels(const StateID ID) const { 
		return states[ID].species_level; 
	}

	/**
	 * @param ID	ID of the state to get the neighbour from
	 * @param neighbour_index	index in the vector of neighbours
	 *
	 * @return	ID of the specie that vary between the two states
	 */
	inline const std::size_t getSpecieID(const StateID ID, const std::size_t neighbour_index) const {
		return states[ID].transitions[neighbour_index].changed_specie;
	}

	/**
	 * @param ID	ID of the state to get the neighbour from
	 * @param neighbour_index	index in the vector of neighbours
	 *
	 * @return	Direction in which the specie changes
	 */
	inline const Direction getDirection(const StateID ID, const std::size_t neighbour_index) const {
		return states[ID].transitions[neighbour_index].change_direction;
	}
};

#endif
