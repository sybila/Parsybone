/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_COMMON_METHODS_INCLUDED
#define PARSYBONE_COMMON_METHODS_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file This file contains functions that may be usable throughout the whole program, mainly templates.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "data_types.hpp"
#include "output_streamer.hpp"

const size_t INF = static_cast<size_t>(~0); ///< this value represents infinite value - used for showing that a variable is unset

// Platform dependent min or max
#ifdef __GNUC__
#define my_max max
#define my_min min
#else
#define my_max max
#define my_min min
#endif

// Header for demangle function
#ifdef __GNUC__
#include <cxxabi.h>
#endif

/**
 * Return string with name of the used type, based on the architecture.
 *
 * @param[in] type_info  a structure holding infromation about the type whose name is requested
 *
 * @return a string with the demangle name of the type
 */
string demangle(const type_info & type_info) {
#ifdef __GNUC__
   int status;
   return string(abi::__cxa_demangle(type_info.name(), 0, 0, &status));
#else
   return string(type_info.name());
#endif
}

/**
 * Python-like range function - returns a vector of values in the range [begin,end[.
 *
 * @param[in] begin  first number in the range
 * @param[in] end    first number not in the range
 * @return  vector of values from the range [begin,end[
 */
template<typename T>
vector<T> range(const T begin, const T end) {
    vector<T> my_arr(end - begin);
    for (int i = 0; i < end - begin; i++) {
        my_arr[i] = begin + i;
    }
    return my_arr;
}

/**
 * Python-like range function - returns a vector of values in the range [0,end[.
 *
 * @param[in] end    first number not in the range
 * @return  vector of values from the range [0,end[
 */
template<typename T>
vector<T> range(const T end) {
    return range(0, end);
}

/**
 * Apply function on all the data stored within the object.
 *
 * @param[in,out] obj	object storing the data, must have begin() and end() methods
 * @param[in] fun	pointer to the function to apply
 *
 * @return provided function (usually unnamed function)
 */
template<class Object, class Function>
Function forEach(Object & obj, Function fun)
{
   return for_each(obj.begin(), obj.end(), fun);
}

/**
 * Conversion of basic types to string data type. If an error occurs, displays it and throws an exception.
 *
 * @param[in] data variable to convert
 *
 * @return  converted string
 */
template<class basic_T>
string toString(basic_T data) {
	string result;
	try {
		result = move(lexical_cast<string, basic_T>(data));
	} catch (bad_lexical_cast e) {
		output_streamer.output(error_str, "Error occured while trying to cast a variable", OutputStreamer::no_newl)
				.output(data, OutputStreamer::no_newl).output(" to a string: ", OutputStreamer::no_newl).output(e.what());
		throw runtime_error("lexical_cast<string," + string(typeid(basic_T).name()) + ">(\"data\")) failed");
	}
	return result;
}

/**
 * Increases integral value by 1.
 *
 * @param[in,out] val  reference to value that will be increased
 */
template<class integral_T>
void increase(typename vector<integral_T>::reference val) {val++;}

/**
 * Sets boolean value to true.
 *
 * @param[in,out] val  reference to value that will be increased
 */
template<>
void increase<bool>(vector<bool>::reference val) {val = true;}

/**
 * Iterates values from left to right if it is possible. If so, return true, otherwise return false.
 *
 * @param[in] top vector of maximal values each component can reach
 * @param[in] bottom vector of minimal values each component can reach
 * @param[in,out] interated   vector of values to iterate
 *
 * @return  true if the iteration was valid, false if it caused overflow (iterated > bottom)
 */
template<class integral_T>
bool iterate(const vector<integral_T> & top, const vector<integral_T> & bottom, vector<integral_T> & interated) {
	for (size_t val_num = 0; val_num <= interated.size(); val_num++) {
		if (val_num == interated.size())
			return false;
		if (interated[val_num] == top[val_num]) {
			interated[val_num] = bottom[val_num];
		}
		else {
			increase<integral_T>(interated[val_num]);
			break;
		}
	}
	return true;
}

#endif // PARSYBONE_COMMON_METHODS_INCLUDED
