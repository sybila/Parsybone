/*
 * Copyright (C) 2012 - Adam Streck
 *
 * This file is part of PoSeIDoN (Parameter Synthetizer for Discrete Networks) verification tool
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

#ifndef POSEIDON_PRODUCT_STRUCTURE_INCLUDED
#define POSEIDON_PRODUCT_STRUCTURE_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ProductStructure stores product of BA and PKS
// States are indexed as (BA_state_count * KS_state_ID + BA_state_ID) - e.g. if 3-state BA state ((1,0)x(1)) would be at position 3*1 + 1 = 4
// In other words, first iterate through BA then through KS
// ProductStructure data can be set only form the ProductStructureBuilder object.
// Rest of the code can access the data only via constant getters.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../auxiliary/data_types.hpp"

#include <stdexcept>

class ProductStructure {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// pointer used to access all the parameters
	Parameters * states;
	Parameters * updates;

	// Information
	const std::size_t states_count;
	const std::size_t parameters_count;
	bool are_states_built, are_updates_built;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA HANDLING FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	/**
	 * Creates the array of parameters with no value
	 *
	 * @param use_state	true if states array is requested, false if updates array is requested
	 */ 
	void createEmptyArray(const bool use_states) {
		bool & is_array_built = use_states ? are_states_built : are_updates_built;
		if (is_array_built)
			throw std::runtime_error("Trying to build array of parameters that is already built.");
		try {
			// Create
			if (use_states)
				states = new Parameters [states_count];
			else 
				updates = new Parameters [states_count];
			Parameters * array_pointer = use_states ? states : updates;

			// Fill and set all to zero
			const std::size_t _parameters_count = parameters_count;
			std::for_each(array_pointer, array_pointer + states_count, [_parameters_count](Parameters & parameters) {
				parameters.resize(_parameters_count, 0);
			});
		}
		catch (std::exception & e) {
			std::cerr << "Error while trying to create parameter states" << e.what();
			throw std::runtime_error("new Parameters [states_count] failed");
		}
		is_array_built = true;
	}

	/**
	 * Deletes the array
	 *
	 * @param use_state	true if states array is requested, false if updates array is requested
	 */
	void deleteArray(const bool use_states) {
		Parameters * array_pointer = use_states ? states : updates;
		bool & is_array_built = use_states ? are_states_built : are_updates_built;
		if (is_array_built) {
			is_array_built = false;
			delete [] array_pointer;
			array_pointer = 0;
		}
		else 
			throw std::runtime_error("deleteArray(const Parameters * array_pointer, bool & is_array_built) forbidden - trying to delete not built memory");
	}

	/**
	 * Sets all to zero
	 *
	 * @param use_state	true if states array is requested, false if updates array is requested
	 */ 
	void resetArray (const bool use_states) {
		Parameters * array_pointer = use_states ? states : updates;
		std::for_each(array_pointer, array_pointer + states_count,[](Parameters & parameters) {
			parameters.reset();
		});
	}

	/**
	 * gets safely pointer to parameters
	 *
	 * @param use_state	true if states array is requested, false if updates array is requested
	 *
	 * @return requested pointer
	 */
	Parameters * safeGetPointer(const bool use_states) const {
		if (use_states && !are_states_built) 
			throw std::runtime_error("Trying to access array of states which is not created at the time.");
		else if (!are_updates_built)
			throw std::runtime_error("Trying to access array of updates which is not created at the time.");
		return use_states ? states : updates;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OTHER FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ProductStructure(const ProductStructure & other);            // Forbidden copy constructor.
	ProductStructure& operator=(const ProductStructure & other); // Forbidden assignment operator.

public:
	/**
	 * Constructor that builds requested namber of parameters for states and updates array.
	 */
	ProductStructure (const std::size_t _states_count, const std::size_t _parameters_count) // Passing data
		: states_count(std::move(_states_count)), parameters_count(std::move(_parameters_count)) {
		states = updates = 0;
		are_states_built = are_updates_built = false;
		createEmptyArray(true);
		createEmptyArray(false);
	}

	/**
	 * Safely deletes given arrays.
	 */
	~ProductStructure() {
		deleteArray(true);
		deleteArray(false);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	/**
	 * @return	number of states of the product structure
	 */
	inline const std::size_t getStatesCount() const {
		return states_count;
	}

	/**
	 * @return	size of parameter space
	 */
	inline const std::size_t getParametersCount() const {
		return parameters_count;
	}

	/**
	 * @param state_num	index of the state to ask
	 * @param use_state	true if states array is requested, false if updates array is requested
	 * 
	 * @return true if the state has no parameters assigned to it
	 */
	inline const bool isEmpty(const std::size_t state_num, const bool use_states) const {
		Parameters * array_pointer = safeGetPointer(use_states);
		return array_pointer[state_num].empty();
	}

	/**
	 * @param state_num	index of the state to fill
	 * @param parameters to add - if empty, add all, otherwise use bitwise or
	 * @param use_state	true if states array is requested, false if updates array is requested
	 * 
	 * @return true if there was an actuall update
	 */
	const bool updateParameters(const Parameters & parameters, const std::size_t state_num, const bool use_states) {
		Parameters * array_pointer = safeGetPointer(use_states);
		if (parameters.is_subset_of(array_pointer[state_num]))
			return false;
		array_pointer[state_num] |= parameters;
		return true;
	}

	/**
	 * @param state_num	index of the state to ask
	 * @param use_state	true if states array is requested, false if updates array is requested
	 * 
	 * @return parameters assigned to the state
	 */
	inline const Parameters & getParameters(const std::size_t state_num, const bool use_states) {
		Parameters * array_pointer = safeGetPointer(use_states);
		return array_pointer[state_num];
	}

	/**
	 * Outside call to reset all the parameters in the array to zero.
	 *
	 * @param use_state	true if states array is requested, false if updates array is requested
	 */
	inline void resetParameters(const bool use_states) {
		resetArray(use_states);
	}
};

#endif