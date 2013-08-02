#ifndef COLORING_FUNC_HPP
#define COLORING_FUNC_HPP

#include "../auxiliary/data_types.hpp"

namespace ColoringFunc {
   /**
    * Main function of coloring - creates intersection of passing and transition colors to create and update color.
    *
    * @param passed	parameters that are passed through the transition
    * @param step_size	how many parameters share the same value for given function
    * @param transitive_values	mask of all values from which those that have false are non-transitive
    */
   void passParameters(const Range synthesis_range, Paramset & passed, const size_t step_size, const vector<bool> & transitive_values) {
      // INITIALIZATION OF VALUES FOR POSITIONING
      // Number of the first parameter
      ParamNum param_num = synthesis_range.first;
      // First value might not bet 0 - get it from current parameter position
      size_t value_num = (param_num / step_size) % transitive_values.size();
      // Mask that will be created
      register Paramset temporary = 0;

      // COMPUTATION OF MASK
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
               add >>= (paramset_helper.getSetSize() - bits_in_step);
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
}

#endif // COLORING_FUNC_HPP
