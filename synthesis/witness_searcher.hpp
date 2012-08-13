/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_WITNESS_SEARCHER_INCLUDED
#define PARSYBONE_WITNESS_SEARCHER_INCLUDED

#include "../construction/construction_holder.hpp"
#include "color_storage.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Class executes a search through the synthetized space in order to find transitions included in shortest paths for every parametrization.
/// Procedure is supposed to be first executed and then it can provide results.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class WitnessSearcher {
   const ProductStructure & product; ///< Product reference for state properties
   const ColorStorage & storage; ///< Constant storage with the actuall data

   /// Acutall storage of the transitions found - transitions are stored by parametrizations numbers in the form (source, traget)
   std::vector<std::set<std::pair<StateID, StateID> > > transitions;
   /// Vector storing for each parametrization initial states it reached
   std::vector<std::vector<StateID> > initials;

   std::vector<std::string> string_paths; ///< This vector stores paths for every parametrization (even those that are not acceptable, having an empty string)

   std::vector<StateID> path; ///< Current path of the DFS with the final vertex on 0 position
   std::vector<Paramset> depth_masks; ///< For each of levels of DFS, stores mask of parametrizations with corresponding cost (those that are not furter used in the DFS)
   std::size_t depth; ///< Current level of the DFS
   std::size_t max_depth; ///< Maximal level of recursion that is possible (maximal Cost in this round).

   /// This structure stores "already tested" paramsets for a state
   struct Marking {
      Paramset succeeded; ///< Mask of those parametrizations that have found a paths from this state.
      std::vector<Paramset> busted; ///< Mask of the parametrizations that are guaranteed to not find a path in (Cost - depth) steps.
   };

   std::vector<Marking> markings; ///< Actuall marking of the sates.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SEARCH FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   /**
    * Storest transitions in the form (source, target) within the @var transitions vector, for the path from the final vertex to the one in the current depth of the DFS procedure.
    *
    * @param which   mask of the parametrizations that allow currently found path
    * @param initil  if true, stores also the last node as an initial one for given parametrizations
    */
   void storeTransitions(const Paramset which, bool initial) {
      std::vector<std::pair<StateID, StateID> > trans;  // Temporary storage for the transitions

      // Go from the end till the lastly reached node
      for (std::size_t step = 0; step < depth; step++) {
         trans.push_back(std::make_pair(path[step+1], path[step]));
         markings[path[step]].succeeded = which; // Mark found for given parametrizations
      }
      markings[path[depth]].succeeded = which;

      // Add transitions to the parametrizations that allow them
      Paramset marker = paramset_helper.getLeftOne();
      for (std::size_t param = 0; param < paramset_helper.getParamsetSize(); param++) {
         if (which & marker) {
            transitions[param].insert(trans.begin(), trans.end());
            if (initial)
               initials[param].push_back(path[depth]);
         }
         marker >>= 1;
      }
   }

   /**
    * Searching procedure itself. This method is called recursivelly based on the depth of the search and passes current parametrizations based on the predecessors.
    *
    * @param ID   ID of the state visited
    * @param paramset   parametrizations passed form the successor
    */
   void DFS(const StateID ID, Paramset paramset) {
      // Add the state to the path
      path[depth] = ID;
      if (depth > max_depth)
         throw std::runtime_error("Depth boundary overcome during the DFS procedure.");

      // If this state already has proven to lie on a path to the source, add this possible successors
      // Note that this works correctly due to the fact, that parametrizations are removed form the BFS during the coloring once they prove acceptable
      Paramset connected = markings[ID].succeeded & paramset;
      if (connected)
         storeTransitions(connected, false);

      // If a way to the source was found, apply it as well
      if (product.isInitial(ID))
         storeTransitions(paramset, true);

      // Remove those with Cost lower than this level of the search (meaning that nothing more that cycles would be found)
      paramset &= ~depth_masks[depth];

      // Remove parametrizations that already have proven to be used/useless
      for (std::size_t level = 1; level <= depth && paramset; level++)
         paramset &= ~markings[ID].busted[level];
      markings[ID].busted[depth] |= paramset; // Forbid usage of these parametrizations for depth levels as high or higher than this one

      // If there is anything left, pass it further to the predecessors
      if (paramset) {
         depth++;

         auto predecessors = std::move(storage.getNeighbours(ID, false, paramset)); // Get predecessors
         auto pred_label = std::move(storage.getMarking(ID, false, paramset)); auto label_it = pred_label.begin(); // Get its values

         for (auto pred = predecessors.begin(); pred != predecessors.end(); pred++, label_it++) {
            DFS(*pred, paramset & *label_it); // Recursive descent with parametrizations passed from the predecessor.
         }

         depth--;
      }
   }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   /**
    * Clear the data objects used during the computation that may contain some data from the previous round
    */
   void clearPaths() {
      // Empty strings
      forEach(string_paths, [](std::string & single_path){single_path = "";});
      // Empty path tracker
      path = std::vector<StateID>(storage.getMaxDepth() + 1, 0);
      // Empty the storage of transitions
      transitions.clear();
      transitions.resize(paramset_helper.getParamsetSize());
      // Empty the storage of inital states
      initials.clear();
      initials.resize(paramset_helper.getParamsetSize());
      // Clear markings
      forEach(markings, [](Marking & marking){
         marking.succeeded = 0;
         marking.busted.assign(marking.busted.size(),0);
      });
   }

   /**
    * Fills a depth_masks vector that specifies which of the parametrizations end at which round
    */
   void prepareMasks() {
      // clear the data
      depth_masks.clear();

      // Helping data
      std::vector<std::vector<std::size_t> > members(storage.getMaxDepth() + 1); // Stores parametrization numbers with their Cost
      std::size_t param_num = 0; // number in the interval (0,|paramset|-1)

      // Store parametrization numbers with their BFS level (Cost)
      forEach(storage.getCost(), [&members, &param_num](std::size_t current){
         if (current != ~0)
            members[current].push_back(param_num);
         param_num++;
      });

      // Fill masks based on the members vector
      forEach(members, [&](std::vector<std::size_t> numbers){
         depth_masks.push_back(paramset_helper.getMaskFromNums(numbers));
      });

   }

   WitnessSearcher(const WitnessSearcher & other); ///< Forbidden copy constructor.
   WitnessSearcher& operator=(const WitnessSearcher & other); ///< Forbidden assignment operator.

public:
   /**
    * Constructor ensures that data objects used within the whole computation process have appropriate size
    */
   WitnessSearcher(const ConstructionHolder & _holder, const ColorStorage & _storage)
      : product(_holder.getProduct()), storage(_storage) {
      Marking empty = {0, std::vector<Paramset>(product.getStateCount(), 0)};
      markings.resize(product.getStateCount(), empty);
      string_paths.resize(paramset_helper.getParamsetSize(), "");
   }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INTERFACE
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   /**
    * Function that executes the whole searching process
    */
   void findWitnesses() {
      // Preparation
      clearPaths();
      prepareMasks();
      depth = 0;
      max_depth = storage.getMaxDepth();

      // Search paths from all the final states
      auto finals = product.getFinalStates();
      for (auto final = finals.begin(); final != finals.end(); final++) {
         if (storage.getColor(*final))
            DFS(*final, storage.getColor(*final));
      }
   }

   /**
    * Reformats the transitions computed in the round into the strings
    *
    * @return  string with all transitions for each resulting parametrization
    */
   const std::vector<std::string> getOutput() const {
      std::vector<std::string> acceptable_paths; // Vector fo actuall data
      // Cycle throught the parametrizations
      for (auto param_it = transitions.begin(); param_it != transitions.end(); param_it++) {
         if (!param_it->empty()) { // Test for emptyness
            std::string path;
            // Reformat based on the user request
            for (auto trans_it = param_it->begin(); trans_it != param_it->end(); trans_it++){
               if (!user_options.BA())
                  path.append("[").append(toString(trans_it->first)).append(">").append(toString(trans_it->second)).append("]");
               else
                  path.append(",").append(product.getString(trans_it->first)).append(">").append(product.getString(trans_it->second));
            }
            // Add the string
            acceptable_paths.push_back(std::move(path));
         }
      }
      return acceptable_paths;
   }

   /**
    * @retur   transitions for each parametrizations in the form (source, target)
    */
   const std::vector<std::set<std::pair<StateID, StateID> > > & getTransitions() const {
      return transitions;
   }

   /**
    * @return  a vector of IDs of intial states
    */
   const std::vector<std::vector<StateID> > & getInitials() const {
      return initials;
   }
};

#endif
