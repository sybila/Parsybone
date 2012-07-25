/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_COMMON_FUNCTIONS_INCLUDED
#define PARSYBONE_COMMON_FUNCTIONS_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file this file contains functions that may be usable throughout the whole program.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "data_types.hpp"
#include "output_streamer.hpp"
#include "boost/lexical_cast.hpp"

// Platform dependent min or max
#ifdef __GNUC__
#define my_max std::max
#define my_min std::min
#else
#define my_max max
#define my_min min
#endif

/**
 * Apply function on all the data stored within the object. 
 *
 * @param obj	object storing the data, must have begin() and end() methods
 * @param fun	pointer to the function to apply
 *
 * @return provided function (usually unnamed function)
 */
template<class Object, class Function>
Function forEach(Object & obj, Function fun)
{
	return std::for_each(obj.begin(), obj.end(), fun);
}

/**
 * Conversion of basic types to std::string data type. If an error occurs, displays it and throws an exception.
 *
 * @param data variable to convert
 *
 * @return converted string
 */
template<class basic_T>
	std::string toString(basic_T data) {
	std::string result;
	try {
		 result = std::move(boost::lexical_cast<std::string, basic_T>(data));
	} catch (boost::bad_lexical_cast e) {
		output_streamer.output(error_str, "Error occured while trying to cast a varible", OutputStreamer::no_newl)
				.output(data, OutputStreamer::no_newl).output(" to a string: ", OutputStreamer::no_newl).output(e.what());
		throw std::runtime_error("boost::lexical_cast<std::string, basic_T>(data)) failed");
	}
	return result;
}

#endif
