/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_OUTPUT_MANAGER_INCLUDED
#define PARSYBONE_OUTPUT_MANAGER_INCLUDED

#include "../auxiliary/user_options.hpp"
#include "../model/property_automaton.hpp"
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
   const PropertyAutomaton & property; ///< Property automaton.
   const Model & model; ///< Reference to the model itself.
   const ColorStorage & storage; ///< Provides current costs.
   const ColoringAnalyzer & analyzer; ///< Provides parametrizations' numbers and exact values.
   const SplitManager & split_manager; ///< Provides round and split information.
   const WitnessSearcher & searcher; ///< Provides witnesses in the form of transitions.
   const RobustnessCompute & robustness; ///< Provides Robustness value.

   DatabaseFiller & database; ///< Fills data to the database.

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // CREATION METHODS
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   OutputManager(const OutputManager & other) = delete;
   OutputManager& operator=(const OutputManager & other) = delete;

public:
	/**
	 * Simple constructor that only passes the references.
	 */
   OutputManager(const PropertyAutomaton & _property, const Model & _model, const ColorStorage & _storage, DatabaseFiller & _database, const ColoringAnalyzer & _analyzer,
					  const SplitManager & _split_manager, WitnessSearcher & _searcher, RobustnessCompute & _robustness)
      : property(_property), model(_model), storage(_storage), analyzer(_analyzer), split_manager(_split_manager), searcher(_searcher), robustness(_robustness), database(_database) {	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// OUTPUT METHODS
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
   void eraseData() const {
      if (user_options.toFile()) {
         output_streamer.createStreamFile(results_str, user_options.textFile());
      }
      if (user_options.toDatabase()) {
         database.finishOutpout();
         database.dropTables();
      }
      outputForm();
   }

   void outputForm() const {
      if (user_options.toDatabase())
         database.creteTables();
      string format_desc = "#:(";

      for(SpecieID ID:range(model.species.size())) {
         for(auto param:model.getParameters(ID)) {
            format_desc += model.getName(ID) + "{" + param.context + "},";
         }
      }
      format_desc.back() = ')';
      format_desc += ":Cost:Robustness:WitnessPath";
      output_streamer.output(results_str, format_desc);

      if (user_options.toDatabase())
         database.startOutput();
   }

   /**
    * Output summary after the computation.
    *
    * @param total_count	number of all feasible colors
    */
   void outputSummary(const size_t total_count) const {
      if (user_options.toDatabase())
         database.finishOutpout();
      OutputStreamer::Trait trait = (user_options.toConsole()) ? 0 : OutputStreamer::rewrite_ln;
      output_streamer.output(verbose_str, "Total number of parametrizations: " + toString(total_count) + "/" + toString(split_manager.getProcColorsCount()) + ".", trait);
   }

   /**
    * Outputs round number - if there are no data within, then erase the line each round.
    */
   void outputRoundNum() const {
      // output numbers
      OutputStreamer::Trait trait = (user_options.toConsole()) ? 0 : OutputStreamer::no_newl | OutputStreamer::rewrite_ln;
      output_streamer.output(verbose_str, "Round: " + toString(split_manager.getRoundNum()) + "/" + toString(split_manager.getRoundCount()) + ":", trait);
   }

   /**
    * Recreate vector of cost values into a vector of strings.
    */
   const vector<string> getCosts(const vector<size_t> cost_vals) const {
      vector<string> costs;
      for(const auto & cost:cost_vals){
         if (cost != ~static_cast<size_t>(0))
            costs.push_back(toString(cost));
      }
      return costs;
   }

   /**
       * Output parametrizations from this round together with additional data, if requested.
       */
   void outputRound(const SynthesisResults & results) const {
      // Get referencese
      auto numbers = move(analyzer.getNumbers()); auto num_it = numbers.begin();

      auto costs = move(getCosts(results.getCost())); auto cost_it = costs.begin();
      auto params = move(analyzer.getStrings()); auto param_it = params.begin();
      auto witnesses = move(searcher.getOutput()); auto witness_it = witnesses.begin();
      auto robusts = move(robustness.getOutput()); auto robust_it = robusts.begin();

      // Control the actual size of vectors - they must be the same, if the vectors are employed
      if (property.getPropType() == TimeSeries && (params.size() != costs.size())) {
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

         if (property.getPropType() == TimeSeries) {
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
         }

         output_streamer.output(results_str, line);
         if (user_options.toDatabase())
            database.addParametrization(update);

         num_it++; param_it++;
         cost_it+= property.getPropType() == TimeSeries;
         robust_it += user_options.robustness();
         witness_it += user_options.witnesses();
      }
   }
};

#endif // PARSYBONE_OUTPUT_MANAGER_INCLUDED
