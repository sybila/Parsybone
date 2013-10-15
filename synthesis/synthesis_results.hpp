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
   size_t lower_bound; ///< costs of individual parametrizations used this round
   bool is_accepting; ///< a mask for parametrizations accepting in this round
   map<StateID, size_t> found_depth; ///< when a final state was found

   SynthesisResults() {
      lower_bound = INF;
      is_accepting = false;
   }

   /**
    * @brief derive  information from stored final states
    */
   void derive() {
      is_accepting = !found_depth.empty();
      for (const pair<StateID, size_t> & state : found_depth)
         lower_bound = min(lower_bound, state.second);
   }
};

#endif // SYNTHESIS_RESULTS_HPP
