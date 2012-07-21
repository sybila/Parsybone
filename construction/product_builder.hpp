/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_PRODUCT_BUILDER_INCLUDED
#define PARSYBONE_PRODUCT_BUILDER_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ProductBuilder creates the Product for coloring, based on automaton and PKS
// States of product are indexed as (BA_state_count * KS_state_ID + BA_state_ID) - e.g. if 3-state BA state ((1,0)x(1)) would be at position 3*1 + 1 = 4
// In other words, first iterate through BA then through KS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "product_structure.hpp"
#include "color_storage.hpp"

class ProductBuilder {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const ParametrizedStructure & structure; // Stores info about KS states
	const AutomatonStructure & automaton; // Stores info about BA states
	ProductStructure & product; // Product to build
	ColorStorage & storage; // Auxiliary product storage to build

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTATION FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Given this source BA state, find out all the target BA that are reachable under this KS state
	 *
	 * @param KS_ID	source KS state
	 * @param BA_ID source BA state
	 *
	 * @return	vector of all the reachable BA states from give product state
	 */
	std::vector<StateID> getReachableBA(const StateID KS_ID, const StateID BA_ID) const {
		// Vector to store them
		std::vector<StateID> reachable;
		// Cycle through all the transitions
		for (std::size_t trans_num = 0; trans_num < automaton.getTransitionCount(BA_ID); trans_num++) {
			// Check the transitibility
			if (automaton.isTransitionFeasible(BA_ID, trans_num, structure.getStateLevels(KS_ID)))
				reachable.push_back(automaton.getTargetID(BA_ID, trans_num));
		}
		return reachable;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTING FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Creates the vector with all parameters set to zero
	 *
	 * @param states_count	how many states the product will have
	 */ 
	void createEmptyStorage (const std::size_t states_count) {
		// add empty states to storage
		for (StateID ID = 0; ID < states_count; ID++) {
			storage.addState(ID, states_count);
		}
	}

	/**
	 * Create position set of final states and initial states in the product
	 */
	void markStates() {
		for (std::size_t ba_state_num = 0; ba_state_num < automaton.getStateCount(); ba_state_num++) {
			// Insert the state if it is an initial state
			if (ba_state_num == 0) {
				for (std::size_t ks_state_num = 0; ks_state_num < structure.getStateCount(); ks_state_num++) {
					product.initial_states.push_back(product.getProductID(ks_state_num, ba_state_num));
				}
			}
			// Insert the state if it is a final state
			if (automaton.isFinal(ba_state_num)) {
				for (std::size_t ks_state_num = 0; ks_state_num < structure.getStateCount(); ks_state_num++)  {
					product.final_states.push_back(product.getProductID(ks_state_num, ba_state_num));
				}
			}
		}
	}

	/**
	 * Create state of the product from its KS and BA counterparts
	 *
	 * @param KS_ID	source in the KS
	 * @param BA_ID source in the BA
	 */
	void createProductStates(const StateID KS_ID, const StateID BA_ID) {
		// Add this state
		product.addState(KS_ID, BA_ID, automaton.isInitial(BA_ID), automaton.isFinal(BA_ID), structure.getStateLevels(KS_ID));
		const StateID ID = product.getProductID(KS_ID, BA_ID); 

		// Get all possible BA targets
		auto BA_targets = std::move(getReachableBA(KS_ID, BA_ID));

		// Add all the transitions possible
		for (std::size_t KS_trans = 0; KS_trans < structure.getTransitionCount(KS_ID); KS_trans++) {
			// Get transition data
			const StateID KS_target_ID = structure.getTargetID(KS_ID, KS_trans);
			const std::size_t step_size = structure.getStepSize(KS_ID, KS_trans);
			const std::vector<bool> transitive_values = structure.getTransitive(KS_ID, KS_trans);

			// Add transition for all allowed targets
			for (auto BA_traget_it = BA_targets.begin(); BA_traget_it != BA_targets.end(); BA_traget_it++) {
				// Compute target position
				const StateID target = product.getProductID(KS_target_ID, *BA_traget_it);
				// Store the transition
				product.addTransition(ID, target, step_size, transitive_values);
			}
		}
	}
	
	ProductBuilder(const ProductBuilder & other);            // Forbidden copy constructor.
	ProductBuilder& operator=(const ProductBuilder & other); // Forbidden assignment operator.

public:
	/**
	 * Constructor just attaches the references to data holders
	 */
	ProductBuilder(const ParametrizedStructure & _structure, const AutomatonStructure & _automaton, ProductStructure & _product, ColorStorage & _storage) 
		: structure(_structure), automaton(_automaton), product(_product), storage(_storage) { }

	/**
	 * Create the the product from BA and KS together.
	 */
	void buildProduct() {
		// count product states
		const std::size_t states_count = structure.getStateCount() * automaton.getStateCount();
		
		// Start storage
		createEmptyStorage(states_count);

		// Add transitions
		for (std::size_t KS_ID = 0; KS_ID < structure.getStateCount(); KS_ID++) {
			for (std::size_t BA_ID = 0; BA_ID < automaton.getStateCount(); BA_ID++) {
				createProductStates(KS_ID, BA_ID);
			}
		}

		// Create final and intial states vectors
		markStates();
	}
};

#endif