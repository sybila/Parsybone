#ifndef CHECKER_SETTING_HPP
#define CHECKER_SETTING_HPP

#include "../auxiliary/paramset_helper.hpp"
#include "../construction/product_structure.hpp"

class CheckerSettings {
public:
   const ProductStructure & product;
   StateID starting_state;
   StateID final_state;
   bool bounded;
   bool minimal;
   Range range;
   size_t bfs_bound;
   Paramset tested_params;

   CheckerSettings(const ProductStructure & _product) : product(_product), starting_state(INF), final_state(INF),
      bounded(false), minimal(false), range({0lu,0lu}), bfs_bound(INF), tested_params(ParamsetHelper::getNone()) { }

   void copyData(const CheckerSettings & other) {
      starting_state = other.starting_state;
      final_state = other.final_state;
      bounded = other.bounded;
      minimal = other.minimal;
      range = other.range;
      bfs_bound = other.bfs_bound;
      tested_params = other.tested_params;
   }

   CheckerSettings operator=(const CheckerSettings & other) {
      if (&other.product != &product)
         throw runtime_error("Copying model checker settings for different product.");
      copyData(other);

      return *this;
   }

   CheckerSettings(const CheckerSettings & other) : product(other.product) {
      copyData(other);
   }

   inline Paramset getStartingParams() const {
      return tested_params;
   }

   inline const Range & getRange() const {
      return range;
   }

   inline bool isBounded() const {
      return bounded;
   }

   inline bool isMinimal() const {
      return minimal;
   }

   inline bool isInitial(const StateID ID) const {
      if (starting_state != INF)
         return (starting_state == ID);
      else
         return product.isInitial(ID);
   }

   inline bool isFinal(const StateID ID) const {
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

   const set<StateID> hashInitials() const {
      if (starting_state != INF)
         return set<StateID>();
      else
         return set<StateID>(product.getInitialStates().begin(), product.getInitialStates().end());
   }
};

#endif // CHECKER_SETTING_HPP
