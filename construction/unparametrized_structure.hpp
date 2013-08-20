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
#include "basic_structure.hpp"

struct TransConst {
   ParamNo step_size; ///< How many bits of a parameter space bitset is needed to get from one targe value to another.
   bool req_dir; ///<
   ActLevel req_comp; ///<
   const Levels & targets; ///<
};

/// Storing a single transition to neighbour state together with its transition function.
struct ParTransitionion : public TransitionProperty {
   TransConst trans_const;

   ParTransitionion(const StateID target_ID, const ParamNo _step_size, const bool _req_op, const ActLevel _req_comp, const Levels & _targets)
      : TransitionProperty(target_ID), trans_const({_step_size, _req_op, _req_comp, _targets}) {}
};

/// Simple state enriched with transition functions
struct ParState : public StateProperty<ParTransitionion> {
   Levels species_level; ///< Species_level[i] = activation level of specie i.

   ParState(const StateID ID, const Levels& _species_level)
      : StateProperty<ParTransitionion>(ID), species_level(_species_level) { } ///< Simple filler, assigns values to all the variables.
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

   /**
	 * Add a new state, only with ID and levels
	 */
   inline void addState(const StateID ID, const Levels& species_level) {
      states.push_back(ParState(ID, species_level));
	}

	/**
	 * @param ID	add data to the state with this IS
	 * Add a new transition to the source specie, containg necessary edge labels for the CMC
	 */
   inline void addTransition(const StateID ID, const StateID target_ID, const ParamNo step_size,  const bool _dir, const ActLevel level, const Levels & targets) {
      states[ID].transitions.push_back(ParTransitionion(target_ID, step_size, _dir, level, targets));
	}

public:
   /**
    * @return
    */
	inline const Levels & getStateLevels(const StateID ID) const {
		return states[ID].species_level;
	}

   /**
    * @return
    */
	inline ParamNo getStepSize(const StateID ID, const size_t transtion_num) const {
      return states[ID].transitions[transtion_num].trans_const.step_size;
	}

   /**
    * @return
    */
   inline const ParTransitionion & getTransition(const StateID ID, const size_t transtion_num) const {
      return states[ID].transitions[transtion_num];
   }
};

#endif // PARSYBONE_UNPARAMETRIZED_STRUCTURE_INCLUDED
