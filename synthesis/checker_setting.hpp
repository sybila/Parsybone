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
   StateID starting_state;
   StateID final_state;
   bool bounded;
   bool minimal;
   ParamNo tested;
   size_t bfs_bound;

   CheckerSettings() : starting_state(INF), final_state(INF), bounded(false), minimal(false), tested(1ul), bfs_bound(INF) { }

   inline const ParamNo & getTestedNum() const {
      return tested;
   }

   inline bool getBounded() const {
      return bounded;
   }

   inline bool isMinimal() const {
      return minimal;
   }

   inline bool isInitial(const StateID ID, const ProductStructure & product) const {
      if (starting_state != INF)
         return (starting_state == ID);
      else
         return product.isInitial(ID);
   }

   inline bool isFinal(const StateID ID, const ProductStructure & product) const {
      if (final_state != INF)
         return (final_state == ID);
      else
         return product.isFinal(ID);
   }

   inline size_t getBound() const {
      return bfs_bound;
   }

   inline StateID getCoreState() const {
      return starting_state;
   }

   const set<StateID> hashInitials(const ProductStructure & product) const {
      if (starting_state != INF)
         return set<StateID>();
      else
         return set<StateID>(product.getInitialStates().begin(), product.getInitialStates().end());
   }
};

#endif // CHECKER_SETTING_HPP
