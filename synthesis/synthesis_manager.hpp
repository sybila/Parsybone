/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_SYNTHESIS_MANAGER_INCLUDED
#define PARSYBONE_SYNTHESIS_MANAGER_INCLUDED

#include "PunyHeaders/time_manager.hpp"

#include "../model/model.hpp"
#include "../model/property_automaton.hpp"

#include "witness_searcher.hpp"
#include "output_manager.hpp"
#include "color_storage.hpp"
#include "model_checker.hpp"
#include "split_manager.hpp"
#include "robustness_compute.hpp"
#include "checker_setting.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief STEP 3 - Control class for the computation.
///
/// Manager of the synthesis procedure - takes the reference data constructed during previous steps and computes and executes the synthesis.
/// Synthesis is done in three steps:
///	-# preparation: empties data and starts a new round.
///   -# synthesis: computes the coloring, stored in the storage object and adds data to coloring analyzer if needed.
///   -# conclusion: stores additional data and outputs
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SynthesisManager {
   const ProductStructure & product; ///< Holder of all the reference data.
   const Model & model;
   const PropertyAutomaton & property;

   unique_ptr<DatabaseFiller> database; ///< Class to output to a SQLite database;
   unique_ptr<ModelChecker> model_checker; ///< Class for synthesis.
   unique_ptr<OutputManager> output; ///< Class for output.
   unique_ptr<SplitManager> split_manager; ///< Control of independent rounds.
   unique_ptr<ColorStorage> storage; ///< Class that holds.
   unique_ptr<WitnessSearcher> searcher; ///< Class to build wintesses.
   unique_ptr<RobustnessCompute> robustness; ///< Class to compute robustness.

   ParamNo total_colors;
   size_t global_BFS_bound;
   SynthesisResults results;

   /**
    * @return settings fit for this round
    */
   CheckerSettings createRoundSetting() {
      CheckerSettings settings;
      settings.bfs_bound = global_BFS_bound;
      settings.param_no = split_manager->getParamNo();
      return settings;
   }

public:
   /**
    * Constructor builds all the data objects that are used within.
    */
   SynthesisManager(const ProductStructure & _product, const Model & _model, const PropertyAutomaton & _property) : product(_product), model(_model), property(_property) {
      // Create classes that help with the synthesis
      storage.reset(new ColorStorage(product));
      split_manager.reset(new SplitManager(ModelTranslators::getSpaceSize(model)));
      model_checker.reset(new ModelChecker(product, *storage));
      searcher.reset(new WitnessSearcher(product, *storage));
      robustness.reset(new RobustnessCompute(product, *storage));
      database.reset(new DatabaseFiller(model));
      output.reset(new OutputManager(property, model, *database));

      total_colors = 0ul;
      global_BFS_bound = user_options.bound_size;
   }

   /**
    * @brief checkDepthBound see if there is not a new BFS depth bound
    */
   void checkDepthBound() {
      const size_t cur_cost = results.lower_bound;
      if (cur_cost < global_BFS_bound) {
         output_streamer.clear_line(verbose_str);
         if (cur_cost != results.lower_bound || global_BFS_bound != INF) {
            split_manager->setStartPositions();
            output->eraseData();
            output_streamer.output(verbose_str, "New lowest bound on Cost has been found. Restarting the computation. The current Cost is: " + toString(cur_cost));
            total_colors = 0;
         } else { // You may not have to restart if the bound was found this round and everyone has it.
            output_streamer.output(verbose_str, "New lowest bound on Cost has been found. The current Cost is: " + toString(cur_cost));
         }
         global_BFS_bound = cur_cost;
      }
   }

   /**
    * Main synthesis function that iterates through all the rounds of the synthesis.
    */
   void checkFinite() {
      output->outputForm();

      // Do the computation for all the rounds
      do {
         if (user_options.bound_size == INF && user_options.bounded_check) // If there is a requirement for computing with the minimal bound.
            checkDepthBound();
         output->outputRoundNo(split_manager->getRoundNo(), split_manager->getRoundCount());
         storage->reset();

         CheckerSettings settings = createRoundSetting();
         settings.bounded = user_options.bounded_check;
         settings.minimal = true;
         settings.mark_initals = true;
         results = model_checker->conductCheck(settings);

         if (user_options.analysis() && results.is_accepting) {
            searcher->findWitnesses(results, settings);
            robustness->compute(results, searcher->getTransitions(), settings);
         }

         if (results.is_accepting) {
            string robustness_val = user_options.compute_robustness ? toString(robustness->getRobustness()) : "";
            string witness = user_options.compute_wintess ? WitnessSearcher::getOutput(product, searcher->getTransitions()) : "";
            output->outputRound(split_manager->getParamNo(), results.lower_bound, robustness_val, witness);
         }

         total_colors += results.is_accepting;

      } while (split_manager->increaseRound());

      output->outputSummary(total_colors, split_manager->getProcColorsCount());
   }

   /**
    * @brief checkGeneral
    */
   void checkGeneral() {
      output->outputForm();

      // Do the computation for all the rounds
      do {
         if (user_options.bound_size == INF && user_options.bounded_check) // If there is a requirement for computing with the minimal bound.
            checkDepthBound();
         output->outputRoundNo(split_manager->getRoundNo(), split_manager->getRoundCount());

         CheckerSettings settings = createRoundSetting();
         settings.bounded = user_options.bounded_check;
         settings.minimal = false;
         results = model_checker->conductCheck(settings);

         map<StateID, size_t> finals = results.found_depth;
         for (const pair<StateID, size_t> & final : finals) {
            storage->reset();

            CheckerSettings settings = createRoundSetting();
            settings.bounded = false;
            settings.initial_states = {final.first};
            settings.final_states = {final.first};
            settings.bfs_bound = global_BFS_bound - final.second;

            // Start coloring procedure
            results = model_checker->conductCheck(settings);
         }

         if (results.is_accepting) {
            string robustness_val = user_options.compute_robustness ? toString(robustness->getRobustness()) : "";
            string witness = user_options.compute_wintess ? WitnessSearcher::getOutput(product, searcher->getTransitions()) : "";
            output->outputRound(split_manager->getParamNo(), results.lower_bound, robustness_val, witness);
         }
         total_colors += results.is_accepting;
      } while (split_manager->increaseRound());

      output->outputSummary(total_colors, split_manager->getProcColorsCount());
   }
};

#endif // PARSYBONE_SYNTHESIS_MANAGER_INCLUDED
