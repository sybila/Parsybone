/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_TIME_MANAGER_INCLUDED
#define PARSYBONE_TIME_MANAGER_INCLUDED

#include "../auxiliary/output_streamer.hpp"

// Get system clock reference
#ifdef __GNUC__
#include <sys/time.h>
#else
#include <windows.h>
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Class that allows to start a use different clock for time measurement.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class TimeManager {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MEASURE FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	typedef std::pair<std::string, long long> Clock; ///< Clock - contain a start time referenced by a name of the clock
	std::map<std::string, long long> clocks; ///< vector of clocks

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
		output_streamer.output(stats_str, "Started clock ", OutputStreamer::no_newl).output(clock_name);
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
			output_streamer.output(stats_str, "Clock ", OutputStreamer::no_newl).output(clock_name, OutputStreamer::no_newl).output(" counted: ", OutputStreamer::no_newl)
				           .output(runtime, OutputStreamer::no_newl).output("ms.");
		} else { // If you do not find them, fail
			output_streamer.output(stats_str, "Requested clock ", OutputStreamer::no_newl).output(clock_name, OutputStreamer::no_newl).output(" have not been started until now.");
		}
	}
} time_manager;

#endif // PARSYBONE_TIME_MANAGER_INCLUDED
