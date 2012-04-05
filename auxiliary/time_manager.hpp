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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class that allows to start a use different clock for measurement
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "data_types.hpp"
#include "../auxiliary/output_streamer.hpp"

#ifdef __GNUC__
#include <sys/time.h>
#else
#include <windows.h>
#endif

class TimeManager {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MEASURE FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	typedef std::pair<std::string, long long> Clock;
	std::map<std::string, long long> clocks;

	/**
	 * Clock - independently on the architecture always (unless you have Mac... O.o) count time in ms
	 *
	 * @return	time in miliseconds
	 */
	#ifdef __GNUC__
	long long getMilliseconds() const {
		timeval tv;
		gettimeofday(&tv, 0);
		return tv.tv_sec*1000 + tv.tv_usec/1000;
	}
	#else
	long long getMilliseconds() const {
		return GetTickCount();
	}
	#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// RUN FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/**
	 * Starts a clock with given name and, if it is requsted by user, outputs the info.
	 *
	 * @param clock_name	unique ID of the clock that will also be send on the output
	 */
	void startClock(const std::string clock_name) {
		clocks.insert(std::make_pair(clock_name, getMilliseconds()));
		output_streamer.output(verbose, "Started clock ", OutputStreamer::no_newl | OutputStreamer::time_val).output(clock_name);
	}

	/**
	 * Outputs current runtime of the clock
	 *
	 * @param clock_name	name of the clock to output (also appears on the output)
	 */
	void ouputClock(const std::string clock_name) const {
		// Find the clock and output time difference
		if (clocks.find(clock_name) != clocks.end()) {
			long long runtime = getMilliseconds() - clocks.find(clock_name)->second;
			output_streamer.output(verbose, "Clock ", OutputStreamer::no_newl | OutputStreamer::time_val).output(clock_name, OutputStreamer::no_newl).output(" counted: ", OutputStreamer::no_newl)
				           .output(runtime, OutputStreamer::no_newl).output("ms.");
		} else { // If you do not find them, fail
			output_streamer.output(fail, "Requested clock ", OutputStreamer::no_newl | OutputStreamer::time_val).output(clock_name, OutputStreamer::no_newl).output(" have not been started until now.");
		}
	}
} time_manager;

#endif