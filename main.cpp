/*! \mainpage Welcome to the Parsybone code documentation.
 *
 * This is a documentation of code belonging solely to the Parsybone tool, version 1.2.
 * This text is not supposed to be a user manual in any way, but as an reference for further development of this tool.
 * For a description of usage of the tool, please refer to the Manual that is shipped together with the tool.
 *
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

using namespace std;

#include "PunyHeaders/time_manager.hpp"

#include "auxiliary/output_streamer.hpp"
#include "auxiliary/user_options.hpp"
#include "parsing/argument_parser.hpp"
#include "parsing/data_parser.hpp"
#include "parsing/parsing_manager.hpp"
#include "parsing/explicit_filter.hpp"
#include "construction/construction_manager.hpp"
#include "construction/product_builder.hpp"
#include "synthesis/synthesis_manager.hpp"

/**
 * @brief checkDepthBound see if there is not a new BFS depth bound
 */
void checkDepthBound(const size_t depth, SplitManager & split_manager, OutputManager & output, size_t & BFS_bound, ParamNo & valid_param_count) {
   if (depth < BFS_bound && user_options.minimalize_cost) {
      // Reset the outputs if better was found.
      output_streamer.clear_line(verbose_str);
      split_manager.setStartPositions();
      output.eraseData();
      output_streamer.output(verbose_str, "New lowest bound on Cost has been found. Restarting the computation. The current Cost is: " + toString(depth));
      valid_param_count = 0;
      BFS_bound = depth;
   }
}

/**
 * Execution of succesive parts of the parameter synthesis.
 */
int main(int argc, char* argv[]) {
   time_manager.startClock("* Runtime", false);

   Model model;
   PropertyAutomaton property;
   ProductStructure product;
   ExplicitFilter filter;

   // Parsing
   try {
      ParsingManager::parseOptions(argc, argv);
      model = ParsingManager::parseModel(user_options.model_path + user_options.model_name + MODEL_SUFFIX);
      property = ParsingManager::parseProperty(user_options.property_path + user_options.property_name + PROPERTY_SUFFIX);
   }
   catch (std::exception & e) {
      output_streamer.output(error_str, string("Error occured while parsing input: \"").append(e.what()).append("\"."));
      return 1;
   }

   // Construction of data structures
   try {
      ConstructionManager::computeModelProps(model);
      for (const string & filter_name : user_options.filter_databases) {
         SQLAdapter adapter;
         adapter.setDatabase(filter_name);
         filter.addAllowed(model, adapter);
      }
      product = ConstructionManager::construct(model, property);
   }
   catch (std::exception & e) {
      output_streamer.output(error_str, string("Error occured while building the data structures: \"").append(e.what()).append("\"."));
      return 2;
   }

   // Synthesis of parametrizations
   try {
      SplitManager split_manager(ModelTranslators::getSpaceSize(model));
      OutputManager output(property, model);
      SynthesisManager synthesis_manager(product, model, property);
      ParamNo param_count = 0ul; ///< Number of parametrizations that were considered satisfiable.
      size_t BFS_bound = user_options.bound_size; ///< Maximal cost on the verified property.
      output.outputForm();

      // Do the computation for all the rounds
      do {
         output.outputRoundNo(split_manager.getRoundNo(), split_manager.getRoundCount());
         if (!filter.isAllowed(split_manager.getParamNo()))
            continue;

         string witness;
         double robustness_val = 0.;
         size_t cost = INF;

         // Call synthesis procedure based on the type of the property.
         switch (product.getMyType()) {
         case BA_finite:
            cost = synthesis_manager.checkFinite(witness, robustness_val, split_manager.getParamNo(), BFS_bound, user_options.compute_wintess, user_options.compute_robustness);
            break;
         case BA_standard:
            cost = synthesis_manager.checkFull(witness, robustness_val, split_manager.getParamNo(), BFS_bound, user_options.compute_wintess, user_options.compute_robustness);
            break;
         default:
            throw runtime_error("Unsupported Buchi automaton type.");
         }

         // Parametrization was considered satisfying.
         if (cost != INF) {
            checkDepthBound(cost, split_manager, output, BFS_bound, param_count);
            output.outputRound(split_manager.getParamNo(), cost, robustness_val, witness);
            param_count++;
         }
      } while (split_manager.increaseRound());

      output.outputSummary(param_count, split_manager.getProcColorsCount());
   }
   catch (std::exception & e) {
      output_streamer.output(error_str, string("Error occured while syntetizing the parametrizations: \"").append(e.what()).append("\"."));
      return 3;
   }

   if (user_options.be_verbose)
      time_manager.writeClock("* Runtime");
   return 0;
}
