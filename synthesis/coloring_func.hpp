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
   bool passParameters(const ParamNum param_no, const size_t step_size, const vector<ActLevel> & targets, const Direction comp, const ActLevel val) {
      size_t value_num = (param_no / step_size) % targets.size();

      switch (comp) {
      case up_dir:
         return targets[value_num] > val;
      case stay_dir:
         return targets[value_num] == val;
      case down_dir:
         return targets[value_num] < val;
      }
   }

   /**
    * Get stripped parameters for each unique edge (if there are multi-edges, intersect their values).
    * @param ID	ID of the source state in the product
    * @param parameters	parameters that will be distributed
    * @return vector of passed parameters together with their targets
    */
   vector<StateID> broadcastParameters(const ParamNum param_no,  const ProductStructure & product, const StateID ID) {
      // To store parameters that passed the transition but were not yet added to the target
      set<StateID> BA_presence;
      vector<StateID> param_updates;

      size_t KS_state = product.getKSID(ID);
      bool loop = true; // Which of the parameters allow only to remain in this state

      // Cycle through all the transition
      for (size_t trans_num = 0; trans_num < product.getTransitionCount(ID); trans_num++) {
         StateID target_ID = product.getTargetID(ID, trans_num);

         // From an update strip all the parameters that can not pass through the transition - color intersection on the transition
         bool passed = ColoringFunc::passParameters(param_no, product.getStepSize(ID, trans_num), product.getTargets(ID, trans_num),
                                                    product.getOp(ID, trans_num), product.getVal(ID, trans_num));

         // Test if it is a possibility for a loop, if there is nothing outcoming, add to self-loop (if it is still possible)
         if (passed) {
            if (loop && KS_state == product.getKSID(target_ID) ) {
               StateID BA_ID = product.getBAID(target_ID);
               BA_presence.insert(BA_ID);
            } else  {
               param_updates.push_back(target_ID);
               loop = false;
            }
         }
      }

      // If there is a self-loop, add it for all the BA states (its an intersection of transitional parameters for independent loops)
      if (loop) {
         for(const StateID BA_state:BA_presence) {
            param_updates.push_back(product.getProductID(KS_state, BA_state));
         }
      }

      // Return all filled updates
      return param_updates;
   }
}

#endif // COLORING_FUNC_HPP
