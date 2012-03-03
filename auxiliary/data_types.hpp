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

#ifndef POSEIDON_DATA_TYPES_INCLUDED
#define POSEIDON_DATA_TYPES_INCLUDED

#include <boost/dynamic_bitset.hpp>

// Structure that stores parsed user options
struct UserOptions {
	bool show_counterexamples : 1;
	bool show_base_coloring : 1;
	bool verbose : 1;
	bool guarantee : 1;
	bool negative_check : 1;
	bool control_semantics : 1;
	unsigned char padding : 2;
};

// To store the way a specie value has changed
enum Direction {up, stay, down};

// vector of activation levels of species used for labelling of states of KS
typedef std::vector<std::size_t> Levels;

// mask of parameters - each bit represents single combination of target values for each function
typedef boost::dynamic_bitset<> Parameters;

// State number and its coloring
typedef std::pair<std::size_t, Parameters> Coloring;

// Which type of parameter we use in the product
enum ParameterType { state, update };

#endif
