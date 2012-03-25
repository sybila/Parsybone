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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Here are definitions of functions necessary for work with parameters.
// These functions work only for parameters that are basic data types (namely integer).
//
// REMEMBER!
// Parameters in an Paramset are ordered in an ascending order.
// Last bit in an Paramset is its sizeof(Paramset)*8-th parameters.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PARSYBONE_PARAMETERS_FUNCTIONS_INCLUDED
#define PARSYBONE_PARAMETERS_FUNCTIONS_INCLUDED

#include "../auxiliary/data_types.hpp"

#include <climits>

/**
 * @return number of parameters in a single round
 */
const std::size_t getParamsetSize() {
	return sizeof(Parameters) * 8;
}

/**
 * Count number of true bits (active parameters in the parameter set). There is a copy on the input that gets destroyed in the process.
 *
 * @param parameters	parameters to count
 *
 * @return	number of active parameters in the set
 */
const std::size_t count(Parameters parameters) {
	std::size_t result = 0;
	for (std::size_t paramu_pos = 0; paramu_pos < getParamsetSize(); paramu_pos++) {
		if (parameters % 2) 
			result++;
		parameters >>= 1;
	}
	return result;
}

/**
 * @return a parameter set with everything set to 1
 */
inline Parameters getAll() {
	return ~0;
}

/**
 * @return true if none of the paremters is set
 */
inline const bool none(Parameters parameters) {
	return parameters == 0;
}

/**
 * Flips every bit
 *
 * @return copy of input with swapped bits.
 */
inline Parameters flip(const Parameters parameters) {
	return ~parameters;
}

/**
 * Swaps parameters within a variable - last become first etc.
 *
 * @return copy of input with descending order of parameters
 */
inline Parameters swap(Parameters parameters) {
	Parameters new_params = 0;
	for (std::size_t param_num = 0; param_num < getParamsetSize(); param_num++) {
		new_params <<= 1;
		if (parameters % 2)
			new_params |= 1;
		parameters >>= 1;
	}
	return new_params;
}



#endif