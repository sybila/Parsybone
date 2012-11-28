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
#include "database_filler.hpp"
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

   DatabaseFiller & database; ///< Fills data to the database.

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // CREATION METHODS
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   OutputManager(const OutputManager & other); ///< Forbidden copy constructor.
   OutputManager& operator=(const OutputManager & other); ///< Forbidden assignment operator.

public:
	/**
	 * Simple constructor that only passes the references.
	 */
	OutputManager(const ColorStorage & _storage, DatabaseFiller & _database, const ColoringAnalyzer & _analyzer,
					  const SplitManager & _split_manager, WitnessSearcher & _searcher, RobustnessCompute & _robustness)
		: storage(_storage), analyzer(_analyzer), split_manager(_split_manager), searcher(_searcher), robustness(_robustness), database(_database) {
		database.connect();
		database.creteTables();
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// OUTPUT METHODS
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
   /**
    * Output summary after the computation.
    *
    * @param total_count	number of all feasible colors
    */
   void outputSummary(const size_t total_count) {
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
   const vector<string> getCosts(const vector<size_t> cost_vals) const {
      vector<string> costs;
      forEach(cost_vals, [&](const size_t cost){
              if (cost != ~static_cast<size_t>(0))
              costs.push_back(toString(cost));
      });
      return costs;
   }

	/**
	 * Output parametrizations from this round together with additional data, if requested.
	 */
	void outputRound() const {
		// Get referencese
		auto numbers = move(analyzer.getNumbers()); auto num_it = numbers.begin();
		auto costs = move(getCosts(storage.getCost())); auto cost_it = costs.begin();
		auto params = move(analyzer.getStrings()); auto param_it = params.begin();
		auto witnesses = move(searcher.getOutput()); auto witness_it = witnesses.begin();
		auto robusts = move(robustness.getOutput()); auto robust_it = robusts.begin();

		// Control the actual size of vectors - they must be the same, if the vectors are employed
		if (user_options.timeSeries() && (params.size() != costs.size())) {
			string sizes_err = "Sizes of resulting vectors are different. Parametrizations: " + toString(params.size()) + ", costs:" + toString(costs.size());
			throw invalid_argument(sizes_err);
		} else if (user_options.witnesses() && (params.size() != witnesses.size())) {
			string sizes_err = "Sizes of resulting vectors are different. Parametrizations: " + toString(params.size()) + ", witnesses:" + toString(witnesses.size());
			throw invalid_argument(sizes_err);
		} else if (user_options.robustness() && (params.size() != robusts.size())) {
			string sizes_err = "Sizes of resulting vectors are different. Parametrizations: " + toString(params.size()) + ", robustnesses:" + toString(robusts.size());
			throw invalid_argument(sizes_err);
		}

		// Cycle through parametrizations, display requested data
		while (param_it != params.end()) {
			string line = toString(*num_it) + separator + *param_it + separator;
			string update = *param_it; update.back() = ',';

			if (user_options.timeSeries()) {
				line += *cost_it;
				update += *cost_it + ",";
			} line += separator;

			if (user_options.robustness()) {
				line += *robust_it;
				update += *robust_it + ",";
			} line += separator;

			if (user_options.witnesses()) {
				line += *witness_it;
				update += "\"" + *witness_it + "\",";
			} update.back() = ')';

			output_streamer.output(results_str, line);
			database.addParametrization(update);

			num_it++; param_it++; cost_it+= user_options.timeSeries(); robust_it += user_options.robustness(); witness_it += user_options.witnesses();
		}
	}
};

#endif // PARSYBONE_OUTPUT_MANAGER_INCLUDED
