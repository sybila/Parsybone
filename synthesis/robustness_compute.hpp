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
#include <float.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Class responsible for computation of robustness values for each acceptable parametrization.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class RobustnessCompute {
   const ProductStructure & product; ///< Product reference for state properties
   const ColorStorage & storage; ///< Constant storage with the actuall data
   const WitnessSearcher & searcher; ///< Reference to the searcher that contains transitions

   /// This structure holds values used in the iterative process of robustness computation
   struct Marking {
      std::vector<unsigned char> exits; ///< For each parametrization stores a number of transitions this state can be left through under given parametrization.
      std::vector<double> current_prob; ///< For each parametrization stores current probability of reaching
      std::vector<double> next_prob; ///< For each parametrizations will store the probability in the next round
   };

   std::vector<Marking> markings; /// Marking of all states
   std::vector<double> results; /// Resultig probability for each parametrization

   /**
    * Clear data objects used throughout the whole computation process.
    */
   void clear() {
      forEach(markings, [](Marking & marking){
         marking.exits.assign(marking.exits.size(), 0);
         marking.current_prob.assign(marking.current_prob.size(), 0.0);
         marking.next_prob.assign(marking.next_prob.size(), 0.0);
      });
      results.assign(paramset_helper.getParamsetSize(), 0.0);
   }

   /**
    * For each state compute how many exists are under each parametrization
    */
   void computeExits() {
      Paramset current_mask = paramset_helper.getLeftOne();
      // Cycle through parameters
      for (std::size_t param_num = 0; param_num < paramset_helper.getParamsetSize(); param_num++, current_mask >>= 1) {
         // If not acceptable, leave zero
         if (!(current_mask & storage.getAcceptable()))
            continue;
         // Otherwise, set number to the number of exist under this parametrization
         for (StateID ID = 0; ID < product.getStateCount(); ID++) {
            markings[ID].exits[param_num] = storage.getNeighbours(ID, true, current_mask).size();
         }
      }
   }

   /**
    * Set probability of each initial state to 1.0 / number of initial states for this parametrization
    */
   void initiate() {
      // Cycle through vectors of initial states for every parametrization
      auto initials = searcher.getInitials();
      std::size_t param_num = 0;
      for (auto init_it = initials.begin(); init_it != initials.end(); init_it++, param_num++) {
         // Cycle through the states for this parametrization and assign them the weighted probability
         for (auto node_it = init_it->begin(); node_it != init_it->end(); node_it++) {
            markings[*node_it].next_prob[param_num] = 1.0 / init_it->size();
         }
      }
   }

   /**
    * Compute the resulting values as a sum of probabilites of reaching any state
    */
   void finish() {
      forEach(product.getFinalStates(), [&](StateID ID) {
         for (std::size_t param_num = 0; param_num < results.size(); param_num++) {
            results[param_num] += markings[ID].next_prob[param_num];
         }
      });
   }

   RobustnessCompute(const RobustnessCompute & other); ///< Forbidden copy constructor.
   RobustnessCompute& operator=(const RobustnessCompute & other); ///< Forbidden assignment operator.

public:
   /**
    * Constructor ensures that data objects used within the whole computation process have appropriate size
    */
   RobustnessCompute(const ConstructionHolder & _holder, const ColorStorage & _storage,  const WitnessSearcher & _searcher)
      : product(_holder.getProduct()), storage(_storage), searcher(_searcher) {
      Marking empty = { std::vector<unsigned char>(paramset_helper.getParamsetSize(), 0),
         std::vector<double>(paramset_helper.getParamsetSize(), 0.0),
         std::vector<double>(paramset_helper.getParamsetSize(), 0.0)
      };
      markings.resize(product.getStateCount(), empty);
      results.resize(paramset_helper.getParamsetSize(), 0.0);
   }

   /**
    * Function that computes robustness values for each parametrization
    */
   void compute() {
      clear();
      computeExits();
      initiate();
      auto transitions = searcher.getTransitions();

      // Cycle through the levels of the DFS procedure
      for (std::size_t round_num = 0; round_num < storage.getMaxDepth() + 1; round_num++) {
         // Update markings from the previous round
         forEach(markings, [](Marking & marking) {
            marking.current_prob = marking.next_prob;
            marking.next_prob.assign(marking.next_prob.size(), 0.0);
         });
         // Assign probabilites for the initial states
         initiate();
         // Cycle through parametrizations
         for (std::size_t param_num = 0; param_num != paramset_helper.getParamsetSize(); param_num++) {
            // For the parametrization cycle through transitions
            forEach(transitions[param_num], [&](std::pair<StateID, StateID> trans) {
               // Well, this takes a bit of imagination to grasp. Point is, that the interesting node in not the final one, but the one before it
               // so it is necessary not to lower the Robustness any further when in pre-final node.
               std::size_t divisor = product.isFinal(trans.second) ? 1 : markings[trans.first].exits[param_num]; // Count succesor
               // Add probabilities
               if (divisor)
                  markings[trans.second].next_prob[param_num] += markings[trans.first].current_prob[param_num] / divisor ;
            });
         }
      }

      finish();
   }

   /**
    * Reformates the Robustness computed to strings. Nothing is produced for parametrizations with 0 robustness.
    *
    * @return  a vector of robustness strings
    */
   const std::vector<std::string> getOutput() const {
      std::vector<std::string> to_return;
      forEach(results, [&to_return](double robust){
         std::string string_val("<");
         if (robust) // Add if the value is non-zero
            to_return.push_back(string_val.append(toString(robust)).append(">"));
      });

      return to_return;
   }
};

#endif // PARSYBONE_ROBUSTNESS_COMPUTE_INCLUDE
