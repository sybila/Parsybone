/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_PRODUCT_INCLUDED
#define PARSYBONE_PRODUCT_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ProductStructure stores product of BA and PKS
// States are indexed as (BA_state_count * KS_state_ID + BA_state_ID) - e.g. if 3-state BA state ((1,0)x(1)) would be at position 3*1 + 1 = 4
// In other words, first iterate through BA then through KS
// ProductStructure data can be set only form the ProductBuilder object.
// ProductStructure is used for computation - meaning it has also setter / computation functions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../auxiliary/output_streamer.hpp"
#include "../coloring/parameters_functions.hpp"
#include "../reforging/automaton_structure.hpp"
#include "../reforging/functions_structure.hpp"
#include "../reforging/parametrized_structure.hpp"

class ProductStructure : public AutomatonInterface {
	friend class ProductBuilder;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	// Storing a single transition to neighbour state together with its transition function
	struct Transition {
		StateID target_ID; // ID of the state the transition leads to
		std::size_t step_size; // How many bits of a parameter space bitset is needed to get from one targe value to another
		std::vector<bool> transitive_values; // Which values from the original set does not allow a trasition and therefore removes bits from the mask.

		Transition(const std::size_t _target_ID, const std::size_t _step_size, const std::vector<bool>& _transitive_values)
			: target_ID(_target_ID), step_size(_step_size), transitive_values(_transitive_values) {}
	};
	
	// State of the product - same as the state of parametrized structure but together with BA state
	struct State {
		StateID ID; // unique ID of the state
		StateID KS_ID; // ID of original KS state this one is built from
		StateID BA_ID; // ID of original BA state this one is built from
		bool initial; // True if the state is initial
		bool final; // True if the state is final
		Levels species_level; // species_level[i] = activation level of specie i
		std::vector<Transition> transitions; // Indexes of the neigbourging BasicStates - all those whose levels change only in one step of a single value

		State(const StateID _ID, const StateID _KS_ID, const StateID _BA_ID, const bool _initial, const bool _final, const  Levels & _species_level)
			: ID(_ID), KS_ID(_KS_ID), BA_ID(_BA_ID), initial(_initial), final(_final), species_level(_species_level) { }
	};
	
	// References to data structures
	const FunctionsStructure & functions; // Implicit reprezentation of functions - used as reference
	const ParametrizedStructure & structure; // Stores info about KS states
	const AutomatonStructure & automaton; // Stores info about BA states

	// DATA STORAGE
	std::vector<State> states;

	// Information about states
	std::vector<StateID> initial_states;
	std::vector<StateID> final_states;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FILLING FUNCTIONS (can be used only from ProductBuilder)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Add a new state, only with ID and levels
	 */
	inline void addState(const StateID KS_ID, const StateID BA_ID, const bool initial, const bool final, const Levels & species_level) {
		states.push_back(State(getProductID(KS_ID, BA_ID), KS_ID, BA_ID, initial, final, species_level));
	}

	/**
	 * @param ID	add data to the state with this IS
	 *
	 * Add a new transition with all its values
	 */
	inline void addTransition(const StateID ID, const StateID target_ID, const std::size_t step_size, const std::vector<bool> & transitive_values) {
		states[ID].transitions.push_back(Transition(target_ID, step_size, transitive_values));
	}
	
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OTHER FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ProductStructure(const ProductStructure & other);            // Forbidden copy constructor.
	ProductStructure& operator=(const ProductStructure & other); // Forbidden assignment operator.

public:
	ProductStructure(const FunctionsStructure & _functions, const ParametrizedStructure & _structure, const AutomatonStructure & _automaton) 
		: functions(_functions), structure(_structure), automaton(_automaton) { }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// KRIPKE STRUCTURE FUNCTIONS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @override
	 */
	inline const std::size_t getStateCount() const {
		return states.size();
	}

	/**
	 * @override
	 */
	const std::size_t getTransitionCount(const StateID ID) const { 
		return states[ID].transitions.size(); 
	}

	/**
	 * @override
	 */
	const std::size_t getTargetID(const StateID ID, const std::size_t trans_number) const {
		return states[ID].transitions[trans_number].target_ID;
	}

	/**
	 * Create string in the form KSstateBAstate or KSstate based on if user requests BA as well
	 * @override
	 */
	const std::string getString(const StateID ID) const {
		// Get states numbers
		StateID KS_ID = getKSID(ID);
		StateID BA_ID = getBAID(ID);
		// Concat strings of subparts
		std::string state_string = std::move(structure.getString(KS_ID));
		if (user_options.BA())
			state_string += std::move(automaton.getString(BA_ID));

		return std::move(state_string);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BUCHI AUTOMATON FUNCTIONS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @override
	 */
	virtual inline const bool isFinal(const StateID ID) const {
		return states[ID].final;
	}

	/**
	 * @override
	 */
	virtual inline const bool isInitial(const StateID ID) const {
		return states[ID].initial;
	}

	/**
	 * @override
	 */
	virtual inline const std::vector<StateID> & getFinalStates() const {
		return final_states;
	}

	/**
	 * @override
	 */
	virtual inline const std::vector<StateID> & getInitialStates() const {
		return initial_states;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// REFORMING GETTERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @return index of this combination of states in the product
	 */
	inline const StateID getProductID(const StateID KS_ID, const StateID BA_ID) const {
		return (KS_ID * automaton.getStateCount() + BA_ID);
	}

	/**
	 * @return index of BA state form the product
	 */
	inline const StateID getBAID(const StateID ID) const {
		return states[ID].BA_ID;
	}

	/**
	 * @return index of BA state form the product
	 */
	inline const StateID getKSID(const StateID ID) const {
		return states[ID].KS_ID;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// REFERENCE GETTERS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @return constant reference to Kripke structure stored within the product
	 */
	const ParametrizedStructure & getKS() const {
		return structure;
	}

	/**
	 * @return constant reference to Buchi automaton stored within the product
	 */
	const AutomatonStructure & getBA() const {
		return automaton;
	}

	/**
	 * @return constant reference to structure with regulatory functions
	 */
	const FunctionsStructure & getFunc() const {
		return functions;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA GETTERS 
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
	 * @param transition_num index of the transition to get the data from
	 *
	 * @return	number of neighbour parameters that share the same value of the function
	 */
	inline const std::size_t getStepSize(const StateID ID, const std::size_t transtion_num) const {
		return states[ID].transitions[transtion_num].step_size;
	}

	/**
	 * @param ID	ID of the state to get the data from
	 * @param transition_num index of the transition to get the data from
	 *
	 * @return	target values that are includete in non-transitive parameters that have to be removed
	 */
	inline const std::vector<bool> & getTransitive(const StateID ID, const std::size_t transtion_num) const {
		return states[ID].transitions[transtion_num].transitive_values;
	}
};

#endif