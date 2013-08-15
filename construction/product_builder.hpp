/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_PRODUCT_BUILDER_INCLUDED
#define PARSYBONE_PRODUCT_BUILDER_INCLUDED

#include "product_structure.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Creates a final ProductStructure that is used as a template for the synthesis procedure, as a product of ParametrizedStructure and AutomatonStructure.
///
/// ProductBuilder creates the an automaton corresponding to the synchronous product of BA and KS.
/// @attention States of product are indexed as (BA_state_count * KS_state_ID + BA_state_ID) - e.g. if 3-state BA state ((1,0)x(1)) would be at position 3*1 + 1 = 4.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ProductBuilder {
   const UnparametrizedStructure & structure;
   const AutomatonStructure & automaton;

   /**
    * Given this source BA state, find out all the target BA that are reachable under this KS state.
    *
    * @param KS_ID	source KS state
    * @param BA_ID	source BA state
    *
    * @return	vector of all the reachable BA states from give product state
    */
   vector<StateID> getReachableBA(const StateID KS_ID, const StateID BA_ID) const {
      // Vector to store them
      vector<StateID> reachable;
      // Cycle through all the transitions
      for (size_t trans_num = 0; trans_num < automaton.getTransitionCount(BA_ID); trans_num++) {
         // Check the transitibility
         if (automaton.isTransitionFeasible(BA_ID, trans_num, structure.getStateLevels(KS_ID)))
            reachable.push_back(automaton.getTargetID(BA_ID, trans_num));
      }
      return reachable;
   }

   /**
    * Create set with indexes of final and initial states of the product.
    */
   void markStates(ProductStructure & product) const {
      for (size_t ba_state_num = 0; ba_state_num < automaton.getStateCount(); ba_state_num++) {
         // Insert the state if it is an initial state
         if (ba_state_num == 0) {
            for (size_t ks_state_num = 0; ks_state_num < structure.getStateCount(); ks_state_num++) {
               product.initial_states.push_back(product.getProductID(ks_state_num, ba_state_num));
            }
         }
         // Insert the state if it is a final state
         if (automaton.isFinal(ba_state_num)) {
            for (size_t ks_state_num = 0; ks_state_num < structure.getStateCount(); ks_state_num++)  {
               product.final_states.push_back(product.getProductID(ks_state_num, ba_state_num));
            }
         }
      }
   }

   /**
    * Create state of the product as a combination of a single BA and a single UKS state
    * @param KS_ID	source in the KS
    * @param BA_ID	source in the BA
    * @param transition_count	value which counts the transition for the whole product, will be filled
    */
   void createProductState(const StateID KS_ID, const StateID BA_ID, size_t & transition_count, ProductStructure & product) const {
      // Add this state
      product.addState(KS_ID, BA_ID, structure, automaton);
      const StateID ID = product.getProductID(KS_ID, BA_ID);

      // Get all possible BA targets
      auto BA_targets = getReachableBA(KS_ID, BA_ID);

      // Add all the transitions possible
      for (size_t KS_trans = 0; KS_trans < structure.getTransitionCount(KS_ID); KS_trans++) {
         // Get transition data
         const StateID KS_target_ID = structure.getTargetID(KS_ID, KS_trans);
         const ParTransitionion & trans = structure.getTransition(KS_ID, KS_trans);

         // Add transition for all allowed targets
         for (auto BA_traget_it = BA_targets.begin(); BA_traget_it != BA_targets.end(); BA_traget_it++) {
            // Compute target position
            const StateID target = product.getProductID(KS_target_ID, *BA_traget_it);
            // Store the transition
            product.addTransition(ID, target, trans);
            transition_count++;
         }
      }
   }


public:
   ProductBuilder(const UnparametrizedStructure & _structure, const AutomatonStructure & _automaton) : structure(_structure), automaton(_automaton) {}

   /**
    * Create the the synchronous product of the provided BA and UKS.
    */
   ProductStructure buildProduct() const {
      ProductStructure product(automaton.getStateCount(), structure.getStateCount());
      size_t transition_count = 0;
      const size_t state_count = structure.getStateCount() * automaton.getStateCount();
      size_t state_no = 0;

      // Creates states and their transitions
      for (size_t KS_ID = 0; KS_ID < structure.getStateCount(); KS_ID++) {
         for (size_t BA_ID = 0; BA_ID < automaton.getStateCount(); BA_ID++) {
            output_streamer.output(verbose_str, "Building product state: " + toString(++state_no) + "/" + toString(state_count) + ".", OutputStreamer::no_newl | OutputStreamer::rewrite_ln);
            createProductState(KS_ID, BA_ID, transition_count, product);
         }
      }
      output_streamer.clear_line(verbose_str);

      // Create final and intial states vectors
      markStates( product);

      // output_streamer.output(verbose_str, "Product automaton has " + toString(structure.getStateCount() * automaton.getStateCount()) + " states with " + toString(transition_count) + " transitions.");
      return product;
   }
};

#endif // PARSYBONE_PRODUCT_BUILDER_INCLUDED
