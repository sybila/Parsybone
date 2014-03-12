/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warranty. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_UNPARAMETRIZED_STRUCTURE_INCLUDED
#define PARSYBONE_UNPARAMETRIZED_STRUCTURE_INCLUDED

#include "../auxiliary/output_streamer.hpp"
#include "../auxiliary/common_functions.hpp"
#include "transition_system_interface.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Complete Kripke structure with only possible transitions containing encoded kinetic functions.
///
/// UnparametrizedStructure stores states of the Kripke structure created from the model together with labelled transitions.
/// Each transition contains a function that causes it with explicit enumeration of values from the function that are transitive.
/// To easily search for the values in the parameter bitmask, step_size of the function is added
/// - that is the value saying how many bits of mask share the the same value for the function.
/// UnparametrizedStructure data can be set only from the UnparametrizedStructureBuilder object.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class UnparametrizedStructure : public TSInterface<TSStateProperty> {
	friend class UnparametrizedStructureBuilder;
	Levels maxes; ///< Maximal activity levels of the species.
	Levels mins; ///< Minimal activity levels of the species.
	Levels range_size; ///< Differences between the two.

public:
	UnparametrizedStructure() = default;
	UnparametrizedStructure(UnparametrizedStructure &&) = default;
	UnparametrizedStructure(const UnparametrizedStructure &) = delete;
	UnparametrizedStructure& operator=(const UnparametrizedStructure &) = delete;
	UnparametrizedStructure& operator=(UnparametrizedStructure && other) {
		states = move(other.states);
		return *this;
	}

	/**
	  * Add a new state, only with ID and levels
	  */
	template <class LevelT>
	inline void addState(const StateID ID, LevelT&& species_level) {
		GraphInterface<TSStateProperty>::states.emplace_back(TSStateProperty(ID, forward<LevelT>(species_level)));
	}

	/**
	 * @param ID	add data to the state with this IS
	 * Add a new transition to the source specie, containg necessary edge labels for the CMC
	 */
	inline void addTransition(const StateID ID, const StateID target_ID, const ParamNo step_size, const bool _dir, const ActLevel level, const Levels & targets) {
		GraphInterface<TSStateProperty>::states[ID].transitions.push_back(TSTransitionProperty(target_ID, step_size, _dir, level, targets));
	}

	inline StateID getID(const Levels & levels) const {
		StateID result = 0;
		size_t factor = 1;

		for (size_t lvl_no = 0; lvl_no < levels.size(); lvl_no++) {
			result += (levels[lvl_no] - mins[lvl_no]) * factor;
			factor *= (range_size[lvl_no]);
		}

		return result;
	}
};

#endif // PARSYBONE_UNPARAMETRIZED_STRUCTURE_INCLUDED
