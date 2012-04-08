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
	ProductStructure & product; // Product on which the computation will be conducted

	// Used for computation
	std::set<std::size_t> updates; // Set of states that need to spread their updates
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
	void updateTarget(Parameters parameters, const std::size_t product_source, const std::size_t KS_trans, const std::size_t BA_target) {
		// Obtain parts of source
		std::size_t KS_source = product.getStateIndexes(product_source).first;
		std::size_t BA_source = product.getStateIndexes(product_source).second;
		// From an update strip all the parameters that can not pass through the transition - color intersection on the transition
		passParameters(parameters, structure.getStepSize(KS_source, KS_trans), structure.getTransitive(KS_source, KS_trans));
		// If some parameters get passed
		if (!none(parameters)) {
			// Compute and update target state of product
			std::size_t KS_target = structure.getTargetID(KS_source, KS_trans);
			std::size_t product_target = product.getProductIndex(KS_target, BA_target);
			// If there is something new, schedule the target for an update
			if (product.updateParameters(parameters, product_target)) {
				product.addPredecessor(product_target, product_source, parameters);
				updates.insert(product_target);
			}
		}
	}

	/**
	 * Given this source BA state, find out all the target BA that are reachable under this KS state
	 *
	 * @param source_state	source state in the product
	 *
	 * @return	vector of all the reachable BA states
	 */
	std::vector<std::size_t> getReachableBA(const std::size_t product_source) const {
		// From where 
		std::size_t KS_source = product.getStateIndexes(product_source).first;
		std::size_t BA_source = product.getStateIndexes(product_source).second;
		// Vector to store them
		std::vector<std::size_t> reachable;
		// Cycle through all the transitions
		for (std::size_t transition_num = automaton.getBeginIndex(BA_source); transition_num < automaton.getBeginIndex(BA_source + 1); transition_num++) {
			// Check the transitibility
			if (automaton.isTransitionFeasible(transition_num, structure.getStateLevels(KS_source)))
				reachable.push_back(automaton.getTarget(transition_num));
		}
		return reachable;
	}

	/**
	 * From all the updates pick the one from the state with most bits
	 *
	 * @return index of the state to start an update from
	 */
	const std::size_t getStrongestUpdate() const {
		// Reference value
		std::size_t state_num = 0;
		Parameters current_par = 0;
		// Cycle throught the updates
		for (auto update_it = updates.begin(); update_it != updates.end(); update_it++) {
			// Compapre with current data - if better, replace
			if (product.getParameters(*update_it) == (current_par | product.getParameters(*update_it))) {
				state_num = *update_it;
				current_par = product.getParameters(state_num);
			}
		}
		return state_num;
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
	ModelChecker(ProductStructure & _product) : structure(_product.getKS()), automaton(_product.getBA()), product(_product) { }

	/**
	 * From the source distribute its parameters and newly colored neighbours shedule for update.
	 *
	 * @param souce_state	ID of the source state in the product
	 * @param parameters	parameters that will be distributed
	 */
	void transferUpdates(const std::size_t product_source, const Parameters parameters) {
		// Get current KS state
		std::size_t KS_source = product.getStateIndexes(product_source).first;
		// For each feasible transition of BA store its ID in the queue
		std::vector<std::size_t> reach_BA = getReachableBA(product_source);

		// Push updates for each BA transition times each KS transition
		for (auto state_it = reach_BA.begin(); state_it != reach_BA.end(); state_it++) {	
			// Combine BA transition with all KS transitions and update those
			for (std::size_t KS_trans = 0; KS_trans < structure.getTransitionsCount(KS_source); KS_trans++) {
				// Send an update to the given target
				updateTarget(parameters, product_source, KS_trans, *state_it);
			}
		}
	}

	/**
	 * Distribute updates and store the new ones.
	 */
	void doColoring() {
		// While there are updates, pass them to succesing vertices
		while (!updates.empty()) {
			// Within updates, find the one with most bits
			std::size_t state_num = getStrongestUpdate();
			// Pass data from updated vertex to its succesors
			transferUpdates(state_num, product.getParameters(state_num));
			// Erase completed update from the set
			updates.erase(state_num);
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SETTERS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Assign provided set as current updates.
	 */
	void setUpdates(const std::set<std::size_t> & _updates = std::set<std::size_t>()) {
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