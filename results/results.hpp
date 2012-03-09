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

#ifndef POSEIDON_RESULTS_INCLUDED
#define POSEIDON_RESULTS_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Results are used by ModelChecker to store the important data from synthesis.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <assert.h>

class ModelChecker;

class Results {
	friend class ModelChecker;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Storing a single final state - the parameters are paritioned accordingly to rounds of coloring
	struct ColoredState {
		std::size_t state_ID;
		std::size_t KS_num;
		std::size_t BA_num;
		std::vector<Parameters> parameters_parts;

		ColoredState(const std::size_t _state_ID, const std::size_t _KS_num, const std::size_t _BA_num, const std::size_t rounds_count, const std::size_t round_size)
		: state_ID(_state_ID), KS_num(_KS_num), BA_num(_BA_num) {
			for(std::size_t round_num = 0; round_num < rounds_count; round_num++) {
				parameters_parts.push_back(Parameters(round_size));
			}
		}
	};

	std::vector<ColoredState> states;

	// Auxiliary data
	std::size_t rounds_count;
	std::size_t round_size;
	std::size_t last_round_size;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FILLING FUNCTIONS (can be used only from BasicStructureBuilder)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Strore a final state - states are stored at the beginning and later filled
	 */
	void addState(const std::size_t state_ID, const std::size_t KS_num, const std::size_t BA_num) {
		states.push_back(ColoredState(state_ID, KS_num, BA_num, rounds_count, round_size));
	}

	/**
	 * Data that have to be set before the results are stored - if they are not, exception will probably be caused
	 */
	void setAuxiliary(const std::size_t _rounds_count, const std::size_t _round_size, const std::size_t _last_round_size) {
		rounds_count = _rounds_count;
		round_size = _round_size;
		last_round_size = _last_round_size;
	}
	 	/**
	 * Fill results from current round only 
	 */	
	void addResult(const std::size_t state_ID, const Parameters & parameters, const std::size_t round_num) {
		assert(round_num < states[state_ID].parameters_parts.size());
		states[state_ID].parameters_parts[round_num] |= parameters;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTING FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	Results(const Results & other);            // Forbidden copy constructor.
	Results& operator=(const Results & other); // Forbidden assignment operator.

public:
	Results() {} // Default empty constructor, needed to create an empty object that will be filled

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @return	total number of parameters
	 */
	const std::size_t countParameters() const {
		Parameters all;
		std::size_t parameter_count = 0;
		// Go through each partition 
		for (std::size_t round_num = 0; round_num < rounds_count; round_num++) {
			// copy requested partition from the first final state
			all = states[0].parameters_parts[round_num];
			std::for_each(states.begin(), states.end(), [&all, round_num](const ColoredState & state){
				// Add partition from each other final state
				all |= state.parameters_parts[round_num];
			});
			// sum number of parameters from this partition with that from previous partitions
			parameter_count += all.count();
		}
		return parameter_count;
	}

	/**
	 * @return	All feasible parameters from the paramter space
	 */
	/*const Parameters getAllParameters() const {
		Parameters all;
		if (coloring.empty())
			return all;
		all = coloring[0].second;
		std::for_each(coloring.begin(), coloring.end(), [&all](const Coloring & coloring){
			all |= coloring.second;
		});
		return all;
	}*/

	/**
	 * @return	number of colorings in the result
	 */
	inline const std::size_t getStatesCount() const {
		return states.size();
	}

	/**
	 * @param state_index	index in the vector of states
	 *
	 * @return	ID of state of the Kripke Structure this state is build from
	 */
	inline const std::size_t & getKSNum(const std::size_t state_index) const {
		return states[state_index].KS_num;
	}

	/**
	 * @param state_index	index in the vector of states
	 *
	 * @return	ID of state of the Buchi automaton this state is build from
	 */
	inline const std::size_t & getBANum(const std::size_t state_index) const {
		return states[state_index].BA_num;
	}

	/**
	 * Get parameters of the state - warning, this function is costy - has to copy all the bits
	 *
	 * @param state_index	index in the vector of states
	 *
	 * @return	coloring with given index
	 */
	const Parameters getStateParameters(const std::size_t state_index) const {
		// Iterate through rounds
		Parameters results(round_size * rounds_count);
		for (std::size_t round_num = 0; round_num < rounds_count; round_num++){
			// Store bits from this round
			for (std::size_t bit_num = 0; bit_num < round_size; bit_num++){
				results <<= 1;
				results[0] = states[state_index].parameters_parts[round_num][bit_num];
			}
		}
		for (std::size_t bit_num = 0; bit_num < last_round_size; bit_num++){
			results <<= 1;
			results[0] = states[state_index].parameters_parts[rounds_count-1][bit_num];
		}
		return results;
	}

};

#endif
