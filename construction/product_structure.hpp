/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_PRODUCT_INCLUDED
#define PARSYBONE_PRODUCT_INCLUDED

#include "../auxiliary/output_streamer.hpp"
#include "../construction/automaton_structure.hpp"
#include "../construction/parametrized_structure.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// This is the final step of construction - a structure that is acutally used during the computation. For simplicity, it copies data from its predecessors (BA and PKS).
/// @attention States of product are indexed as (BA_state_count * KS_state_ID + BA_state_ID) - e.g. if 3-state BA state ((1,0)x(1)) would be at position 3*1 + 1 = 4.
///
/// ProductStructure data can be set only form the ProductBuilder object.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ProductStructure : public AutomatonInterface {
	friend class ProductBuilder;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	/// Storing a single transition to a neighbour state together with its transition function
	struct Transition : public TransitionProperty {
		std::size_t step_size; ///<  How many bits of a parameter space bitset is needed to get from one targe value to another
		std::vector<bool> transitive_values; ///< Which values from the original set does not allow a trasition and therefore removes bits from the mask.

		Transition(const StateID target_ID, const std::size_t _step_size, const std::vector<bool>& _transitive_values)
			: TransitionProperty(target_ID), step_size(_step_size), transitive_values(_transitive_values) {} ///< Simple filler, assigns values to all the variables
	};
	
	// State of the product - same as the state of parametrized structure but together with BA state
	struct State : public StateProperty<Transition> {
		StateID KS_ID; ///< ID of an original KS state this one is built from
		StateID BA_ID; ///< ID of an original BA state this one is built from
		bool initial; ///< True if the state is initial
		bool final; ///< True if the state is final
		Levels species_level; ///< species_level[i] = activation level of specie i in this state

		/// Simple filler, assigns values to all the variables
		State(const StateID ID, const StateID _KS_ID, const StateID _BA_ID, const bool _initial, const bool _final, const  Levels & _species_level)
			: StateProperty<Transition>(ID), KS_ID(_KS_ID), BA_ID(_BA_ID), initial(_initial), final(_final), species_level(_species_level) {}
	};
	
	// References to data predecessing data structures
	const ParametrizedStructure & structure; ///< Stores info about KS states, used in the getString function
	const AutomatonStructure & automaton; ///< Stores info about BA states, used in the getString function

	/// Stores all the data
	std::vector<State> states;

	// Information about states
	std::vector<StateID> initial_states; ///< Vector of inital states of the product
	std::vector<StateID> final_states; ///< Vector of final states of the product

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
// CONSTRUCTION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ProductStructure(const ProductStructure & other);            // Forbidden copy constructor.
	ProductStructure& operator=(const ProductStructure & other); // Forbidden assignment operator.

public:
	ProductStructure(const ParametrizedStructure & _structure, const AutomatonStructure & _automaton)
		: structure(_structure), automaton(_automaton) { }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// KRIPKE STRUCTURE FUNCTIONS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline const std::size_t getStateCount() const {
		return states.size();
	}

	const std::size_t getTransitionCount(const StateID ID) const { 
		return states[ID].transitions.size(); 
	}

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
	virtual inline const bool isFinal(const StateID ID) const {
		return states[ID].final;
	}

	virtual inline const bool isInitial(const StateID ID) const {
		return states[ID].initial;
	}

	virtual inline const std::vector<StateID> & getFinalStates() const {
		return final_states;
	}

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
