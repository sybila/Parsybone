/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef CHECKER_SETTING_HPP
#define CHECKER_SETTING_HPP

#include "../construction/product_structure.hpp"

class CheckerSettings {
public:
   vector<StateID> initial_states;
   vector<StateID> final_states;
   bool minimal;
   ParamNo param_no;
   size_t bfs_bound;
   bool mark_initals;

   CheckerSettings() :  minimal(false), param_no(1ul), bfs_bound(INF), mark_initals(false) { }

   inline const ParamNo & getParamNo() const {
      return param_no;
   }

   inline bool isMinimal() const {
      return minimal;
   }

   inline bool isInitial(const StateID ID, const ProductStructure & product) const {
      if (initial_states.empty())
         return product.isInitial(ID);
      else
         return (find(initial_states.begin(),initial_states.end(), ID) != initial_states.end());
   }

   inline bool isFinal(const StateID ID, const ProductStructure & product) const {
      if (final_states.empty())
         return product.isFinal(ID);
      else
         return (find(final_states.begin(),final_states.end(), ID) != final_states.end());
   }

   inline const vector<StateID> & getInitials(const ProductStructure & product) const {
      if (initial_states.empty())
         return product.getInitialStates();
      else
         return initial_states;
   }

   inline const vector<StateID> & getFinals(const ProductStructure & product) const {
      if (final_states.empty())
         return product.getFinalStates();
      else
         return final_states;
   }

   inline size_t getBound() const {
      return bfs_bound;
   }

   inline bool markInitials() const {
      return mark_initals;
   }
};

#endif // CHECKER_SETTING_HPP
