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

#ifndef POSEIDON_DATA_TYPES_INCLUDED
#define POSEIDON_DATA_TYPES_INCLUDED

#include <boost/dynamic_bitset.hpp>

// Structure that stores parsed user options
struct UserOptions {
	bool show_counterexamples : 1;
	bool show_base_coloring : 1;
	bool verbose : 1;
	bool negative_check : 1;
	bool control_semantics : 1;
	unsigned char padding : 3;
};

// If a regulation is missing, what kind of value it should get?
enum UnspecifiedRegulations {error, basal, param};

// To store the way a specie value has changed
enum Direction {up, stay, down};

// vector of activation levels of species used for labelling of states of KS
typedef std::vector<std::size_t> Levels;

// mask of parameters - each bit represents single combination of target values for each function
typedef boost::dynamic_bitset<> Parameters;

// State number and its coloring
typedef std::pair<std::size_t, Parameters> Coloring;

// Storing range of values that will be used in the form [first, last) i.e. first index is used, last is not
typedef std::pair<std::size_t, std::size_t> Range;

#endif
