/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_MODEL_CHECKER_INCLUDED
#define PARSYBONE_MODEL_CHECKER_INCLUDED

#include "PunyHeaders/common_functions.hpp"
#include "color_storage.hpp"
#include "coloring_func.hpp"
#include "synthesis_results.hpp"
#include "checker_setting.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Main class of the computation - responsible for the CMC procedure.
///
/// ModelChecker class solves the parameter synthesis problem by iterative transfer of feasible parametrizations from initial states to final ones.
/// Functions in model checker use many supporting variables and therefore are quite long, it would not make sense to split them, though.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ModelChecker {
   // Information
   const ProductStructure & product; ///< Product on which the computation will be conducted.
   CheckerSettings settings; ///< Setup for the process.

   // Coloring storage
   ColorStorage & storage; ///< Class that actually stores colors during the computation.
   ColorStorage next_round_storage; ///< Class that stores updated colors for next round (prevents multiple transitions through one BFS round).
   set<StateID> updates; ///< Set of states that need to spread their updates.
   set<StateID> next_updates; ///< Updates that are sheduled forn the next round.

   // BFS boundaries
   size_t BFS_level; ///< Number of current BFS level during coloring, starts from 0, meaning 0 transitions.
   bool accepted; ///< True if the property was accepted.

   /**
    * From the source distribute its parameters and newly colored neighbours shedule for update.
    * @param ID	ID of the source state in the product
    * @param parameters	parameters that will be distributed
    */
   void transferUpdates(const StateID ID) {
      // Get passed colors, unique for each sucessor

      const vector<StateID> transports = ColoringFunc::broadcastParameters(settings.getTestedNum(), product, ID);
      const vector<StateID> & targets = transports.empty() ? product.getLoops(ID) : transports;

      // For all passed values make update on target
      for (const StateID trans : targets) {
         // If something new is added to the target, schedule it for an update
         if (storage.soft_update(trans)) {
            // Determine what is necessary to update
            next_round_storage.update(trans);
            next_updates.insert(trans);
         }
      }
   }

   /**
    * Main coloring function - passes parametrizations from newly colored states to their neighbours.
    * Executed as an BFS - in rounds.
    */
   void doColoring() {
      StateID ID = *updates.begin();

      // Check if this is not the last round
      if (settings.isFinal(ID, product))
         accepted = true;

      transferUpdates(ID);
      updates.erase(ID);

      // If there this round is finished, but there are still paths to find
      if (updates.empty() && (BFS_level < settings.getBound())) {
         updates = move(next_updates);
         storage.addFrom(next_round_storage);
         if (settings.isMinimal() && accepted)
            return;
         BFS_level++; // Increase level
      }
   }

   /**
    * @brief prepareObjects   create empty space in the employed objects
    */
   void prepareObjects() {
      next_updates.clear(); // Ensure emptiness of the next round
      next_round_storage.reset(); // Copy starting values
      BFS_level = 0;
      accepted = false;
   }

   /**
    * @brief initiateCheck initiate data for the check based on the settings
    */
   void initiateCheck() {
      if (settings.getCoreState() != INF) {
         transferUpdates(settings.getCoreState());
      } else {
         updates = settings.hashInitials(product);
         for (const StateID init_ID : updates)
            storage.update(init_ID);
      }
   }

public:
   ModelChecker(const ProductStructure & _product, ColorStorage & _storage) : product(_product), storage(_storage) {
      next_round_storage = storage; // Create an identical copy of the storage.
   }

   /**
    * Start a new coloring round for cycle detection from a single state.
    */
   SynthesisResults conductCheck(const CheckerSettings & _settings) {
      settings = _settings;
      prepareObjects();
      initiateCheck();

      // While there are updates, pass them to succesing vertices
      do  {
         doColoring();
      } while (!updates.empty());

      // After the coloring, pass cost to the coloring (and computed colors = starting - not found)
      SynthesisResults results;
      results.setResults(BFS_level, accepted);

      return results;
   }
};

#endif // PARSYBONE_MODEL_CHECKER_INCLUDED
