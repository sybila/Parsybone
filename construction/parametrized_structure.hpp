/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_PARAMETRIZED_STRUCTURE_INCLUDED
#define PARSYBONE_PARAMETRIZED_STRUCTURE_INCLUDED

#include "../auxiliary/output_streamer.hpp"
#include "graph_interface.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ParametrizedStructure stores states of the Kripke structure created from the model together with labelled transitions.
/// Each transition contains a function that causes it with explicit enumeration of values from the function that are transitive.
/// To easily search for the values in the parameter bitmask, step_size of the function is added
/// - that is the value saying how many bits of mask share the the same value for the function.
/// ParametrizedStructure data can be set only form the ParametrizedStructureBuilder object.
/// Rest of the code can access the data only via constant getters - once the data are parse, model remains constant.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ParametrizedStructure : public GraphInterface {
	friend class ParametrizedStructureBuilder;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// Storing a single transition to neighbour state together with its transition function
	struct Transition : public TransitionProperty {
		std::size_t step_size; ///< How many bits of a parameter space bitset is needed to get from one targe value to another
		std::vector<bool> transitive_values; ///< Which values from the original set does not allow a trasition and therefore removes bits from the mask.

		Transition(const StateID target_ID, const std::size_t _step_size, std::vector<bool>&& _transitive_values)
			: TransitionProperty(target_ID), step_size(_step_size), transitive_values(std::move(_transitive_values)) {} ///< Simple filler, assigns values to all the variables
	};

	/// Simple state enriched with transition functions
	struct State : public StateProperty<Transition> {
		Levels species_level; ///< species_level[i] = activation level of specie i

		State(const StateID ID, const Levels& _species_level)
			: StateProperty<Transition>(ID), species_level(_species_level) { } ///< Simple filler, assigns values to all the variables
	};

	/// Vector of all the states of the PKS
	std::vector<State> states;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FILLING FUNCTIONS (can be used only from ParametrizedStructureBuilder)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	 * Add a new state, only with ID and levels
	 */
	inline void addState(const StateID ID, const Levels& species_level) {
		states.push_back(State(ID, species_level));
	}

	/**
	 * @param ID	add data to the state with this IS
	 *
	 * Add a new transition to the source specie, containg necessary edge labels for the CMC
	 */
	inline void addTransition(const StateID ID, const StateID target_ID, const std::size_t step_size, std::vector<bool>&& transitive_values) {
		states[ID].transitions.push_back(Transition(target_ID, step_size, std::move(transitive_values)));
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OTHER FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ParametrizedStructure(const ParametrizedStructure & other); ///<  Forbidden copy constructor.
	ParametrizedStructure& operator=(const ParametrizedStructure & other); ///<  Forbidden assignment operator.

public:
	ParametrizedStructure() {} ///<  Default empty constructor

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// KRIPKE STRUCTURE FUNCTIONS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline const std::size_t getStateCount() const {
		return states.size();
	}

	inline const std::size_t getTransitionCount(const StateID ID) const {
		return states[ID].transitions.size();
	}

	inline const std::size_t getTargetID(const StateID ID, const std::size_t transtion_num) const {
		return states[ID].transitions[transtion_num].target_ID;
	}

	/**
	 * Return string representing given state in the form (specie1_val, specie2_val, ...)
	 */
	const std::string getString(const StateID ID) const {
		std::string state_string = "(";
		// Add species levels
		for (auto spec_it = getStateLevels(ID).begin(); spec_it != getStateLevels(ID).end() - 1; spec_it++) {
			state_string += boost::lexical_cast<std::string, std::size_t>(*spec_it);
			state_string += ",";
		}
		// Add the last species level
		state_string += boost::lexical_cast<std::string, std::size_t>(getStateLevels(ID).back());
		// End the state
		state_string += ")";
		return std::move(state_string);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OTHER CONSTANT GETTERS 
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
	inline const std::size_t getStepSize(const StateID ID, const std::size_t transtion_num) const {
		return states[ID].transitions[transtion_num].step_size;
	}

	/**
	 * @param ID	ID of the state to get the data from
	 * @param transition_num	index of the transition to get the data from
	 *
	 * @return	target values that are includete in non-transitive parameters that have to be removed
	 */
	inline const std::vector<bool> & getTransitive(const StateID ID, const std::size_t transtion_num) const {
		return states[ID].transitions[transtion_num].transitive_values;
	}
};

#endif
