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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Main class of the computation - responsible for the CMC procedure.
///
/// ModelChecker class solves the parameter synthesis problem by iterative transfer of feasible parametrizations from initial states to final ones.
/// Functions in model checker use many supporting variables and therefore are quite long, it would not make sense to split them, though.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ModelChecker {
   // Information
   PropType prop_type;
   const ProductStructure & product; ///< Product on which the computation will be conducted.
   Range synthesis_range; ///< First and one beyond last color to be computed in this round.
   StateID starting_state; ///< State from which the synthesis goes during the general LTL search.

   // Coloring storage
   ColorStorage & storage; ///< Class that actually stores colors during the computation.
   ColorStorage next_round_storage; ///< Class that stores updated colors for next round (prevents multiple transitions through one BFS round).
   set<StateID> updates; ///< Set of states that need to spread their updates.
   set<StateID> next_updates; ///< Updates that are sheduled forn the next round.

   // BFS boundaries
   Paramset starting; ///< Mask of parameters provided for this round.
   Paramset to_find; ///< Mask of parameters that are still not found.
   Paramset restrict_mask; ///< Mask of parameters that are secure to left out.
   vector<size_t> BFS_reach; ///< In which round this color was found.
   size_t BFS_level; ///< Number of current BFS level during coloring, starts from 0, meaning 0 transitions.
   size_t BFS_BOUND; ///< Boundary on depth, if exists.

   /**
    * From all the updates pick the one from the state with most bits.
    * @return	index of the state to start an update from
    */
   StateID getStrongestUpdate() const {
      // Reference value
      register StateID ID = 0;
      register Paramset current = 0;
      // Cycle throught the updates
      for (auto update_it = updates.begin(); update_it != updates.end(); update_it++) {
         Paramset test = storage.getColor(*update_it);
         // Compare with current data - if better, replace
         if (test != current) {
            if (test == (current | test)) {
               ID = *update_it;
               current = test;
            }
         }
      }
      return ID;
   }

   /**
    * For each found color add this BFS level as a first when the state was updated, if it was not already found.
    * @param colors	current coloring
    */
   void markLevels(const Paramset colors) {
      // If all is found, end
      if (!to_find || !(to_find & colors))
         return;

      // Which are new
      Paramset store = to_find & colors;
      // Remove currently found
      to_find &= ~store;

      // Store those that were found in this round
      for (int color_pos = static_cast<int>(ParamsetHelper::getSetSize() - 1); color_pos >= 0 ; store >>= 1, color_pos--) {
         if (store % 2)
            BFS_reach[color_pos] = BFS_level;
      }
   }

   /**
    * From the source distribute its parameters and newly colored neighbours shedule for update.
    * @param ID	ID of the source state in the product
    * @param parameters	parameters that will be distributed
    */
   void transferUpdates(const StateID ID, const Paramset parameters) {
      // Get passed colors, unique for each sucessor
      vector<Coloring> update = ColoringFunc::broadcastParameters(synthesis_range, product, ID, parameters);

      // For all passed values make update on target
      for (auto update_it = update.begin(); update_it != update.end(); update_it++) {
         // Skip empty updates
         if (ParamsetHelper::none(update_it->second))
            continue;

         // If something new is added to the target, schedule it for an update
         if (storage.soft_update(update_it->first, update_it->second)) {
            // Determine what is necessary to update
            next_round_storage.update(update_it->first, update_it->second);
            next_updates.insert(update_it->first);
         }
      }
   }

   /**
    * Main coloring function - passes parametrizations from newly colored states to their neighbours.
    * Executed as an BFS - in rounds.
    */
   SynthesisResults doColoring() {
      // While there are updates, pass them to succesing vertices
      do  {
         // Within updates, find the one with most bits
         StateID ID = getStrongestUpdate();
         // Check if this is not the last round
         if (prop_type == TimeSeries) {
            if (product.isFinal(ID))
               markLevels(storage.getColor(ID));
            else
               transferUpdates(ID, storage.getColor(ID) & restrict_mask);
         }
         else
            transferUpdates(ID, storage.getColor(ID));

         // Erase completed update from the set
         updates.erase(ID);

         // If there this round is finished, but there are still paths to find
         if (updates.empty() && to_find && (BFS_level < BFS_BOUND)) {
            updates = move(next_updates);
            storage.addFrom(next_round_storage);
            restrict_mask = to_find;
            BFS_level++; // Increase level
         }
      } while (!updates.empty());

      // After the coloring, pass cost to the coloring (and computed colors = starting - not found)
      SynthesisResults results;
      if (prop_type == TimeSeries)
         results.setResults(BFS_reach, ~to_find & starting);
      else if (starting_state != ~static_cast<size_t>(0))
         results.setResults(BFS_reach, storage.getColor(starting_state));
      return results;
   }

   /**
    * Sets/resets all coloring reference values.
    * @param parameters starting parameters for the cycle detection
    * @param _range	range of parameters for this coloring round
    * @param _updates	states that are will be scheduled for an update in this round
    */
   void prepareCheck(const Paramset parameters, const Range & _range, const size_t _BFS_BOUND, set<StateID> start_updates = set<StateID>()) {
      starting = to_find = restrict_mask = parameters; // Store which parameters are we searching for
      updates = move(start_updates); // Copy starting updates
      synthesis_range = _range; // Copy range of this round

      BFS_level = 0; // Set sterting number of BFS
      BFS_BOUND = _BFS_BOUND;
      next_updates.clear(); // Ensure emptiness of the next round
      BFS_reach.clear();
      BFS_reach.resize(ParamsetHelper::getSetSize(), INF); // Begin with infinite reach (symbolized by INF)
      next_round_storage.reset(); // Copy starting values
   }

public:
   /**
    * Constructor, passes the data and sets up auxiliary storage.
    */
   ModelChecker(const PropType _prop_type, const ProductStructure & _product, ColorStorage & _storage) : prop_type(_prop_type), product(_product), storage(_storage) {
      next_round_storage = storage; // Create an identical copy of the storage.
   }

   /**
    * Start a new coloring round for cycle detection from a single state.
    * @param ID	ID of the state to start cycle detection from
    * @param parameters	starting parameters for the cycle detection
    * @param _range	range of parameters for this coloring round
    */
   SynthesisResults startColoring(const StateID ID, const Paramset parameters, const Range & _range, const size_t _BFS_BOUND = INF) {
      prepareCheck(parameters, _range, _BFS_BOUND);
      transferUpdates(ID, parameters); // Transfer updates from the start of the detection
      starting_state = ID;
      return doColoring();
   }

   /**
    * Start a new coloring round for coloring of the nodes from the set of inital ones.
    * @param parameters	starting parameters to color the structure with
    * @param _updates	states that are will be scheduled for an update in this round
    * @param _range	range of parameters for this coloring round
    */
   SynthesisResults startColoring(const Paramset parameters, const set<StateID> & _updates, const Range & _range, const size_t _BFS_BOUND = INF){
      prepareCheck(parameters, _range, _BFS_BOUND, _updates);
      starting_state = INF;
      return doColoring();
   }
};

#endif // PARSYBONE_MODEL_CHECKER_INCLUDED
