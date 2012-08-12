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

class WitnessSearcher {
   const ProductStructure & product; ///< Product reference for state properties
   const ColorStorage & storage; ///< Constant storage with the actuall data

   std::vector<std::set<std::pair<StateID, StateID> > > transitions;
   std::vector<std::string> string_paths; ///< This vector stores paths for every parametrization (even those that are not acceptable)

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
   void storeTransitions(const Paramset which) {
      std::vector<std::pair<StateID, StateID> >  trans;
      for (std::size_t step = 0; step < depth; step++) {
         trans.push_back(std::make_pair(path[step+1], path[step]));
         markings[path[step]].succeeded = which;
         // path_str.append("[").append(toString(path[step])).append("<").append(toString(path[step+1])).append("]");
      }

      Paramset marker = paramset_helper.getLeftOne();
      for (std::size_t param = 0; param < paramset_helper.getParamsetSize(); param++) {
         if (which & marker)
            transitions[param].insert(trans.begin(), trans.end());
         marker >>= 1;
      }
   }

   void DFS(const StateID ID, Paramset paramset) {
      path[depth] = ID;
      if (depth > max_depth)
         throw std::runtime_error("Depth boundary overcome during the DFS procedure.");

      Paramset connected = markings[ID].succeeded & paramset;
      if (connected)
         storeTransitions(connected);
      paramset &= ~connected;

      if (product.isInitial(ID)) {
         storeTransitions(paramset);
      }

      paramset &= ~depth_masks[depth];

      for (std::size_t level = 1; level <= depth && paramset; level++)
         paramset &= ~markings[ID].busted[level];
      markings[ID].busted[depth] |= paramset;

      if (paramset) {
         depth++;
         auto predecessors = storage.getNeighbours(ID, false, paramset);
         for (auto pred = predecessors.begin(); pred != predecessors.end(); pred++) {
            DFS(*pred, paramset);
         }
         depth--;
      }
   }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   void clearPaths() {
      if (string_paths.size() != paramset_helper.getParamsetSize())
         string_paths.resize(paramset_helper.getParamsetSize(), "");
      else
         forEach(string_paths, [](std::string & single_path){single_path = "";});
      path = std::vector<StateID>(storage.getMaxDepth() + 1, 0);
   }

   void prepareMasks() {
      depth_masks.clear();
      std::vector<std::vector<std::size_t> > members(storage.getMaxDepth() + 1);
      std::size_t param_num = 0;

      forEach(storage.getCost(), [&members, &param_num](std::size_t current){
         if (current != ~0)
            members[current].push_back(param_num);
         param_num++;
      });

      forEach(members, [&](std::vector<std::size_t> numbers){
         depth_masks.push_back(paramset_helper.getMaskFromNums(numbers));
      });

      forEach(markings, [](Marking & marking){
         marking.succeeded = 0;
         marking.busted.assign(marking.busted.size(),0);
      });

      transitions.clear();
      transitions.resize(paramset_helper.getParamsetSize());
   }

   WitnessSearcher(const WitnessSearcher & other); ///< Forbidden copy constructor.
   WitnessSearcher& operator=(const WitnessSearcher & other); ///< Forbidden assignment operator.

public:
   /**
    * Constructor, passes the data
    */
   WitnessSearcher(const ConstructionHolder & _holder, const ColorStorage & _storage)
      : product(_holder.getProduct()), storage(_storage) {
      Marking empty = {0, std::vector<Paramset>(product.getStateCount(), 0)};
      markings.resize(product.getStateCount(), empty);
   }

   void findWitnesses() {
      clearPaths();
      prepareMasks();
      depth = 0;
      max_depth = storage.getMaxDepth();

      auto finals = product.getFinalStates();
      for (auto final = finals.begin(); final != finals.end(); final++) {
         if (storage.getColor(*final))
            DFS(*final, storage.getColor(*final));
      }
   }

   const std::vector<std::string> getOutput() const {
      std::vector<std::string> acceptable_paths;
      for (auto param_it = transitions.begin(); param_it != transitions.end(); param_it++) {
         if (!param_it->empty()) {
            std::string path;
            for (auto trans_it = param_it->begin(); trans_it != param_it->end(); trans_it++){
               if (!user_options.BA())
                  path.append("[").append(toString(trans_it->first)).append(">").append(toString(trans_it->second)).append("]");
               else
                  path.append(",").append(product.getString(trans_it->first)).append(">").append(product.getString(trans_it->second));
            }
            acceptable_paths.push_back(std::move(path));
         }
      }
      return acceptable_paths;
   }


   const std::vector<std::set<std::pair<StateID, StateID> > > & getTransitions() const {
      return transitions;
   }
};

#endif
