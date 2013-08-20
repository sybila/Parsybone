/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_BASIC_STRUCTURE_INCLUDED
#define PARSYBONE_BASIC_STRUCTURE_INCLUDED

#include "PunyHeaders/common_functions.hpp"

#include "../auxiliary/output_streamer.hpp"
#include "graph_interface.hpp"

/// Stores an unlabelled transition to next state.
struct BasTransition : public TransitionProperty {
   const SpecieID changed_specie; ///< ID of specie that differs between this and neighbour.
   const bool change_direction; ///< Way the specie's value is changed.

   BasTransition(const StateID target_ID, const size_t _changed_specie, const bool _change_direction)
      : TransitionProperty(target_ID), changed_specie(_changed_specie), change_direction(_change_direction) { } ///< Simple filler, assigns values to all the variables.
};

/// Storing a single state - its activation levels of each of the species and IDs of states that are neighbours (differ only in single step of single value).
struct BasState : public StateProperty<BasTransition> {
   const Levels species_level; ///< Species_level[i] = activation level of specie i.

   BasState(const StateID ID, const Levels _species_level)
      : StateProperty<BasTransition>(ID), species_level(_species_level) { }  ///< Simple filler, assigns values to all the variables.
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief A simple structure describing the complete state space.
///
/// BasicStructure stores states of the Kripke structure created from the model - each state knows its levels and indexes of all the neighbours.
/// Order of neighbours of state is (specie 1 down, specie 1 stay, specie 1 up, specie 2 down, ... )
/// BasicStructure data can be set only form the BasicStructureBuilder object.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class BasicStructure : public GraphInterface<BasState> {
	friend class BasicStructureBuilder;

	/**
	 * Add a new state, with its ID, levels and label
	 */
   inline void addState(const StateID ID, const Levels& species_level) {
      states.push_back(BasState(ID, species_level));
	}

	/**
	 * Add a new neighbour to the target state
	 */
   inline void addNeighbour(const StateID ID, StateID target_ID, const size_t changed_specie, const bool _change_direction) {
      states[ID].transitions.push_back(move(BasTransition(target_ID, changed_specie, _change_direction)));
	}

public:
   /**
	 * @param ID	ID of the state to get
	 * @return	levels of the state
	 */
	inline const Levels & getStateLevels(const StateID ID) const { 
		return states[ID].species_level; 
	}

	/**
	 * @param ID	ID of the state to get the neighbour from
	 * @param neighbour_index	index in the vector of neighbours
	 * @return	ID of the specie that vary between the two states
	 */
	inline size_t getSpecieID(const StateID ID, const size_t neighbour_index) const {
		return states[ID].transitions[neighbour_index].changed_specie;
	}

	/**
	 * @param ID	ID of the state to get the neighbour from
	 * @param neighbour_index	index in the vector of neighbours
	 * @return	Direction in which the specie changes
	 */
   inline bool getDirection(const StateID ID, const size_t neighbour_index) const {
		return states[ID].transitions[neighbour_index].change_direction;
	}
};

#endif // PARSYBONE_BASIC_STRUCTURE_INCLUDED
