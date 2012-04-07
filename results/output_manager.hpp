/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_OUTPUT_MANAGER_INCLUDED
#define PARSYBONE_OUTPUT_MANAGER_INCLUDED

#include "../auxiliary/user_options.hpp"
#include "../coloring/split_manager.hpp"
#include "product_analyzer.hpp"

class OutputManager {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Provided with constructor
	const SplitManager & split_manager;
	ProductAnalyzer & analyzer;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	OutputManager(const OutputManager & other);            // Forbidden copy constructor.
	OutputManager& operator=(const OutputManager & other); // Forbidden assignment operator.

public:
	OutputManager(const SplitManager & _split_manager, ProductAnalyzer & _analyzer) 
		         : split_manager(_split_manager), analyzer(_analyzer) { } 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OUTPUT FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	/**
//	 * display given parameters in the form [fun1, fun2, ...]
//	 */
//	void outputColors() const {
//		if (!user_options.coloring()) 
//			return;
//		auto colors = std::move(analyzer.getColors());
//		for (auto color_it = colors.begin(); color_it != colors.end(); color_it++) {
//			output_streamer.output(data, *color_it);
//		}
//	}
//
//public:
//	/**
//	 * main output function - displays number of parameters and parameters themselves if needed
//	 *
//	 * @param colors	if true, coloring of individuall final states will be shown
//	 */
//	void output() const {
//		// Display amount of all colors
//		output_streamer.output(data, "Total number of parameters is ", OutputStreamer::no_newl)
//						.output(count(analyzer.mergeColors()));
//
//		// display the colors
//		outputColors();
//	}
};

#endif