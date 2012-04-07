/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_OUTPUT_MANAGER_INCLUDED
#define PARSYBONE_OUTPUT_MANAGER_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class that outputs formatted data from results
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../auxiliary/user_options.hpp"
#include "../coloring/split_manager.hpp"
#include "../reforging/product_structure.hpp"
#include "result_storage.hpp"

class OutputManager {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Provided with constructor
	const SplitManager & split_manager;
	const ResultStorage & results;
	const ProductStructure & product;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	OutputManager(const OutputManager & other);            // Forbidden copy constructor.
	OutputManager& operator=(const OutputManager & other); // Forbidden assignment operator.

public:
	OutputManager(const ProductStructure & _product, const SplitManager & _split_manager, const ResultStorage & _results) 
		         : product(_product), split_manager(_split_manager), results(_results) { } 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OUTPUT FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

public:
	void outputSum() {
		output_streamer.output(data, "#Total number of colors: ", OutputStreamer::no_newl).output(results.getTotalColors(), OutputStreamer::no_newl)
			.output("/", OutputStreamer::no_newl).output(split_manager.getProcessRange().second - split_manager.getProcessRange().first); 
	}

	/**
	 * Ouputs round number
	 */ 
	void outputRound() {
		// Erase the line if outputting to file or not at all
		if (output_streamer.isResultInFile() || (!user_options.coloring() && !user_options.witnesses())) 
			output_streamer.output(verbose, "Round: ", OutputStreamer::no_newl | OutputStreamer::rewrite_ln);
		else 
			output_streamer.output(verbose, "Round: ", OutputStreamer::no_newl);

		// Output data
		output_streamer.output(split_manager.getRoundNum() + 1, OutputStreamer::no_newl).output("/", OutputStreamer::no_newl)
			           .output(split_manager.getRoundCount(), OutputStreamer::no_newl);

		// Add white space if outputting to file or not at all, otherwise add a new line
		if (output_streamer.isResultInFile() || (!user_options.coloring() && !user_options.witnesses())) 
			output_streamer.output("         ", OutputStreamer::no_newl);
		else
			output_streamer.output("");
	}

	/**
	 * Display colors synthetized during current round
	 */
	void outputColors() const {
		if (!user_options.coloring())
			return;
		auto colors = results.getAllColors();
		// Display amount of all colors
		std::for_each(colors.begin(), colors.end(), [&] (std::string color) {
			output_streamer.output(data, color);
		});
	}
};

#endif