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

   unique_ptr<ModelChecker> model_checker; ///< Class for synthesis.
   unique_ptr<ColorStorage> storage; ///< Class that holds.
   unique_ptr<WitnessSearcher> searcher; ///< Class to build wintesses.
   unique_ptr<RobustnessCompute> computer; ///< Class to compute robustness.

   /**
    * @brief analyseLasso Parametrization is know to be satisfiable, make analysis of it.
    */
   void analyseLasso(const pair<StateID, size_t> & final, vector<StateTransition> & trans, double & robust, const bool robustness) {
      SynthesisResults results;
      CheckerSettings settings;
      settings.final_states = {final.first};
      settings.minimal = settings.mark_initals = true;
      results = model_checker->conductCheck(settings);
      searcher->findWitnesses(results, settings);
      trans = searcher->getTransitions();
      if (robustness) {
         computer->compute(results, searcher->getTransitions(), settings);
         robust = computer->getRobustness();
      }

      settings.mark_initals = false;
      settings.initial_states = {final.first};
      results = model_checker->conductCheck(settings);
      searcher->findWitnesses(results, settings);
      const vector<StateTransition> & trans_ref = searcher->getTransitions();
      trans.insert(trans.begin(), trans_ref.begin(), trans_ref.end());
      if (robustness) {
         computer->compute(results, searcher->getTransitions(), settings);
         robust *= computer->getRobustness();
      }
   }

   /**
    * @brief computeLasso parametrization is know to reach a final state, test that state for a bounded loop.
    */
   size_t computeLasso(const pair<StateID, size_t> & final, vector<StateTransition> & trans,  double & robust, const size_t BFS_bound, const bool witnesses, const bool robustness) {
      CheckerSettings settings;
      settings.minimal = true;
      settings.initial_states = settings.final_states = {final.first};
      settings.bfs_bound = BFS_bound == INF ? BFS_bound : (BFS_bound - final.second);

      SynthesisResults results = model_checker->conductCheck(settings);
      const size_t cost = results.lower_bound == INF ? INF : results.lower_bound + final.second;
      if (results.is_accepting && (witnesses || robustness))
         analyseLasso(final, trans, robust, robustness);

      return cost;
   }

public:
   /**
    * Constructor builds all the data objects that are used within.
    */
   SynthesisManager(const ProductStructure & _product, const Model & _model, const PropertyAutomaton & _property) : product(_product), model(_model), property(_property) {
      storage.reset(new ColorStorage(product));
      model_checker.reset(new ModelChecker(product, *storage));
      searcher.reset(new WitnessSearcher(product, *storage));
      computer.reset(new RobustnessCompute(product, *storage));
   }

   /**
    * @brief checkFull conduct model check with only reachability
    * @param[in] witnesses for all the shortest cycles
    * @param[in] robustness_val  robustness of the whole computation
    * @param param_no number of parametrization to test
    * @param BFS_bound current bound on depth
    * @param witnesses should compute witnesses
    * @param robustness should compute robustness
    * @return the Cost value for this parametrization
    */
   size_t checkFull(string & witness_str, double & robust, const ParamNo param_no, const size_t BFS_bound, const bool witnesses, const bool robustness) {
      vector<StateTransition> trans;

      CheckerSettings settings;
      settings.bfs_bound = BFS_bound;
      settings.param_no = param_no;
      settings.mark_initals = true;
      SynthesisResults results = model_checker->conductCheck(settings);

      size_t cost = INF;
      map<StateID, size_t> finals = results.found_depth;
      for (const pair<StateID, size_t> & final : finals) {
         vector<StateTransition> trans_temp;
         double robust_temp;
         size_t new_cost = computeLasso(final, trans_temp, robust_temp, BFS_bound, witnesses, robustness);
         // Clear data if the new path is shorter than the others.
         if (new_cost < cost) {
            cost = new_cost;
            robust = 0.;
            trans.clear();
         }
         robust += robust_temp;
         trans.insert(trans.begin(), trans_temp.begin(), trans_temp.end());
      }

      sort(trans.begin(), trans.end());
      trans.erase(unique(trans.begin(), trans.end()), trans.end());
      witness_str = WitnessSearcher::getOutput(product, trans);

      return cost;
   }

   /**
    * @brief checkFull conduct model check with both trying to reach and with cycle detection.
    * @param[in] witness of the shortest path
    * @param[in] robustness_val robustness of the shortest paths
    * @param param_no number of parametrization to test
    * @param BFS_bound current bound on depth
    * @param witnesses should compute witnesses
    * @param robustness should compute robustness
    * @return  the Cost value for this parametrization
    */
   size_t checkFinite(string & witness_str, double & robustness_val, const ParamNo param_no, const size_t BFS_bound, const bool witnesses, const bool robustness) {
      CheckerSettings settings;
      settings.param_no = param_no;
      settings.bfs_bound = BFS_bound;
      settings.minimal = true;
      settings.mark_initals = true;
      SynthesisResults results = model_checker->conductCheck(settings);

      if ((witnesses || robustness) && results.is_accepting) {
         searcher->findWitnesses(results, settings);
         if (robustness)
            computer->compute(results, searcher->getTransitions(), settings);
         robustness_val = robustness ? computer->getRobustness() : 0.;
         witness_str = user_options.compute_wintess ? WitnessSearcher::getOutput(product, searcher->getTransitions()) : "";
      }

      return results.lower_bound;
   }
};

#endif // PARSYBONE_SYNTHESIS_MANAGER_INCLUDED
