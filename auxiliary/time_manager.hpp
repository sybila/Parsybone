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

#ifndef POSEIDON_TIME_MANAGER_INCLUDED
#define POSEIDON_TIME_MANAGER_INCLUDED

#include "../results/output_streamer.hpp"

// Clock - dependendent on the achitecture. 
#ifdef __GNUC__
#include <sys/time.h>
/**
	* @return	time in miliseconds
	*/
long long myClock() {
	timeval tv;
	gettimeofday(&tv, 0);
	return tv.tv_sec*1000 + tv.tv_usec/1000;
}
#else
#include <windows.h>
/**
	* @return	time in miliseconds
	*/
long long myClock() {
	return GetTickCount();
}
#endif

class TimeManager {

};

#endif