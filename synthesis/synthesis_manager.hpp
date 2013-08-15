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

#include "../auxiliary/paramset_helper.hpp"
#include "../model/model.hpp"
#include "../model/property_automaton.hpp"

#include "coloring_analyzer.hpp"
#include "witness_searcher.hpp"
#include "output_manager.hpp"
#include "color_storage.hpp"
#include "model_checker.hpp"
#include "split_manager.hpp"
#include "SQLAdapter.hpp"
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

   ParamNum total_colors;
   size_t global_BFS_bound;
   SynthesisResults results;

   CheckerSettings createRoundSetting() {
      CheckerSettings settings(product);
      settings.bfs_bound = global_BFS_bound;
      settings.range = split_manager->getRoundRange();
      return settings;
   }

   /**
    * Do initial coloring of states - start from initial states and distribute all the transitional parameters.
    */
   void colorProduct(const bool bounded, const bool minimal) {
      storage->reset();

      CheckerSettings settings = createRoundSetting();
      settings.tested_params = split_manager->createStartingParameters();
      settings.bounded = bounded;
      settings.minimal = minimal;

      // Start coloring procedure
      results = model_checker->conductCheck(settings);
   }

   /**
    * @brief colorAccepting
    * @param accepting
    */
   void colorAccepting(const Coloring & accepting) {
      storage->reset();

      CheckerSettings settings = createRoundSetting();
      settings.bounded = false;
      settings.final_state = accepting.first;
      settings.tested_params = accepting.second;

      // Start coloring procedure
      results = model_checker->conductCheck(settings);
   }

   /**
    * @brief detectCycle
    * @param accepting
    */
   void detectCycle(const Coloring & accepting) {
      storage->reset();

      CheckerSettings settings = createRoundSetting();
      settings.bounded = false;
      settings.starting_state = accepting.first;
      settings.final_state = accepting.first;
      settings.tested_params = accepting.second;

      // Start coloring procedure
      results = model_checker->conductCheck(settings);
   }

public:
	unique_ptr<ColoringAnalyzer> analyzer; ///< Class for analysis.
	unique_ptr<DatabaseFiller> database; ///< Class to output to a SQLite database;
	unique_ptr<ModelChecker> model_checker; ///< Class for synthesis.
	unique_ptr<OutputManager> output; ///< Class for output.
	unique_ptr<SplitManager> split_manager; ///< Control of independent rounds.
	unique_ptr<ColorStorage> storage; ///< Class that holds.
	unique_ptr<WitnessSearcher> searcher; ///< Class to build wintesses.
	unique_ptr<RobustnessCompute> robustness; ///< Class to compute robustness.

   /**
    * Constructor builds all the data objects that are used within.
    */
   SynthesisManager(const ProductStructure & _product, const Model & _model, const PropertyAutomaton & _property) : product(_product), model(_model), property(_property) {
      // Create classes that help with the synthesis
      analyzer.reset(new ColoringAnalyzer(model));
      storage.reset(new ColorStorage(product));
      split_manager.reset(new SplitManager(ModelTranslators::getSpaceSize(model)));
      model_checker.reset(new ModelChecker(product, *storage));
      searcher.reset(new WitnessSearcher(product, *storage));
      robustness.reset(new RobustnessCompute(product, *storage));
      database.reset(new DatabaseFiller(model));
      output.reset(new OutputManager(property, model, *storage, *database, *analyzer, *split_manager, *searcher, *robustness));

      total_colors = 0ul;
      global_BFS_bound = user_options.bound_size;
      results.setResults(vector<size_t>(ParamsetHelper::getSetSize(), INF), ParamsetHelper::getNone());
   }

   /**
    * @brief checkDepthBound see if there is not a new BFS depth bound
    */
   void checkDepthBound() {
      const size_t min_depth = results.getMinDepth();
      if (min_depth < global_BFS_bound) {
         output_streamer.clear_line(verbose_str);
         if (min_depth != results.getMaxDepth() || global_BFS_bound != INF) {
            split_manager->setStartPositions();
            output->eraseData();
            output_streamer.output(verbose_str, "New lowest bound on Cost has been found. Restarting the computation. The current Cost is: " + toString(min_depth));
            total_colors = 0;
         } else { // You may not have to restart if the bound was found this round and everyone has it.
            output_streamer.output(verbose_str, "New lowest bound on Cost has been found. The current Cost is: " + toString(min_depth));
         }
         global_BFS_bound = min_depth;
      }
   }

	/**
	 * Setup everything that needs it for computation in this round.
	 */
	void doPreparation() {
		// Output round number
		output->outputRoundNum();
		// Pass information about round (necessary for setup)
		analyzer->strartNewRound(split_manager->getRoundRange());
	}

   /**
    * @brief doAnalysis Compute additional analyses.
    */
   void doAnalysis() {
      // Compute witnesses etc. if there is anything to computed, if so, print
      if (analyzer->getMask()) {
         searcher->findWitnesses(split_manager->getRoundRange(), results);
         robustness->compute(split_manager->getRoundRange(), results, searcher->getTransitions());
      }
   }

   /**
    * Store results that have not been stored yet and finalize the round where needed.
    */
   void doOutput() {
      if (analyzer->getMask())
         // Output what has been synthetized (colors, witnesses)
         output->outputRound(results);
   }

   /**
    * Main synthesis function that iterates through all the rounds of the synthesis.
    */
   void checkFinite() {
      // time_manager.startClock("coloring");
      output->outputForm();

      // Do the computation for all the rounds
      do {
         if (user_options.bound_size == INF && user_options.bounded_check) // If there is a requirement for computing with the minimal bound.
            checkDepthBound();
         doPreparation();
         colorProduct(user_options.bounded_check, true);
         for (const Coloring & final : storage->getColorings(product.getFinalStates()))
            analyzer->storeResults(final);
         if (user_options.analysis())
            doAnalysis();
         doOutput();
         total_colors += ParamsetHelper::count(analyzer->getMask());
      } while (split_manager->increaseRound());

      output->outputSummary(total_colors);
      // time_manager.writeClock("coloring");
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
         doPreparation();
         colorProduct(user_options.bounded_check, false);
         vector<Coloring> finals = storage->getColorings(product.getFinalStates());
         for (const Coloring & final : finals) {
            if (ParamsetHelper::hasNone(final.second))
               continue;
            colorAccepting(final);
            detectCycle(Coloring(final.first, storage->getColor(final.first)));
            analyzer->storeResults(Coloring(final.first, storage->getColor(final.first)));
         }

         doOutput();
         total_colors += ParamsetHelper::count(analyzer->getMask());
      } while (split_manager->increaseRound());

      output->outputSummary(total_colors);
   }
};

#endif // PARSYBONE_SYNTHESIS_MANAGER_INCLUDED
