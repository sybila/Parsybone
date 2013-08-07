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

/// Currently broken.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Class responsible for computation of robustness values for each acceptable parametrization.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class RobustnessCompute {
   const ProductStructure & product; ///< Product reference for state properties.
   const ColorStorage & storage; ///< Constant storage with the actuall data.
   const WitnessSearcher & searcher; ///< Reference to the searcher that contains transitions.
   Range round_range; ///< Range of parametrizations used this round

   /// This structure holds values used in the iterative process of robustness computation.
   struct Marking {
      vector<unsigned char> exits; ///< For each parametrization stores a number of transitions this state can be left through under given parametrization.
      vector<double> current_prob; ///< For each parametrization stores current probability of reaching.
      vector<double> next_prob; ///< For each parametrizations will store the probability in the next round.
   };

   vector<Marking> markings; /// Marking of all states.
   vector<double> results; /// Resultig probability for each parametrization.

   /**
    * Clear data objects used throughout the whole computation process.
    */
   void clear() {
      for (auto & marking:markings) {
         marking.exits.assign(marking.exits.size(), 0);
         marking.current_prob.assign(marking.current_prob.size(), 0.0);
         marking.next_prob.assign(marking.next_prob.size(), 0.0);
      }
      results.assign(ParamsetHelper::getSetSize(), 0.0);
   }

   /**
    * For each state compute how many exists are under each parametrization.
    */
   void computeExits() {
      Paramset current_mask = ParamsetHelper::getLeftOne();
      // Cycle through parameters
      for (size_t param_num:range(ParamsetHelper::getSetSize())) {
         // If not acceptable, leave zero
         if (current_mask & storage.getAcceptable()) {
            for (StateID ID = 0; ID < product.getStateCount(); ID++) {
               auto succs = ColoringFunc::broadcastParameters(round_range, product, ID, current_mask);

               StateID max_BA = 0;
               for (Coloring succ:succs) {
                  max_BA = max(max_BA, product.getBAID(succ.first));
               }
               size_t exits = 0;
               for (Coloring succ:succs) {
                  exits += static_cast<size_t>(max_BA == product.getBAID(succ.first));
               }
               markings[ID].exits[param_num] = exits;
            }
         }

         current_mask >>= 1;
      }
   }

   /**
    * Set probability of each initial state to 1.0 / number of used initial states for this parametrization.
    */
   void initiate() {
      // Cycle through vectors of initial states for every parametrization
      const vector<StateID> & initials = product.getInitialStates();

      // Cycle through the states and assign them the weighted probability
      for (size_t param_num = 0; param_num < ParamsetHelper::getSetSize(); param_num++) {
         for (StateID init:initials) {
            markings[init].next_prob[param_num] = 1.0 / initials.size();
         }
      }
   }

   /**
    * Compute the resulting values as a sum of probabilites of reaching any state.
    */
   void finish() {
      for (StateID ID:product.getFinalStates()) {
         for (size_t param_num = 0; param_num < results.size(); param_num++) {
            results[param_num] += markings[ID].next_prob[param_num];
         }
      }
   }

public:
   /**
    * Constructor ensures that data objects used within the whole computation process have appropriate size.
    */
   RobustnessCompute(const ProductStructure & _product, const ColorStorage & _storage,  const WitnessSearcher & _searcher)
      : product(_product), storage(_storage), searcher(_searcher) {
      Marking empty = { vector<unsigned char>(ParamsetHelper::getSetSize(), 0),
                        vector<double>(ParamsetHelper::getSetSize(), 0.0),
                        vector<double>(ParamsetHelper::getSetSize(), 0.0) };
      markings.resize(product.getStateCount(), empty);
      results.resize(ParamsetHelper::getSetSize(), 0.0);
   }

   /**
    * Function that computes robustness values for each parametrization.
    */
   void compute(const Range & _round_range) {
      round_range = _round_range;
      clear();
      computeExits();
      initiate();
      auto transitions = searcher.getTransitions();

      // Cycle through the levels of the DFS procedure
      for (size_t round_num = 0; round_num < storage.getMaxDepth(); round_num++) {
         // Update markings from the previous round
         for (auto & marking:markings) {
            marking.current_prob = marking.next_prob;
            marking.next_prob.assign(marking.next_prob.size(), 0.0);
         }
         // Assign probabilites for the initial states
         initiate();
         // Cycle through parametrizations
         for (size_t param_num = 0; param_num != ParamsetHelper::getSetSize(); param_num++) {
            // For the parametrization cycle through transitions
            for (const auto & trans:transitions[param_num]) {
               size_t divisor = markings[trans.first].exits[param_num]; // Count succesor
               // Add probabilities
               if (divisor)
                  markings[trans.second].next_prob[param_num] += markings[trans.first].current_prob[param_num] / divisor ;
            }
         }
      }

      finish();
   }

   /**
    * Reformes the Robustness values computed to strings. Nothing is produced for parametrizations with 0 robustness.
    *
    * @return  a vector of robustness strings
    */
   const vector<string> getOutput() const {
      vector<string> to_return;
      for (const double & robust:results){
         if (robust) // Add if the value is non-zero
            to_return.push_back(toString(robust));
      }

      return to_return;
   }
};

#endif // PARSYBONE_ROBUSTNESS_COMPUTE_INCLUDE
