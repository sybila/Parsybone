/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_WITNESS_SEARCHER_INCLUDED
#define PARSYBONE_WITNESS_SEARCHER_INCLUDED

#include "coloring_func.hpp"
#include "color_storage.hpp"
#include "synthesis_results.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Class for search of transitions belonging to shortest time series paths.
///
/// Class executes a search through the synthetized space in order to find transitions included in shortest paths for every parametrization.
/// Procedure is supposed to be first executed and then it can provide results.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class WitnessSearcher {
   const ProductStructure & product; ///< Product reference for state properties.
   const ColorStorage & storage; ///< Constant storage with the actuall data.
   ParamNum param_no; ///<

   set<pair<StateID, StateID> >  transitions; ///< Acutall storage of the transitions found - transitions are stored by parametrizations numbers in the form (source, traget).
   string string_path; ///< This vector stores paths for every parametrization (even those that are not acceptable, having an empty string).

   vector<StateID> path; ///< Current path of the DFS with the final vertex on 0 position.
   size_t depth; ///< Current level of the DFS.
   size_t max_depth; ///< Maximal level of recursion that is possible (maximal Cost in this round).
   size_t last_branch; ///< TODO

   /// This structure stores "already tested" paramsets for a state.
   struct Marking {
      size_t succeeded;
      size_t busted; ///< Mask of the parametrizations that are guaranteed to not find a path in (Cost - depth) steps.
   };
   vector<Marking> markings; ///< Actuall marking of the states.

   /**
    * Storest transitions in the form (source, target) within the transitions vector, for the path from the final vertex to the one in the current depth of the DFS procedure.
    * @param which   mask of the parametrizations that allow currently found path
    */
   void storeTransitions(const size_t depth) {
      // Go from the end till the lastly reached node
      for (size_t step = 0; step < depth; step++) {
         transitions.insert(make_pair(path[step], path[step+1]));
         markings[path[step]].succeeded = step; // Mark found for given parametrizations
      }
      markings[path[depth]].succeeded = depth;
   }

   /**
    * Searching procedure itself. This method is called recursivelly based on the depth of the search and passes current parametrizations based on the predecessors.
    * @param ID   ID of the state visited
    * @param paramset   parametrizations passed form the successor
    */
   void DFS(const StateID ID) {
      if (markings[ID].busted <= depth)
         return;
      markings[ID].busted = depth;

      path[depth] = ID;
      assert(depth <= max_depth);

      if (markings[ID].succeeded <= depth)
         storeTransitions(depth);
      else {
         depth++;
         const vector<StateID> & succs = ColoringFunc::broadcastParameters(param_no, product, ID); // Get predecessors
         for (const StateID & succ: succs)
            DFS(succ); // Recursive descent with parametrizations passed from the predecessor.
         depth--;
      }
   }

   /**
    * Clear the data objects used during the computation that may contain some data from the previous round.
    */
   void clearStorage(const SynthesisResults & results) {
      string_path = "";

      path = vector<StateID>(results.getCost(), 0);
      transitions.clear();

      for (auto & marking:markings) {
         marking.succeeded = INF;
         marking.busted = INF;
      }
   }

public:
   /**
    * Constructor ensures that data objects used within the whole computation process have appropriate size.
    */
   WitnessSearcher(const ProductStructure & _product, const ColorStorage & _storage)
      : product(_product), storage(_storage) {
      //      Marking empty = {0, vector<size_t>(ParamsetHelper::getSetSize(), INF)};
      //      markings.resize(product.getStateCount(), empty);
      //      string_paths.resize(ParamsetHelper::getSetSize(), "");
   }

   /**
    * Function that executes the whole searching process
    */
   void findWitnesses(const ParamNum _param_no, const SynthesisResults & results) {
      param_no = _param_no;

      // Preparation
      clearStorage(results);

      // Search paths from all the final states
      auto inits = product.getInitialStates();
      for (const auto & init : inits) {
         if (storage.getColor(init))
            DFS(init);
      }
   }

   /**
    * Re-formes the transitions computed during the round into strings.
    * @return  strings with all transitions for each acceptable parametrization
    */
   const string getOutput() const {
      //      vector<string> acceptable_paths; // Vector fo actuall data
      //      // Cycle throught the parametrizations
      //      for (auto param_it = transitions.begin(); param_it != transitions.end(); param_it++) {
      //         if (!param_it->empty()) { // Test for emptyness of the set of transitions
      //            string path = "{";
      //            // Reformes based on the user request
      //            for (auto trans_it = param_it->begin(); trans_it != param_it->end(); trans_it++){
      //               if (!user_options.use_long_witnesses)
      //                  path.append(toString(trans_it->first)).append(">").append(toString(trans_it->second)).append(",");
      //               else
      //                  path.append(product.getString(trans_it->first)).append(">").append(product.getString(trans_it->second)).append(",");
      //            }
      //            if (path.length() == 1)
      //               path.append("}");
      //            else
      //               path[path.length() - 1] = '}';
      //            // Add the string
      //            acceptable_paths.push_back(move(path));
      //         }
      //      }
      //      return acceptable_paths.front();
      return "no_path";
   }

   /**
    * @return  transitions for each parametrizations in the form (source, target)
    */
   const set<pair<StateID, StateID> > & getTransitions() const {
      return transitions;
   }
};

#endif // PARSYBONE_WITNESS_SEARCHER_INCLUDED
