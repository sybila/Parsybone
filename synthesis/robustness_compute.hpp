/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_ROBUSTNESS_COMPUTE_INCLUDE
#define PARSYBONE_ROBUSTNESS_COMPUTE_INCLUDE

#include "witness_searcher.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Class responsible for computation of robustness values for each acceptable parametrization.
///
/// @attention Now broken due to the fact that initials do not match initial measurement.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class RobustnessCompute {
   const ProductStructure & product; ///< Product reference for state properties.
   const ColorStorage & storage; ///< Constant storage with the actuall data.
   ParamNo param_no; ///< Range of parametrizations used this round

   /// This structure holds values used in the iterative process of robustness computation.
   vector<size_t> exits; ///< A number of transitions this state can be left through under given parametrization.
   vector<double> current_prob; ///< Current probability of reaching.
   vector<double> next_prob; ///< Will store the probability in the next round.

   double robustness; /// Resultig probability for each parametrization.

   /**
    * For each state compute how many exists are under each parametrization.
    */
   void computeExits(const vector<Transition> & transitions) {
      // If not acceptable, leave zero
      for (const Transition & tran : transitions) {
         auto succs = ColoringFunc::broadcastParameters(param_no, product, tran.first);

         // Consider only the steps that go towards a single state of the BA (picked the highest).
         StateID max_BA = 0;
         for (const StateID succ:succs)
            max_BA = max(max_BA, product.getBAID(succ));
         for (const StateID succ:succs)
            exits[tran.first] += static_cast<size_t>(max_BA == product.getBAID(succ));
      }
   }


   /**
    * Set probability of each initial state to 1.0 / number of used initial states for this parametrization.
    */
   void initiate() {
      robustness = 0;
      exits.assign(exits.size(), 0);
      current_prob.assign(current_prob.size(), 0.);
      next_prob = current_prob;

      setInitials();
   }

   /**
    * @brief setInitials
    */
   void setInitials() {
      const vector<StateID> & initials = product.getInitialStates();

      for (const StateID init:initials)
         next_prob[init] = 1.0 / initials.size();
   }

   /**
    * Compute the resulting values as a sum of probabilites of reaching any state.
    */
   void finish() {
      for (StateID ID:product.getFinalStates())
         robustness += next_prob[ID];
   }

public:
   /**
    * Constructor ensures that data objects used within the whole computation process have appropriate size.
    */
   RobustnessCompute(const ProductStructure & _product, const ColorStorage & _storage) : product(_product), storage(_storage) {
      exits.resize(product.getStateCount());
      current_prob.resize(product.getStateCount());
      next_prob.resize(product.getStateCount());
   }

   /**
    * Function that computes robustness values for each parametrization.
    */
   void compute(const ParamNo _param_no, const SynthesisResults & results, const vector<pair<StateID,StateID> > & transitions) {
      param_no = _param_no;
      initiate();
      computeExits(transitions);

      // Cycle through the levels of the DFS procedure
      for (size_t round_num = 0; round_num < results.getCost(); round_num++) {
         // Copy the data from the previous round.
         current_prob = next_prob;
         next_prob.assign(next_prob.size(), 0.);
         // Assign probabilites for the initial states
         setInitials();
         // For the parametrization cycle through transitions
         for (const auto & trans:transitions) {
            size_t divisor = exits[trans.first]; // Count succesor
            // Add probabilities
            if (divisor)
               next_prob[trans.second] += current_prob[trans.first] / divisor ;
         }
      }

      finish();
   }

   /**
    * @return the current robustness
    */
   double getRobustness() const {
      return robustness;
   }

   /**
    * @return robustness as a string
    */
   const string getOutput() const {
      return toString(robustness);
   }
};

#endif // PARSYBONE_ROBUSTNESS_COMPUTE_INCLUDE
