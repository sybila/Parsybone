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
#include "witness_storage.hpp"

class OutputManager {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Provided with constructor
	const ProductStructure & product;
	const SplitManager & split_manager;
	const ResultStorage & results;
	const WitnessStorage & witnesses;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	OutputManager(const OutputManager & other);            // Forbidden copy constructor.
	OutputManager& operator=(const OutputManager & other); // Forbidden assignment operator.

public:
	OutputManager(const ProductStructure & _product, const SplitManager & _split_manager, const ResultStorage & _results, WitnessStorage & _witnesses) 
		         : product(_product), split_manager(_split_manager), results(_results), witnesses(_witnesses) { } 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OUTPUT FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

public:
	void outputSummary() {
		output_streamer.output(stats_str, "Total number of colors: ", OutputStreamer::no_newl).output(results.getTotalColors(), OutputStreamer::no_newl)
			.output("/", OutputStreamer::no_newl).output(split_manager.getProcessRange().second - split_manager.getProcessRange().first); 
	}

	/**
	 * Ouputs round number - if there are no data within, then erase the line each round
	 */ 
	void outputRoundNum() {
		// Erase the line if outputting to file or not at all
		if (output_streamer.isResultInFile() || (!user_options.coloring() && !user_options.witnesses())) 
			output_streamer.output(verbose_str, "Round: ", OutputStreamer::no_newl | OutputStreamer::rewrite_ln);
		else 
			output_streamer.output(verbose_str, "Round: ", OutputStreamer::no_newl);

		// Output data
		output_streamer.output(split_manager.getRoundNum() + 1, OutputStreamer::no_newl).output("/", OutputStreamer::no_newl)
			           .output(split_manager.getRoundCount(), OutputStreamer::no_newl);

		// Add white space if outputting to file or not at all, otherwise add a new line
		if (output_streamer.isResultInFile() || (!user_options.coloring() && !user_options.witnesses())) 
			output_streamer.output("         ", OutputStreamer::no_newl);
		else
			output_streamer.output("");

		output_streamer.flush();
	}

	/**
	 * Outputs color 
	 */
	void outputColor(const std::string & color) const {
		// Output color
		output_streamer.output(results_str, color, OutputStreamer::no_newl).output(" | ", OutputStreamer::no_newl);
	}

	/**
	 * Outputs  witness
	 *
	 * @param paths	vector of witnessed for initil to final paths with their final vertex number
	 * @param cycles vector of witnessed for final to final paths with their final vertex number
	 */
	void outputWitness(const std::vector<std::pair<std::size_t, std::string>> & paths) const {
		// Cycle through paths
		for (auto path_it = paths.begin(); path_it != paths.end(); path_it++) 
			output_streamer.output(" | ", OutputStreamer::no_newl).output(path_it->second, OutputStreamer::no_newl);
	}

	/**
	 * Outputs  witness
	 *
	 * @param paths	vector of witnessed for initil to final paths with their final vertex number
	 * @param cycles vector of witnessed for final to final paths with their final vertex number
	 */
	void outputWitness(const std::vector<std::pair<std::size_t, std::string>> & paths, const std::vector<std::pair<std::size_t, std::string>> & cycles) const {
		// cycle paths
		for (auto path_it = paths.begin(); path_it != paths.end(); path_it++) 
			// cycle cycles
			for (auto cycle_it = cycles.begin(); cycle_it != cycles.end(); cycle_it++) 
				// If agree on state, combine
				if (path_it->first == cycle_it->first)
					output_streamer.output(" | ", OutputStreamer::no_newl).output(path_it->second, OutputStreamer::no_newl)
									.output("-", OutputStreamer::no_newl).output(cycle_it->second, OutputStreamer::no_newl);
	}

	/**
	 * Display colors synthetized during current round
	 */
	void outputData() const {
		// If there is nothing to output, skip
		if (!user_options.coloring() && !user_options.witnesses()) 
			return;
		// Storing objects
		std::vector<std::pair<std::size_t, std::string>> colors;
		std::vector<std::pair<std::size_t, std::vector<std::pair<std::size_t, std::string>>>> path_wits, cycle_wits;
		// Get colors if needed
		if (user_options.coloring())
			colors = std::move(results.getAllColors());
		// Get witnesses if needed
		if (user_options.witnesses()) {
			path_wits = std::move(witnesses.getAllWitnesses(true));
			cycle_wits = std::move(witnesses.getAllWitnesses(false));
		}

		// Display color and witness if requested
		for (std::size_t color_index = 0; color_index < colors.size(); color_index++) {
			if (user_options.coloring() && user_options.witnesses() && user_options.timeSerie()) {
				outputColor(colors[color_index].second);
				outputWitness(path_wits[color_index].second);
			}
			else if (user_options.coloring() && user_options.witnesses() && !user_options.timeSerie()) {
				outputColor(colors[color_index].second);
				outputWitness(path_wits[color_index].second, cycle_wits[color_index].second);
			}
			else if (user_options.coloring() && !user_options.witnesses()) {
				outputColor(colors[color_index].second);
			}
			else if (user_options.timeSerie()) {
				outputWitness(path_wits[color_index].second);
			}
			else {
				outputWitness(path_wits[color_index].second, cycle_wits[color_index].second);
			}
			output_streamer.output("");
		}
	}
};

#endif