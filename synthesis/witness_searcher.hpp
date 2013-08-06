/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_WITNESS_SEARCHER_INCLUDED
#define PARSYBONE_WITNESS_SEARCHER_INCLUDED

#include "../construction/construction_holder.hpp"
#include "coloring_func.hpp"
#include "color_storage.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Class for search of transitions belonging to shortest time series paths.
///
/// Class executes a search through the synthetized space in order to find transitions included in shortest paths for every parametrization.
/// Procedure is supposed to be first executed and then it can provide results.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class WitnessSearcher {
   const ProductStructure & product; ///< Product reference for state properties.
   const ColorStorage & storage; ///< Constant storage with the actuall data.
   Range round_range; ///< Range of parametrizations used this round

   vector<set<pair<StateID, StateID> > > transitions; ///< Acutall storage of the transitions found - transitions are stored by parametrizations numbers in the form (source, traget).
   vector<string> string_paths; ///< This vector stores paths for every parametrization (even those that are not acceptable, having an empty string).

   vector<StateID> path; ///< Current path of the DFS with the final vertex on 0 position.
   vector<Paramset> depth_masks; ///< For each of levels of DFS, stores mask of parametrizations with corresponding cost (those that are not furter used in the DFS).
   size_t depth; ///< Current level of the DFS.
   size_t max_depth; ///< Maximal level of recursion that is possible (maximal Cost in this round).

   /// This structure stores "already tested" paramsets for a state.
   struct Marking {
      Paramset succeeded; ///< Mask of those parametrizations that have found a paths from this state.
      vector<Paramset> busted; ///< Mask of the parametrizations that are guaranteed to not find a path in (Cost - depth) steps.
   };
   vector<Marking> markings; ///< Actuall marking of the states.

   /**
    * Storest transitions in the form (source, target) within the transitions vector, for the path from the final vertex to the one in the current depth of the DFS procedure.
    * @param which   mask of the parametrizations that allow currently found path
    */
   void storeTransitions(const Paramset which, const size_t depth) {
      vector<pair<StateID, StateID> > trans;  // Temporary storage for the transitions

      // Go from the end till the lastly reached node
      for (size_t step = 0; step < depth; step++) {
         trans.push_back(make_pair(path[step], path[step+1]));
         markings[path[step]].succeeded |= which; // Mark found for given parametrizations
      }
      markings[path[depth]].succeeded |= which;

      // Add transitions to the parametrizations that allow them
      Paramset marker = ParamsetHelper::getLeftOne();
      for (size_t param = 0; param < ParamsetHelper::getSetSize(); param++) {
         if (which & marker) {
            transitions[param].insert(trans.begin(), trans.end());
         }
         marker >>= 1;
      }
   }

   /**
    * Searching procedure itself. This method is called recursivelly based on the depth of the search and passes current parametrizations based on the predecessors.
    * @param ID   ID of the state visited
    * @param paramset   parametrizations passed form the successor
    */
   void DFS(const StateID ID, Paramset paramset) {
      // Add the state to the path
      path[depth] = ID;
      if (depth > max_depth)
         throw runtime_error("Depth boundary overcome during the DFS procedure.");

      // If a way to the source was found, apply it as well
      if (product.isFinal(ID))
         storeTransitions(paramset, depth);

      // Remove those with Cost lower than this level of the search (meaning that nothing more that cycles would be found)
      paramset &= ~depth_masks[depth];

      // Remove parametrizations that already have proven to be used/useless
      for (size_t level = 1; level < depth && paramset; level++)
         paramset &= ~markings[ID].busted[level];

      // If this state already has proven to lie on a path to the source, add this possible successors
      // Note that this works correctly due to the fact, that parametrizations are removed form the BFS during the coloring once they prove acceptable
      Paramset connected = markings[ID].succeeded & paramset;
      if (connected)
         storeTransitions(connected, depth);

      paramset &= ~markings[ID].busted[depth];
      markings[ID].busted[depth] |= paramset; // Forbid usage of these parametrizations for depth levels as high or higher than this one

      // If there is anything left, pass it further to the predecessors
      if (paramset) {
         depth++;

         auto succs = ColoringFunc::broadcastParameters(round_range, product, ID, paramset); // Get predecessors

         for (const Coloring & succ: succs) {
            DFS(succ.first, succ.second); // Recursive descent with parametrizations passed from the predecessor.
         }

         depth--;
      }
   }

   /**
    * Clear the data objects used during the computation that may contain some data from the previous round.
    */
   void clearPaths() {
      // Empty strings
      for(auto & path:string_paths) {
         path = "";
      }
      // Empty path tracker
      path = vector<StateID>(storage.getMaxDepth() + 1, 0);
      // Empty the storage of transitions
      transitions.clear();
      transitions.resize(ParamsetHelper::getSetSize());
      // Clear markings
      for (auto & marking:markings) {
         marking.succeeded = 0;
         marking.busted.assign(marking.busted.size(),0);
      }
   }

   /**
    * Fills a depth_masks vector that specifies which of the parametrizations end at which round.
    */
   void prepareMasks() {
      // clear the data
      depth_masks.clear();

      // Helping data
      vector<vector<size_t> > members(storage.getMaxDepth() + 1); // Stores parametrization numbers with their Cost
      size_t param_num = 0; // number in the interval (0,|paramset|-1)

      // Store parametrization numbers with their BFS level (Cost)
      for (const auto & cost:storage.getCost()) {
         if (cost != INF)
            members[cost].push_back(param_num);
         else
            members[0].push_back(param_num);
         param_num++;
      }

      // Fill masks based on the members vector
      for (const auto & numbers:members) {
         depth_masks.push_back(ParamsetHelper::getMaskFromNums(numbers));
      }
   }

public:
   /**
    * Constructor ensures that data objects used within the whole computation process have appropriate size.
    */
   WitnessSearcher(const ConstructionHolder & _holder, const ColorStorage & _storage)
      : product(_holder.getProduct()), storage(_storage) {
      Marking empty = {0, vector<Paramset>(product.getStateCount(), 0)};
      markings.resize(product.getStateCount(), empty);
      string_paths.resize(ParamsetHelper::getSetSize(), "");
   }

   /**
    * Function that executes the whole searching process
    */
   void findWitnesses(const Range & _round_range) {
      round_range = _round_range;

      // Preparation
      clearPaths();
      prepareMasks();
      depth = 0;
      max_depth = storage.getMaxDepth();

      // Search paths from all the final states
      auto inits = product.getInitialStates();
      for (const auto & init : inits) {
         if (storage.getColor(init))
            DFS(init, storage.getColor(init));
      }
   }

   /**
    * Re-formes the transitions computed during the round into strings.
    * @return  strings with all transitions for each acceptable parametrization
    */
   const vector<string> getOutput() const {
      vector<string> acceptable_paths; // Vector fo actuall data
      // Cycle throught the parametrizations
      for (auto param_it = transitions.begin(); param_it != transitions.end(); param_it++) {
         if (!param_it->empty()) { // Test for emptyness of the set of transitions
            string path = "{";
            // Reformes based on the user request
            for (auto trans_it = param_it->begin(); trans_it != param_it->end(); trans_it++){
               if (!user_options.longWit())
                  path.append(toString(trans_it->first)).append(">").append(toString(trans_it->second)).append(",");
               else
                  path.append(product.getString(trans_it->first)).append(">").append(product.getString(trans_it->second)).append(",");
            }
            if (path.length() == 1)
               path.append("}");
            else
               path[path.length() - 1] = '}';
            // Add the string
            acceptable_paths.push_back(move(path));
         }
      }
      return acceptable_paths;
   }

   /**
    * @return  transitions for each parametrizations in the form (source, target)
    */
   const vector<set<pair<StateID, StateID> > > & getTransitions() const {
      return transitions;
   }
};

#endif // PARSYBONE_WITNESS_SEARCHER_INCLUDED
