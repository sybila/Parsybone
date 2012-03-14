/*
 * Copyright (C) 2012 - Adam Streck
 *
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 *
 * ParSyBoNe is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 *
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno.
 * See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_PRODUCT_STRUCTURE_INCLUDED
#define PARSYBONE_PRODUCT_STRUCTURE_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ProductStructure stores product of BA and PKS
// States are indexed as (BA_state_count * KS_state_ID + BA_state_ID) - e.g. if 3-state BA state ((1,0)x(1)) would be at position 3*1 + 1 = 4
// In other words, first iterate through BA then through KS
// Product is pretty safe to use to create / delete dynamic memory.
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
	Parameters * state_parameters;

	// Information
	const std::size_t states_count;
	const std::size_t parameters_count;
	bool is_product_built;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA HANDLING FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// TODO CHANGE SIZE!
	/**
	 * Creates the array of parameters with no value
	 */ 
	void createEmptyProduct () {
		if (is_product_built)
			throw std::runtime_error("trying to build product even thought it has been already built");
		try {
			state_parameters = new Parameters [states_count];
			// Fill and set all to zero
			const std::size_t _parameters_count = parameters_count;
			std::for_each(state_parameters, state_parameters + states_count, [_parameters_count](Parameters & parameters) {
				parameters.reset();
			});
		}
		catch (std::exception & e) {
			std::cerr << "Error while trying to create product states: " << e.what() << "\n.";
			throw std::runtime_error("new ProductStructure::Parameters [states_count] failed");
		}
		is_product_built = true;
	}

	/**
	 * Deletes the array
	 */
	void deleteProduct() {
		if (is_product_built) {
			is_product_built = false;
			delete [] state_parameters;
			state_parameters = 0;
		}
		else 
			throw std::runtime_error("trying to deleteProduct(), but the product is not built");
	}

	/**
	 * Sets all to zero
	 */ 
	void resetProduct() {
		std::for_each(state_parameters, state_parameters + states_count,[](Parameters & parameters) {
			parameters.reset();
		});
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OTHER FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ProductStructure(const ProductStructure & other);            // Forbidden copy constructor.
	ProductStructure& operator=(const ProductStructure & other); // Forbidden assignment operator.

public:
	ProductStructure (const std::size_t _states_count, const std::size_t _parameters_count) // Passing data
		: states_count(std::move(_states_count)), parameters_count(std::move(_parameters_count)), state_parameters(0), is_product_built(false) {
		createEmptyProduct();
	}

	~ProductStructure() {
		deleteProduct();
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
	 * @param state_num	index of the state to fill
	 * @param parameters to add - if empty, add all, otherwise use bitwise or
	 * 
	 * @return true if there was an actuall update
	 */
	inline bool updateParameters(const Parameters & parameters, const std::size_t state_num) const {
		if (state_parameters[state_num].count() == (parameters | state_parameters[state_num]).count())
			return false;
		state_parameters[state_num] |= parameters;
		return true;
	}

	/**
	 * @param state_num	index of the state to ask
	 * 
	 * @return parameters assigned to the state
	 */
	inline const Parameters & getParameters(const std::size_t state_num) {
		return state_parameters[state_num];
	}

	/**
	 * Outside call to reset the product.
	 */
	inline void reset() {
		resetProduct();
	}
};

#endif