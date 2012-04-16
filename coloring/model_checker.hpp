/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_MODEL_CHECKER_INCLUDED
#define PARSYBONE_MODEL_CHECKER_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ModelChecker class solves the parameter synthesis problem by iterative transfer parameters from initial states to final ones.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../reforging/product_structure.hpp"
#include "parameters_functions.hpp"

class ModelChecker {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	// Provided with constructor
	const ParametrizedStructure & structure; // Stores info about KS states
	const AutomatonStructure & automaton; // Stores info about BA states
	const ProductStructure & product; // Product on which the computation will be conducted
	ColorStorage & storage; // Auxiliary product storage

	// Used for computation
	std::set<StateID> updates; // Set of states that need to spread their updates
	Range synthesis_range; // First and one beyond last color to be computed in this round
	WitnessUse witness_use; // How wintesses will be held in this computation

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTING FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	/**
	 * Main function of coloring - creates intersection of passing and transition colors to create and update color.
	 *
	 * @param target_param	paramters that are passed through the transition
	 * @param step_size	how many parameters share the same value for given function
	 * @param transitive_values	mask of all values from which those that have false are non-transitive
	 */
	void passParameters(Parameters & target_param, const std::size_t step_size, const std::vector<bool> & transitive_values) const {
		// INITIALIZATION OF VALUES FOR POSITIONING
		// Number of the first parameter
		std::size_t param_num = synthesis_range.first;
		// First value might not bet 0 - get it from current parameter position
		std::size_t value_num = (param_num / step_size) % transitive_values.size();
		// As well current value step might not be the first one, it is also necessary to get it from current parameter position
		std::size_t substep = param_num % step_size;
		// Mask that will be created
		register Parameters temporary = 0;

		// COMPUTATION OF MASK
		// List through all the paramters
		while (true) {
			// List through ALL the target values
			for (; value_num < transitive_values.size(); value_num++) {
				// Get size of the step for current value 
				std::size_t bits_in_step = std::min<std::size_t>(step_size, synthesis_range.second - param_num);
				// Move the mask so new value data can be add
				temporary <<= bits_in_step;
				// If transitive, add ones for the width of the step
				if (transitive_values[value_num]) {
					Parameters add = ~0;
					add >>= (getParamsetSize() - bits_in_step);
					temporary |= add;
				}
				// If we went throught the whole size, end
				if ((param_num += bits_in_step) == synthesis_range.second) {
					// Create interection of source parameters and transition parameters
					target_param &= temporary;
					return;
				}
				// Reset steps for the value
				substep = 0;
			}
			// Reset the value
			value_num = 0;
		}
	}

	/**
	 * Update target of the transition with transitible parameters
	 *
	 * @param parameters	parameters that will be distributed
	 * @param KS_transition_num		index of transition for this KS state
	 * @param source_KS_state	this KS state - target_KS is obtained from the transition index
	 * @param BA_target	target state of the BA
	 */
	void updateTarget(Parameters parameters, const StateID ID, const std::size_t trans_num) {
		// From an update strip all the parameters that can not pass through the transition - color intersection on the transition
		passParameters(parameters, product.getStepSize(ID, trans_num), product.getTransitive(ID, trans_num));
		// If some parameters get passed
		if (!none(parameters)) {
			// Compute and update target state of product
			StateID target = product.getTargetID(ID, trans_num);
			// If there is something new, schedule the target for an update
			if (storage.update(parameters, target)) {
				updates.insert(target);
			}
		}
	}

	/**
	 * From all the updates pick the one from the state with most bits
	 *
	 * @return index of the state to start an update from
	 */
	const StateID getStrongestUpdate() const {
		// Reference value
		StateID ID = 0;
		Parameters current_par = 0;
		// Cycle throught the updates
		for (auto update_it = updates.begin(); update_it != updates.end(); update_it++) {
			// Compapre with current data - if better, replace
			if (storage.getParameters(*update_it) == (current_par | storage.getParameters(*update_it))) {
				ID = *update_it;
				current_par = storage.getParameters(ID);
			}
		}
		return ID;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTING FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	ModelChecker(const ModelChecker & other);            // Forbidden copy constructor.
	ModelChecker& operator=(const ModelChecker & other); // Forbidden assignment operator.

public:
	/**
	 * Constructor, passes the data
	 */
	ModelChecker(const ProductStructure & _product, ColorStorage & _storage) : structure(_product.getKS()), automaton(_product.getBA()), product(_product), storage(_storage) { }

	/**
	 * From the source distribute its parameters and newly colored neighbours shedule for update.
	 *
	 * @param souce_state	ID of the source state in the product
	 * @param parameters	parameters that will be distributed
	 */
	void transferUpdates(const StateID ID, const Parameters parameters) {
		// Push updates for each BA transition times each KS transition
		for (std::size_t transition_num = 0; transition_num < product.getTransitionCount(ID); transition_num++) {
			// Send an update to the given target
			updateTarget(parameters, ID, transition_num);
		}
	}

	/**
	 * Distribute updates and store the new ones.
	 */
	void doColoring() {
		// While there are updates, pass them to succesing vertices
		while (!updates.empty()) {
			// Within updates, find the one with most bits
			StateID ID = getStrongestUpdate();
			// Pass data from updated vertex to its succesors
			transferUpdates(ID, storage.getParameters(ID));
			// Erase completed update from the set
			updates.erase(ID);
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SETTERS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Assign provided set as current updates.
	 */
	void setUpdates(const std::set<StateID> & _updates = std::set<StateID>()) {
		updates = _updates;
	}

	/**
	 * Set first and last parameters for this round.
	 */
	void setRange(const Range & _range) {
		synthesis_range = _range;
	}

	/** 
	 * Set how wintesses will be used in this round
	 */
	void setWitnessUse(const WitnessUse _witness_use) {
		witness_use = _witness_use;
	}
};


#endif