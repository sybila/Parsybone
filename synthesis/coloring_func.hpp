#ifndef COLORING_FUNC_HPP
#define COLORING_FUNC_HPP

#include "../construction/product_structure.hpp"

namespace ColoringFunc {
   /**
    * Main function of coloring - creates intersection of passing and transition colors to create and update color.
    * @param passed	parameters that are passed through the transition
    * @param step_size	how many parameters share the same value for given function
    * @param transitive_values	mask of all values from which those that have false are non-transitive
    */
   bool passParameters(const ParamNo param_no, const size_t step_size, const vector<ActLevel> & targets, const bool _dir, const ActLevel val) {
      const size_t value_num = (param_no / step_size) % targets.size();

      if (_dir)
         return targets[value_num] > val;
      else
         return targets[value_num] < val;
   }

   /**
    * Get stripped parameters for each unique edge (if there are multi-edges, intersect their values).
    * @param ID	ID of the source state in the product
    * @param parameters	parameters that will be distributed
    * @return vector of passed parameters together with their targets
    */
   vector<StateID> broadcastParameters(const ParamNo param_no, const ProductStructure & product, const StateID ID) {
      // To store parameters that passed the transition but were not yet added to the target
      vector<StateID> param_updates;

      // Cycle through all the transition
      for (size_t trans_num = 0; trans_num < product.getTransitionCount(ID); trans_num++) {
         StateID target_ID = product.getTargetID(ID, trans_num);

         // From an update strip all the parameters that can not pass through the transition - color intersection on the transition
         if (ColoringFunc::passParameters(param_no, product.getStepSize(ID, trans_num), product.getTargets(ID, trans_num),
                                          product.getDir(ID, trans_num), product.getVal(ID, trans_num)))
            param_updates.push_back(target_ID);
      }

      return param_updates;
   }
}

#endif // COLORING_FUNC_HPP
