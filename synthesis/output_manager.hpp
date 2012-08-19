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
#include "split_manager.hpp"
#include "../construction/product_structure.hpp"
#include "coloring_analyzer.hpp"
#include "witness_searcher.hpp"
#include "robustness_compute.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Class that outputs formatted data from results
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class OutputManager {
	const ColoringAnalyzer & analyzer; ///< Provides parametrizations' numbers and exact values
	const SplitManager & split_manager; ///< Provides round and split information
	const WitnessSearcher & searcher; ///< Provides witnesses in the form of transitions
	const RobustnessCompute & robustness; ///< Provides Robustness value

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	OutputManager(const OutputManager & other); ///< Forbidden copy constructor.
	OutputManager& operator=(const OutputManager & other); ///< Forbidden assignment operator.

public:
	/**
	 * Simple constructor that only passes the references.
	 */
	OutputManager(const ColoringAnalyzer & _analyzer, const SplitManager & _split_manager, WitnessSearcher & _searcher, RobustnessCompute & _robustness)
		: analyzer(_analyzer), split_manager(_split_manager), searcher(_searcher), robustness(_robustness) { }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OUTPUT FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/**
	 * Output summary after the computation
	 *
	 * @param total_count	number of all feasible colors
	 */
	void outputSummary(const std::size_t total_count) {
		output_streamer.output(stats_str, "Total number of colors: ", OutputStreamer::no_newl).output(total_count, OutputStreamer::no_newl)
			.output("/", OutputStreamer::no_newl).output(split_manager.getProcColorsCount());
	}

	/**
	 * Ouputs round number - if there are no data within, then erase the line each round
	 */ 
	void outputRoundNum() {
		// Erase the line if outputting results to file or not at all
        if (output_streamer.isResultInFile() || !(user_options.coloring() || user_options.witnesses()))
			output_streamer.output(verbose_str, "Round: ", OutputStreamer::no_newl | OutputStreamer::rewrite_ln);
		else 
			output_streamer.output(verbose_str, "Round: ", OutputStreamer::no_newl);

		// Output numbers
		output_streamer.output(split_manager.getRoundNum() + 1, OutputStreamer::no_newl).output("/", OutputStreamer::no_newl)
			           .output(split_manager.getRoundCount(), OutputStreamer::no_newl);

		// Add white space if outputting to file or not at all, otherwise add a new line
        if (output_streamer.isResultInFile() || !(user_options.coloring() || user_options.witnesses()))
			output_streamer.output("         ", OutputStreamer::no_newl);
		else
			output_streamer.output("");

		output_streamer.flush();
	}

	/**
	 * Display colors synthetized during current round
	 */
	void outputRound() const {
		// Get referencese
		auto params = std::move(analyzer.getOutput()); auto param_it = params.begin();
		auto witnesses = std::move(searcher.getOutput()); auto witness_it = witnesses.begin();
		auto robusts = robustness.getOutput(); auto robust_it = robusts.begin();

		// Cycle through parametrizations, display requested data
		while (param_it != params.end()) {
			output_streamer.output(results_str, *param_it, OutputStreamer::no_newl);
			output_streamer.output(results_str, separator, OutputStreamer::no_newl);
			param_it++;

			if (user_options.robustness()) {
				output_streamer.output(results_str, *robust_it, OutputStreamer::no_newl);
				robust_it++;
			}

			output_streamer.output(results_str, separator, OutputStreamer::no_newl);
			if (user_options.witnesses()) {
				output_streamer.output(results_str, *witness_it, OutputStreamer::no_newl);
				witness_it++;
			}
			output_streamer.output(results_str, "");
		}
	}
};

#endif
