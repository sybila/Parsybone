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

class SynthesisResults {
   size_t lower_bound; ///< costs of individual parametrizations used this round
   bool is_accepting; ///< a mask for parametrizations accepting in this round

public:
   /**
    * Fills after time series check finished.
    * @param new_costs	a cost value. If the value does not exist (state is not reachable), use INF
    */
   void setResults(const size_t & _lower_bound, const bool _is_accepting) {
      lower_bound = _lower_bound;
      is_accepting = _is_accepting;
   }


   size_t getCost() const {
      return lower_bound;
   }

   const bool & isAccepting() const {
      return is_accepting;
   }
};

#endif // SYNTHESIS_RESULTS_HPP
