/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_ROBUSTNESS_COMPUTE_INCLUDE
#define PARSYBONE_ROBUSTNESS_COMPUTE_INCLUDE

#include "witness_searcher.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Class responsible for computation of robustness values for each acceptable parametrization.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class RobustnessCompute {
   const ProductStructure & product; ///< Product reference for state properties.
   const ColorStorage & storage; ///< Constant storage with the actuall data.
   const WitnessSearcher & searcher; ///< Reference to the searcher that contains transitions.

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
      results.assign(paramset_helper.getSetSize(), 0.0);
   }

   /**
    * For each state compute how many exists are under each parametrization.
    */
   void computeExits() {
      Paramset current_mask = paramset_helper.getLeftOne();
      // Cycle through parameters
      for (size_t param_num:range(paramset_helper.getSetSize())) {
         // If not acceptable, leave zero
         if (current_mask & storage.getAcceptable()) {
            for (StateID ID:range(product.getStateCount())) {
               auto succs = storage.getNeighbours(ID, true, current_mask);
               StateID max_BA = 0;
               for (auto succ:succs) {
                  max_BA = my_max(max_BA, product.getBAID(succ));
               }
               size_t exits = 0;
               for (auto succ:succs) {
                  exits += static_cast<size_t>(max_BA == product.getBAID(succ));
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
      auto initials = searcher.getInitials();
      size_t param_num = 0;
      for (auto init_it = initials.begin(); init_it != initials.end(); init_it++, param_num++) {
         // Cycle through the states for this parametrization and assign them the weighted probability
         for (auto node_it = init_it->begin(); node_it != init_it->end(); node_it++) {
            markings[*node_it].next_prob[param_num] = 1.0 / init_it->size();
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

   RobustnessCompute(const RobustnessCompute & other); ///< Forbidden copy constructor.
   RobustnessCompute& operator=(const RobustnessCompute & other); ///< Forbidden assignment operator.

public:
   /**
    * Constructor ensures that data objects used within the whole computation process have appropriate size.
    */
   RobustnessCompute(const ConstructionHolder & _holder, const ColorStorage & _storage,  const WitnessSearcher & _searcher)
      : product(_holder.getProduct()), storage(_storage), searcher(_searcher) {
      Marking empty = { vector<unsigned char>(paramset_helper.getSetSize(), 0),
         vector<double>(paramset_helper.getSetSize(), 0.0),
         vector<double>(paramset_helper.getSetSize(), 0.0)
      };
      markings.resize(product.getStateCount(), empty);
      results.resize(paramset_helper.getSetSize(), 0.0);
   }

   /**
    * Function that computes robustness values for each parametrization.
    */
   void compute() {
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
         for (size_t param_num = 0; param_num != paramset_helper.getSetSize(); param_num++) {
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
