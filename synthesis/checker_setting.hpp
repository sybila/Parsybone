#ifndef CHECKER_SETTING_HPP
#define CHECKER_SETTING_HPP

#include "../construction/product_structure.hpp"

class CheckerSettings {
public:
   const ProductStructure & product;
   StateID starting_state;
   StateID final_state;
   bool bounded;
   bool minimal;
   ParamNum tested;
   size_t bfs_bound;

   CheckerSettings(const ProductStructure & _product) : product(_product), starting_state(INF), final_state(INF), bounded(false), minimal(false), tested(0ul), bfs_bound(INF) { }

   void copyData(const CheckerSettings & other) {
      starting_state = other.starting_state;
      final_state = other.final_state;
      bounded = other.bounded;
      minimal = other.minimal;
      tested = other.tested;
      bfs_bound = other.bfs_bound;
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

   inline const ParamNum & getTestedNum() const {
      return tested;
   }

   inline bool getBounded() const {
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
