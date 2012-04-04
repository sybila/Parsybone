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

#ifndef PARSYBONE_PRODUCT_BUILDER_INCLUDED
#define PARSYBONE_PRODUCT_BUILDER_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ProductBuilder creates the Product for coloring, based on automaton and PKS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../auxiliary/data_types.hpp"
#include "../auxiliary/output_streamer.hpp"
#include "parametrized_structure.hpp"
#include "automaton_structure.hpp"


class ProductBuilder {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const UserOptions & user_options; // Values provided as parameters
	const ParametrizedStructure & structure; // Stores info about KS states
	const AutomatonStructure & automaton; // Stores info about BA states
	Product & product; // Product to build

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTING FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTING FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Creates the vector with all parameters set to zero
	 */ 
	void createEmptyProduct (const std::size_t states_count) {
		product.state_parameters.resize(states_count);
		// Fill and set all to zero
		product.resetProduct();
	}
	
	ProductBuilder(const ProductBuilder & other);            // Forbidden copy constructor.
	ProductBuilder& operator=(const ProductBuilder & other); // Forbidden assignment operator.

public:
	/**
	 * Constructor just attaches the references to data holders
	 */
	ProductBuilder(const UserOptions &_user_options, const ParametrizedStructure & _structure, const AutomatonStructure & _automaton, Product & _product) 
		: user_options(_user_options), structure(_structure), automaton(_automaton), product(_product) { }

	void buildProduct() {
		const std::size_t states_count = structure.getStatesCount() * automaton.getStatesCount();
		const std::size_t parameters_count = 1;
	}
};

#endif