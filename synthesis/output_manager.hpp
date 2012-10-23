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
#include "../construction/product_structure.hpp"
#include "color_storage.hpp"
#include "coloring_analyzer.hpp"
#include "witness_searcher.hpp"
#include "robustness_compute.hpp"
#include "split_manager.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Class that outputs formatted resulting data.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class OutputManager {
   const ColorStorage & storage; ///< Provides current costs.
   const ColoringAnalyzer & analyzer; ///< Provides parametrizations' numbers and exact values.
   const SplitManager & split_manager; ///< Provides round and split information.
   const WitnessSearcher & searcher; ///< Provides witnesses in the form of transitions.
   const RobustnessCompute & robustness; ///< Provides Robustness value.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREATION METHODS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	OutputManager(const OutputManager & other); ///< Forbidden copy constructor.
	OutputManager& operator=(const OutputManager & other); ///< Forbidden assignment operator.

public:
	/**
	 * Simple constructor that only passes the references.
	 */
   OutputManager(const ColorStorage & _storage, const ColoringAnalyzer & _analyzer, const SplitManager & _split_manager, WitnessSearcher & _searcher, RobustnessCompute & _robustness)
      : storage(_storage), analyzer(_analyzer), split_manager(_split_manager), searcher(_searcher), robustness(_robustness) { }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OUTPUT METHODS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/**
    * Output summary after the computation.
	 *
	 * @param total_count	number of all feasible colors
	 */
	void outputSummary(const std::size_t total_count) {
		output_streamer.output(stats_str, "Total number of colors: ", OutputStreamer::no_newl).output(total_count, OutputStreamer::no_newl)
                     .output("/", OutputStreamer::no_newl).output(split_manager.getProcColorsCount(), OutputStreamer::no_newl).output(".");
	}

	/**
    * Outputs round number - if there are no data within, then erase the line each round.
	 */ 
	void outputRoundNum() {
      // erase the last line
      output_streamer.output(verbose_str, "Round: ", OutputStreamer::no_newl | OutputStreamer::rewrite_ln);

      // output numbers
      output_streamer.output(split_manager.getRoundNum(), OutputStreamer::no_newl).output("/", OutputStreamer::no_newl)
                     .output(split_manager.getRoundCount(), OutputStreamer::no_newl).output(":", OutputStreamer::no_newl);

      // add a new line if the result is not streamed to a file and there is any
      if (!output_streamer.isResultInFile())
         output_streamer.output("");

		output_streamer.flush();
	}

   /**
    * Recreate vector of cost values into a vector of strings.
    */
   const std::vector<std::string> getCosts(const std::vector<std::size_t> cost_vals) const {
      std::vector<std::string> costs;
      forEach(cost_vals, [&](const std::size_t cost){
         if (cost != ~static_cast<std::size_t>(0))
            costs.push_back(toString(cost));
      });
      return costs;
   }

	/**
    * Display colors synthetized during current round.
	 */
	void outputRound() const {
		// Get referencese
      auto costs = std::move(getCosts(storage.getCost())); auto cost_it = costs.begin();
		auto params = std::move(analyzer.getOutput()); auto param_it = params.begin();
		auto witnesses = std::move(searcher.getOutput()); auto witness_it = witnesses.begin();
		auto robusts = robustness.getOutput(); auto robust_it = robusts.begin();
\
		// Control the actual size of vectors - they must be the same
		if ((params.size() != costs.size()) || (params.size() != witnesses.size()) || (params.size() != robusts.size())) {
			std::string sizes_err = "Sizes of resulting vectors are different. Parametrizationss: " + toString(params.size()) +
											", costs:" + toString(costs.size()) + ", witnesses:" + toString(witnesses.size()) +
											", robustnesses:" + toString(robusts.size());
			throw std::invalid_argument(sizes_err);
		}

		// Cycle through parametrizations, display requested data
		while (param_it != params.end()) {
         output_streamer.output(results_str, *(param_it++), OutputStreamer::no_newl);
			output_streamer.output(results_str, separator, OutputStreamer::no_newl);

         if (user_options.timeSeries())
            output_streamer.output(results_str, *(cost_it++), OutputStreamer::no_newl);
         output_streamer.output(results_str, separator, OutputStreamer::no_newl);

         if (user_options.robustness())
            output_streamer.output(results_str, *(robust_it++), OutputStreamer::no_newl);

			output_streamer.output(results_str, separator, OutputStreamer::no_newl);

         if (user_options.witnesses())
            output_streamer.output(results_str, *(witness_it++), OutputStreamer::no_newl);

			output_streamer.output(results_str, "");
		}
	}
};

#endif // PARSYBONE_OUTPUT_MANAGER_INCLUDED
