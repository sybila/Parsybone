/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef SYNTHESIS_RESULTS_HPP
#define SYNTHESIS_RESULTS_HPP

#include "../auxiliary/data_types.hpp"

struct SynthesisResults {
   bool is_accepting; ///< a mask for parametrizations accepting in this round
   map<StateID, size_t> found_depth; ///< when a final state was found
   size_t min_acc;
   size_t max_acc;
   map<size_t, size_t> depths;

   SynthesisResults() : SynthesisResults(1, INF) { }

   SynthesisResults(const size_t min_acc, const size_t max_acc) {
      this->min_acc = min_acc;
      this->max_acc = max_acc;
      is_accepting = false;
   }

   /**
    * @brief derive  information from stored final states
    */
   void derive() {
      is_accepting = (min_acc <= found_depth.size()) && (max_acc >= found_depth.size());

      for (const pair<StateID, size_t> & state : found_depth) {
         auto it = depths.find(state.second);
         if (it != depths.end()) {
            it->second++;
         } else {
            depths.insert(make_pair(state.second, 1u));
         }
      }
   }

   size_t getLowerBound() const {
      if (depths.empty())
         return INF;
      else
         return depths.begin()->first;
   }
};

#endif // SYNTHESIS_RESULTS_HPP
