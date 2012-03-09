/*
 * Copyright (C) 2012 - Adam Streck
 *
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 *
 * Poseidon is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Poseidon is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 *
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno.
 * See http://sybila.fi.muni.cz/ .
 */

// #define DEBUG_OUTPUT

#ifndef POSEIDON_MODEL_CHECKER_INCLUDED
#define POSEIDON_MODEL_CHECKER_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ModelChecker class does the parameter synthesis by at first reachability coloring and the cycle detection from all the vertices that got colored.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdexcept>
#include <queue>
#include <cmath>

#include "../reforging/parametrized_structure.hpp"
#include "../reforging/automaton_structure.hpp"
#include "product_structure.hpp"
#include "../results/results.hpp"

class ModelChecker {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	// Provided with constructor
	const UserOptions & user_options;
	const ParametrizedStructure & structure; // Stores info about KS states
	const AutomatonStructure & automaton; // Stores info about BA states
	const std::size_t & bites_per_round;

	// Used for computation
	std::unique_ptr<ProductStructure> product;

	// Filled with computed data	
	Results & results; 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA STORING FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	/**
	 * Announce what an arrangement of the results will be to the results object.
	 */
	void passResultArrangement() {
		std::size_t BA_state_index = 0, state_ID = 0;
		// List throught product states that are final
		for (std::size_t ba_state_num = 0; ba_state_num < automaton.getStatesCount(); ba_state_num++) {
			// Skip non-final states
			if (!automaton.isFinal(ba_state_num)) 
				continue;
			
			// For each final state of the product prepare arrays of results
			for (std::size_t ks_state_num = 0; ks_state_num < structure.getStatesCount(); ks_state_num++) {
				// Pass information that the state coloring will be stored later
				state_ID = BA_state_index * structure.getStatesCount() + ks_state_num;
				results.addState(state_ID, ks_state_num, ba_state_num);
			}
			BA_state_index++;
		}
	}

	/**
	 * Pick final states from basic coloring and store them with their parameters
	 *
	 * @return queue with all colorings of final states
	 */
	std::queue<Coloring> storeFinalStates() {
		std::queue<Coloring> final_states; // States colored in basic coloring
		std::size_t state_num = 0; // ID of the state in product
		// List throught product states that are final
		for (std::size_t ba_state_num = 0; ba_state_num < automaton.getStatesCount(); ba_state_num++) {
			// Skip non-final states
			if (!automaton.isFinal(ba_state_num)) 
				continue;
			// For each final state of the product store the coloring
			for (std::size_t ks_state_num = 0; ks_state_num < structure.getStatesCount(); ks_state_num++) {
				// Compute the product state position and store
				state_num = ks_state_num * automaton.getStatesCount() + ba_state_num;
				final_states.push(std::make_pair(state_num, product->getParameters(state_num)));
			}
		}
		return final_states;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTING FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	/**
	 * Remove non-transitive parameters.
	 *
	 * @param target_param	paramters that are passed through the transition
	 * @param step_size	how many parameters share the same value for given function
	 * @param transitive_values	mask of all values from which those that have false are non-transitive
	 * @param synthesis_range	position of first and befor the last parameter that will be used in coloring
	 */
	void passParameters(Parameters & target_param, const std::size_t step_size, const std::vector<bool> & transitive_values, const Range & synthesis_range) {
		// List through all the paramters
		std::size_t param_num = synthesis_range.first;
		while (param_num < synthesis_range.second) {
			// List through ALL the target values
			for (std::size_t value = (param_num / step_size) % transitive_values.size(); value < transitive_values.size(); value++) {
				// Remove nont-transitive
				if (transitive_values[value] != true) {
					for (std::size_t substep = param_num % step_size; substep < step_size && param_num < synthesis_range.second; substep++) {
						target_param[param_num - synthesis_range.first] = 0;
						param_num++;
					}
				}
				// Others skip
				else {
					param_num += step_size;
					if (param_num >= synthesis_range.second)
						return;
				}
			}
		}
	}


	/**
	 * Update target of the transition with transitible parameters
	 *
	 * @param updates	set that will hold ID of newly updated state
	 * @param parameters	parameters that will be distributed
	 * @param synthesis_range	position of first and befor the last parameter that will be used in coloring
	 * Positioning:
	 * @param KS_transition_num		index of transition for this KS state
	 * @param source_KS_state	this KS state - target_KS is obtained from the transition index
	 * @param target_BA_state	target state of the BA
	 */
	void pushUpdate(std::set<std::size_t> & updates, const Parameters & parameters, const std::size_t KS_transition_num, 
		            const std::size_t source_KS_state, const std::size_t target_BA_state, const Range & synthesis_range) {
		Parameters passed = parameters;
		passParameters(passed, structure.getStepSize(source_KS_state, KS_transition_num), structure.getTransitive(source_KS_state, KS_transition_num), synthesis_range);
		// If there is at least some update
		if (!passed.none()) {
			// Compute and update target state
			std::size_t target_state = structure.getTargetID(source_KS_state,KS_transition_num) * automaton.getStatesCount() + target_BA_state;
			if (product->updateParameters(passed, target_state))
				updates.insert(target_state);
		}
	}

	/**
	 * From the source distribute its parameters and newly colored neighbours shedule for update.
	 *
	 * @param souce_state	ID of the source state in the producte
	 * @param parameters	parameters that will be distributed
	 * @param updates	set containing IDs of states that are scheduled for update
	 * @param synthesis_range	position of first and befor the last parameter that will be used in coloring
	 */
	void transferUpdates(std::set<std::size_t> & updates, const std::size_t source_state, const Parameters & parameters, const Range & synthesis_range) {
		// From ID of product state compute IDs of KS and BA states
		std::size_t source_BA_state = source_state % automaton.getStatesCount();
		std::size_t source_KS_state = source_state / automaton.getStatesCount();

		// For each feasible transition of BA store its ID in the queue
		std::queue<std::size_t> transitible_ba;
		for (std::size_t transition_num = automaton.getBeginIndex(source_BA_state); transition_num < automaton.getBeginIndex(source_BA_state + 1); transition_num++) {
			if (automaton.isTransitionFeasible(transition_num, structure.getStateLevels(source_KS_state)))
				transitible_ba.push(automaton.getTarget(transition_num));
		}

		// Push updates for each BA transition times each KS transition
		while (!transitible_ba.empty()) {
			std::size_t target_BA_state = transitible_ba.front();
			// Combine BA transition with all KS transitions and update those
			for (std::size_t KS_transition_num = 0; KS_transition_num < structure.getTransitionsCount(source_KS_state); KS_transition_num++) {
				pushUpdate(updates, parameters, KS_transition_num, source_KS_state, target_BA_state, synthesis_range);
			}
			transitible_ba.pop();
		}
	}

	/**
	 * Distribute updates and store the new ones.
	 *
	 * @param updates	set containing IDs of states that are scheduled for update
	 * @param synthesis_range	position of first and befor the last parameter that will be used in coloring
	 */
	void doColoring(std::set<std::size_t> & updates, const Range & synthesis_range) {
		while (!updates.empty()) {
			ProductStructure * product_ptr = product.get();
			std::size_t state_num = *std::max_element(updates.begin(), updates.end(), [product_ptr](std::size_t state_i, std::size_t state_j){ 
				return product_ptr->getParameters(state_i).count() < product_ptr->getParameters(state_j).count();
			});
			transferUpdates(updates, state_num, product->getParameters(state_num), synthesis_range);
			updates.erase(state_num);
		}
	}

	/**
	 * For each final state that has at least one parameter assigned, start cycle detection.
	 *
	 * @param init_coloring	reference to the final state that starts the coloring search with its parameters
	 * @param synthesis_range	position of first and befor the last parameter that will be used in coloring
	 */
	void detectCycle(const Coloring & init_coloring, const Range & synthesis_range) {
		// Assure the product is empty
		product->reset();
		std::set<std::size_t> updates;
		// Send updates from the initial state
		transferUpdates(updates, init_coloring.first, init_coloring.second, synthesis_range);

		doColoring(updates, synthesis_range);
	}

	/**
	 * Do initial coloring of states - start from initial states and distribute all the transitible parameters.
	 *
	 * @param product	product to color
	 * @param synthesis_range	position of first and befor the last parameter that will be used in coloring
	 */
	void colorProduct(std::unique_ptr<ProductStructure> & product, const Range & synthesis_range) {
		std::set<std::size_t> updates;

		// Fill all the initial states with all parameters and schedule them to update
		Parameters all_parameters(product->getParametersCount());
		if (synthesis_range.second - synthesis_range.first == bites_per_round)
			all_parameters.set(); // Set all to one
		// In the last round there might be less parameters than bits, set the rest to zero - they will not be used in coloring
		else {
			for (std::size_t param_num = 0; param_num < synthesis_range.second - synthesis_range.first; param_num++) {
				all_parameters[param_num] = 1;
			}
		}
		// For each initial state, store all the parameters and schedule for the update
		for (std::size_t state_num = 0; state_num < product->getStatesCount(); state_num++) {
			if (state_num % automaton.getStatesCount() == 0) {
					updates.insert(state_num);
					product->updateParameters(all_parameters, state_num);
			}
		}
		doColoring(updates, synthesis_range);
	}

	/**
	 * Entry point of the parameter synthesis. 
	 * In the first round, all states are colored with parameters that are transitive from some initial state. At the end, all final states are stored together with their color.
	 * In the second round, for all final states the strucutre is reset and colores are distributed from the state. After coloring the resulting color of the state is stored.
	 *
	 * @param synthesis_range	position of first and befor the last parameter that will be used in coloring
	 * @param round_num	how many parameter subspaces has already been colored
	 */
	void syntetizeParameters(const Range & synthesis_range, const std::size_t round_num) {
		product->reset();
		// Basic coloring
		colorProduct(product, synthesis_range);
		// Store colored final vertices
		std::queue<Coloring> final_states = std::move(storeFinalStates());
		// Pass the number of final states (even though it may be already known)

		// Get the actuall results by cycle detection
		for (std::size_t state_index = 0; !final_states.empty(); state_index++) {
			// If we do not check a guarantee property, restart the coloring using coloring of the first final state
			if (!user_options.guarantee && !final_states.front().second.empty())
				detectCycle(final_states.front(), synthesis_range);

			// Store the result
			const std::size_t state_num = final_states.front().first;
			if (!product->getParameters(state_num).none())
				results.addResult(state_index, product->getParameters(state_num), round_num);

			// Remove the state
			final_states.pop();
		}
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
	ModelChecker(const UserOptions & _user_options, const ParametrizedStructure & _structure, const AutomatonStructure & _automaton, Results & _results, const std::size_t & _bites_per_round) 
	: user_options(_user_options), structure(_structure), automaton(_automaton), results(_results), bites_per_round(_bites_per_round) {
		// Compute and pass data for result arrangement
		results.setAuxiliary(static_cast<std::size_t>(std::ceil(static_cast<double>(structure.getParametersCount()) / bites_per_round)), bites_per_round, structure.getParametersCount() % bites_per_round);
		passResultArrangement();
	}

	/**
	 * Function that does all the coloring.
	 */
	void computeResults() {
		// Create a new structure
		product.reset(new ProductStructure(structure.getStatesCount() * automaton.getStatesCount() , bites_per_round));
		// Data for computation arrangement
		std::size_t start_position = 0;
		std::size_t round_num = 0;
		// Cycle while there are paremeters
		while (start_position + bites_per_round < structure.getParametersCount()) {
			// Do normal synthesis on arranged data space
			syntetizeParameters( Range(start_position, start_position + bites_per_round), round_num);
			start_position += bites_per_round;
			round_num++;
		}
		// Do synthesis on the rest of the parameters
		syntetizeParameters(Range(start_position , structure.getParametersCount()), round_num);
	}
};


#endif