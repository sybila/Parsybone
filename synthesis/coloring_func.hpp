#ifndef COLORING_FUNC_HPP
#define COLORING_FUNC_HPP

#include "../construction/product_structure.hpp"
#include "paramset_helper.hpp"

namespace ColoringFunc {
   /**
    * Main function of coloring - creates intersection of passing and transition colors to create and update color.
    *
    * @param passed	parameters that are passed through the transition
    * @param step_size	how many parameters share the same value for given function
    * @param transitive_values	mask of all values from which those that have false are non-transitive
    */
   void passParameters(const Range & synthesis_range, Paramset & passed, const size_t step_size, const vector<bool> & transitive_values) {
      // INITIALIZATION OF VALUES FOR POSITIONING
      // Number of the first parameter
      ParamNum param_num = synthesis_range.first;
      // First value might not bet 0 - get it from current parameter position
      size_t value_num = (param_num / step_size) % transitive_values.size();
      // Mask that will be created
      register Paramset temporary = 0;

      // COMPUTATION OF THE MASK
      // List through all the paramters
      while (true) {
         // List through ALL the target values
         for (; value_num < transitive_values.size(); value_num++) {
            // Get size of the step for current value
            size_t bits_in_step = min<size_t>((step_size - (param_num % step_size)), static_cast<size_t>(synthesis_range.second - param_num));
            // Move the mask so new value data can be add
            temporary <<= bits_in_step;
            // If transitive, add ones for the width of the step
            if (transitive_values[value_num]) {
               Paramset add = INF;
               add >>= (ParamsetHelper::getSetSize() - bits_in_step);
               temporary |= add;
            }
            // If we went throught the whole size, end
            if ((param_num += bits_in_step) == synthesis_range.second) {
               // Create interection of source parameters and transition parameters
               passed &= temporary;
               return;
            }
         }
         // Reset the value
         value_num = 0;
      }
   }

   /**
    * Get stripped parameters for each unique edge (if there are multi-edges, intersect their values).
    *
    * @param ID	ID of the source state in the product
    * @param parameters	parameters that will be distributed
    *
    * @return vector of passed parameters together with their targets
    */
   vector<Coloring> broadcastParameters(const Range & synthesis_range, const ProductStructure & product, const StateID ID, const Paramset parameters) {
      // To store parameters that passed the transition but were not yet added to the target
      set<StateID> BA_presence;
      vector<Coloring> param_updates;
      param_updates.reserve(product.getTransitionCount(ID));

      size_t KS_state = product.getKSID(ID);
      Paramset loop_params = INF; // Which of the parameters allow only to remain in this state

      // Cycle through all the transition
      for (size_t trans_num = 0; trans_num < product.getTransitionCount(ID); trans_num++) {
         StateID target_ID = product.getTargetID(ID, trans_num);

         // Parameters to pass through the transition
         Paramset passed = parameters;

         // From an update strip all the parameters that can not pass through the transition - color intersection on the transition
         ColoringFunc::passParameters(synthesis_range, passed, product.getStepSize(ID, trans_num), product.getTransitive(ID, trans_num));

         // Test if it is a possibility for a loop, if there is nothing outcoming, add to self-loop (if it is still possible)
         if (KS_state == product.getKSID(target_ID) ) {
            loop_params &= passed;
            if(loop_params) {
               StateID BA_ID = product.getBAID(target_ID);
               BA_presence.insert(BA_ID);
            }
         }
         // Else add normally and remove from the loop
         else if (passed) {
            loop_params &= ~passed; // Retain only others within a loop
            param_updates.push_back(make_pair(target_ID, passed));
         }
      }

      // If there is a self-loop, add it for all the BA states (its an intersection of transitional parameters for independent loops)
      if (loop_params) {
         for(const StateID BA_state:BA_presence) {
            StateID target = product.getProductID(KS_state, BA_state) ;
            param_updates.push_back(Coloring(target, loop_params));
         }
      }

      // Return all filled updates
      return param_updates;
   }
}

#endif // COLORING_FUNC_HPP
