/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_UNPARAMETRIZED_STRUCTURE_INCLUDED
#define PARSYBONE_UNPARAMETRIZED_STRUCTURE_INCLUDED

#include "../auxiliary/output_streamer.hpp"
#include "PunyHeaders/common_functions.hpp"
#include "graph_interface.hpp"

/// Storing a single transition to neighbour state together with its transition function.
struct ParTransitionion : public TransitionProperty {
	ParamNum step_size; ///< How many bits of a parameter space bitset is needed to get from one targe value to another.
   vector<bool> transitive_values; ///< Which values from the original set do not allow a trasition and therefore removes bits from the mask.

   ParTransitionion(const StateID target_ID, const ParamNum _step_size, vector<bool>&& _transitive_values)
      : TransitionProperty(target_ID), step_size(_step_size), transitive_values(move(_transitive_values)) {} ///< Simple filler, assigns values to all the variables.
};

/// Simple state enriched with transition functions
struct ParState : public StateProperty<ParTransitionion> {
   Levels species_level; ///< Species_level[i] = activation level of specie i.

	ParState(const StateID ID, const Levels& _species_level, const string && label)
      : StateProperty<ParTransitionion>(ID, move(label)), species_level(_species_level) { } ///< Simple filler, assigns values to all the variables.
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Complete Kripke structure with only possible transitions containing encoded kinetic functions.
///
/// UnparametrizedStructure stores states of the Kripke structure created from the model together with labelled transitions.
/// Each transition contains a function that causes it with explicit enumeration of values from the function that are transitive.
/// To easily search for the values in the parameter bitmask, step_size of the function is added
/// - that is the value saying how many bits of mask share the the same value for the function.
/// UnparametrizedStructure data can be set only from the UnparametrizedStructureBuilder object.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class UnparametrizedStructure : public GraphInterface<ParState> {
	friend class UnparametrizedStructureBuilder;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FILLING METHODS (can be used only from UnparametrizedStructureBuilder)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Add a new state, only with ID and levels
	 */
	inline void addState(const StateID ID, const Levels& species_level, const string label) {
		states.push_back(ParState(ID, species_level, move(label)));
	}

	/**
	 * @param ID	add data to the state with this IS
	 *
	 * Add a new transition to the source specie, containg necessary edge labels for the CMC
	 */
	inline void addTransition(const StateID ID, const StateID target_ID, const ParamNum step_size, vector<bool>&& transitive_values) {
		states[ID].transitions.push_back(ParTransitionion(target_ID, step_size, move(transitive_values)));
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OTHER METHODS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   UnparametrizedStructure(const UnparametrizedStructure & other); ///< Forbidden copy constructor.
   UnparametrizedStructure& operator=(const UnparametrizedStructure & other); ///< Forbidden assignment operator.

public:
   UnparametrizedStructure() {} ///<  Default empty constructor

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @param ID	ID of the state to get the data from
	 *
	 * @return	species level 
	 */
	inline const Levels & getStateLevels(const StateID ID) const {
		return states[ID].species_level;
	}

	/**
	 * @param ID	ID of the state to get the data from
	 * @param transition_num	index of the transition to get the data from
	 *
	 * @return	number of neighbour parameters that share the same value of the function
	 */
	inline ParamNum getStepSize(const StateID ID, const size_t transtion_num) const {
		return states[ID].transitions[transtion_num].step_size;
	}

	/**
	 * @param ID	ID of the state to get the data from
	 * @param transition_num	index of the transition to get the data from
	 *
	 * @return	target values that are includete in non-transitive parameters that have to be removed
	 */
	inline const vector<bool> & getTransitive(const StateID ID, const size_t transtion_num) const {
		return states[ID].transitions[transtion_num].transitive_values;
	}
};

#endif // PARSYBONE_UNPARAMETRIZED_STRUCTURE_INCLUDED
