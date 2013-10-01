/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_OUTPUT_MANAGER_INCLUDED
#define PARSYBONE_OUTPUT_MANAGER_INCLUDED

#include "synthesis_manager.hpp"
#include "split_manager.hpp"
#include "witness_searcher.hpp"
#include "robustness_compute.hpp"
#include "database_filler.hpp"
#include "../model/model_translators.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Class that outputs formatted resulting data.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class OutputManager {
   const PropertyAutomaton & property; ///< Property automaton.
   const Model & model; ///< Reference to the model itself.

   DatabaseFiller database; ///< Fills data to the database.
public:
   NO_COPY(OutputManager)

   OutputManager(const PropertyAutomaton & _property, const Model & _model)
      : property(_property), model(_model), database(model, user_options.database_file, user_options.use_database) {	}

public:
   /**
    * @brief eraseData
    */
   void eraseData() {
      if (user_options.use_textfile) {
         output_streamer.createStreamFile(results_str, user_options.datatext_file);
      }
      if (user_options.use_database) {
         database.finishOutpout();
         database.dropTables();
      }
      outputForm();
   }

   /**
    * @brief outputForm
    */
   void outputForm() {
      if (user_options.use_database)
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

      if (user_options.use_database)
         database.startOutput();
   }

   /**
    * Output summary after the computation.
    *
    * @param total_count	number of all feasible colors
    */
   void outputSummary(const ParamNo accepting, const ParamNo total) {
      if (user_options.use_database)
         database.finishOutpout();
      output_streamer.output(verbose_str, "Total number of parametrizations: " + toString(accepting) + "/" + toString(total) + ".");
   }

   /**
    * Outputs round number - if there are no data within, then erase the line each round.
    */
   void outputRoundNo(const ParamNo round_no, const ParamNo round_count) const {
      // output numbers
      OutputStreamer::Trait trait = OutputStreamer::no_newl | OutputStreamer::rewrite_ln;
      output_streamer.output(verbose_str, "Round: " + toString(round_no) + "/" + toString(round_count) + ".", trait);
   }

   /**
    * Output parametrizations from this round together with additional data, if requested.
    */
   void outputRound(const ParamNo param_no, const size_t & cost, const double robustness_val, const string & witness) {
      string param_vals = ModelTranslators::createParamString(model,param_no);
      string line = toString(param_no) + separator + param_vals  + separator;
      string update = param_vals;
      update.back() = ','; // must remove closing bracket, it will be added by database manager

      line += toString(cost) + separator;
      update += toString(cost) + ",";

      string robustness = robustness_val > 0. ? toString(robustness_val) : "\"\"";
      line += robustness + separator;
      update += robustness + ",";

      line += witness + separator;
      update += "\"" + witness + "\",";

      size_t traits = 0;
      if (user_options.output_console && user_options.be_verbose)
         output_streamer.clear_line(verbose_str);
      output_streamer.output(results_str, line, traits);
      if (user_options.use_database)
         database.addParametrization(update);
   }
};

#endif // PARSYBONE_OUTPUT_MANAGER_INCLUDED
