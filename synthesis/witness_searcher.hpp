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
#include "coloring_analyzer.hpp"
#include "color_storage.hpp"

class WitnessSearcher {
   const ProductStructure & product;
   const ColoringAnalyzer & analyzer;
   const ColorStorage & storage;
   ColorStorage workspace;

   std::vector<std::string> string_paths;

   std::vector<StateID> path;
   std::vector<Paramset> depth_masks;
   std::size_t depth;
   std::size_t fork_depth;
   std::size_t max_depth;

   struct Marking {
      Paramset succeeded;
      std::vector<Paramset> busted;
   };

   std::vector<Marking> markings;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SEARCH FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   void storeTransitions(const Paramset which) {
      std::string path_str;
      for (std::size_t step = my_max(fork_depth,1); step < depth; step++) {
         path_str.append("[").append(toString(path[step])).append("<").append(toString(path[step+1])).append("]");
         markings[path[step]].succeeded = which;
      }

      Paramset marker = paramset_helper.getLeftOne();
      for (std::size_t param = 0; param < paramset_helper.getParamsetSize(); param++) {
         if (which & marker)
            string_paths[param].append(path_str);
         marker >>= 1;
      }
      fork_depth = depth;
   }

   void DFS(const StateID ID, Paramset paramset) {
      path[depth] = ID;
      if (depth > max_depth)
         throw std::runtime_error("Depth boundary overcome.");

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
         auto predecessors = workspace.getNeighbours(ID, false, paramset);
         for (auto pred = predecessors.begin(); pred != predecessors.end(); pred++) {
            DFS(*pred, paramset);
         }
         depth--;
      }
      fork_depth = my_min(depth-1, fork_depth);
   }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   void clearPaths() {
      if (string_paths.size() != paramset_helper.getParamsetSize())
         string_paths.resize(paramset_helper.getParamsetSize(), "");
      else
         forEach(string_paths, [](std::string & single_path){single_path = "";});
      path = std::vector<StateID>(getMaxDepth() + 1, 0);
   }

   void prepareMasks() {
      depth_masks.clear();
      std::vector<std::vector<std::size_t> > members(getMaxDepth() + 1);
      std::size_t param_num = 0;
      forEach(workspace.getCost(), [&members, &param_num](std::size_t current){
         if (current != ~0)
            members[current].push_back(param_num);
         param_num++;
      });
      forEach(members, [&](std::vector<std::size_t> numbers){
          depth_masks.push_back(paramset_helper.getMaskFromNums(numbers));
      });

      forEach(markings, [](Marking & marking){marking.succeeded = 0; marking.busted.assign(marking.busted.size(),0);});
   }

   WitnessSearcher(const WitnessSearcher & other); ///< Forbidden copy constructor.
   WitnessSearcher& operator=(const WitnessSearcher & other); ///< Forbidden assignment operator.

public:
   /**
    * Constructor, passes the data
    */
   WitnessSearcher(const ConstructionHolder & _holder, const ColoringAnalyzer & _analyzer, const ColorStorage & _storage)
      : product(_holder.getProduct()), analyzer(_analyzer), storage(_storage) {
      Marking empty = {0, std::vector<Paramset>(product.getStateCount(), 0)};
      markings.resize(product.getStateCount(), empty);
   }

   const std::size_t getMaxDepth () const {
      std::size_t depth = 0;
      forEach(workspace.getCost(), [&depth](std::size_t current){depth = my_max((current == ~0 ? 0 : current), depth);});
      return depth;
   }

   const std::vector<std::string> getOutput () {
      workspace = storage;
      for (StateID ID = 0; ID < product.getStateCount(); ID++) {
         workspace.remove(ID, ~0);
      }
      clearPaths();
      prepareMasks();
      depth = fork_depth = 0;
      max_depth = getMaxDepth();

      auto finals = product.getFinalStates();
      for (auto final = finals.begin(); final != finals.end(); final++) {
         if (storage.getColor(*final))
            DFS(*final, storage.getColor(*final));
      }

      std::vector<std::string> acceptable_paths;
      for (auto path_it = string_paths.begin(); path_it != string_paths.end(); path_it++)
         if (!path_it->empty())
            acceptable_paths.push_back(std::move(*path_it));
      return acceptable_paths;
   }
};

#endif
