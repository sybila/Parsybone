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
// Functions that may be usable within all program.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../auxiliary/data_types.hpp"

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
 * @param obj	object storing the data, must have begin and end method
 * @param fun	function to apply
 */
template<class Object, class Function>
Function forEach(Object & obj, Function fun)
{
	return std::for_each(obj.begin(), obj.end(), fun);
}

#endif