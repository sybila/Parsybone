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

#ifndef PARSYBONE_PRODUCT_INCLUDED
#define PARSYBONE_PRODUCT_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product stores product of BA and PKS
// States are indexed as (BA_state_count * KS_state_ID + BA_state_ID) - e.g. if 3-state BA state ((1,0)x(1)) would be at position 3*1 + 1 = 4
// In other words, first iterate through BA then through KS
// Product is pretty safe to use to create / delete dynamic memory.
// Product data can be set only form the ProductBuilder object.
// Rest of the code can access the data only via constant getters.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <set>
#include <vector>
#include <stdexcept>

#include "../auxiliary/data_types.hpp"
#include "../auxiliary/output_streamer.hpp"

class Product {
	friend class ProductBuilder;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	typedef std::vector<std::size_t> Predecessors;
	
	// pointer used to access all the parameters
	std::vector<Parameters> state_parameters;
	std::vector<Predecessors> state_predecesors;

	std::set<std::size_t> initial_vertices;
	std::set<std::size_t> final_vertices;

	// Information
	const std::size_t states_count;
	WitnessUse witness_use;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA HANDLING FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Sets all to zero
	 */ 
	void resetProduct() {
		std::for_each(state_parameters.begin(), state_parameters.end(),[](Parameters & parameters) {
			parameters = 0;
		});
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OTHER FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	Product(const Product & other);            // Forbidden copy constructor.
	Product& operator=(const Product & other); // Forbidden assignment operator.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/**
	 * @return	number of states of the product structure
	 */
	inline const std::size_t getStatesCount() const {
		return states_count;
	}

	/**
	 * @param state_num	index of the state to fill
	 * @param parameters to add - if empty, add all, otherwise use bitwise or
	 * 
	 * @return true if there was an actuall update
	 */
	inline bool updateParameters(const Parameters parameters, const std::size_t state_num) {
		if (state_parameters[state_num] == (parameters | state_parameters[state_num]))
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