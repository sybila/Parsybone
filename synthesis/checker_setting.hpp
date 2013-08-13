#ifndef CHECKER_SETTING_HPP
#define CHECKER_SETTING_HPP

#include "../auxiliary/paramset_helper.hpp"
#include "../construction/product_structure.hpp"

class CheckerSettings {
public:
   const ProductStructure & product;
   StateID initial_state;
   StateID final_state;
   bool bounded;
   Range range;
   size_t bfs_bound;
   Paramset tested_params;

   CheckerSettings(const ProductStructure & _product) : product(_product), initial_state(INF), final_state(INF),
      bounded(false), range({0lu,0lu}), bfs_bound(INF), tested_params(ParamsetHelper::getNone()) { }

   Paramset getStartingParams() {
      return tested_params;
   }

   const Range & getRange() {
      return range;
   }

   bool isBounded() const {
      return bounded;
   }

   bool isInitial(const StateID ID) const {
      if (initial_state != INF)
         return (initial_state == ID);
      else
         return product.isInitial(ID);
   }

   bool isFinal(const StateID ID) const {
      if (final_state != INF)
         return (final_state == ID);
      else
         return product.isFinal(ID);
   }

   size_t getBound() {
      return bfs_bound;
   }

   const set<StateID> hashInitials() const {
      return set<StateID>(product.getInitialStates().begin(), product.getInitialStates().end());
   }
};

#endif // CHECKER_SETTING_HPP
