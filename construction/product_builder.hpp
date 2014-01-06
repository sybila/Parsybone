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
   /**
    * Given this source BA state, find out all the target BA that are reachable under this KS state.
    * @param KS_ID	source KS state
    * @param BA_ID	source BA state
    * @return	vector of all the reachable BA states from give product state
    */
   vector<StateID> getReachableBA(const StateID KS_ID, const StateID BA_ID, const ProductStructure & product, const bool allow_stable, const bool allow_transient) const {
      // Vector to store them
      vector<StateID> reachable;
      // Cycle through all the transitions
      for (size_t trans_no = 0; trans_no < product.getAutomaton().getTransitionCount(BA_ID); trans_no++) {
         // Check the transitibility
         if (product.getAutomaton().isTransitionFeasible(BA_ID, trans_no, product.getStructure().getStateLevels(KS_ID))
             && (!product.getAutomaton().isStableRequired(BA_ID, trans_no) || allow_stable)
             && (!product.getAutomaton().isTransientRequired(BA_ID, trans_no) || allow_transient))
            reachable.push_back(product.getAutomaton().getTargetID(BA_ID, trans_no));
      }
      return reachable;
   }

   /**
    * Create state of the product as a combination of a single BA and a single UKS state
    * @param KS_ID	source in the KS
    * @param BA_ID	source in the BA
    * @param transition_count	value which counts the transition for the whole product, will be filled
    */
   void createProductState(const StateID KS_ID, const StateID BA_ID, ProductStructure & product) const {
      // Get all possible BA targets
      vector<StateID> BA_loops = getReachableBA(KS_ID, BA_ID, product, true, false);
      vector<StateID> BA_trans = getReachableBA(KS_ID, BA_ID, product, false, true);
      vector<StateID> loops;
      for(const StateID BA_target : BA_loops)
         loops.push_back(product.getProductID(KS_ID, BA_target));
      product.addState(KS_ID, BA_ID, loops);

      // Add all the transitions possible
      const StateID ID = product.getProductID(KS_ID, BA_ID);
      for (size_t KS_trans = 0; KS_trans < product.getStructure().getTransitionCount(KS_ID); KS_trans++) {
         // Get transition data
         const StateID KS_target_ID = product.getStructure().getTargetID(KS_ID, KS_trans);
         const TransConst & trans_const = product.getStructure().getTransitionConst(KS_ID, KS_trans);

         // Add transition for all allowed targets
         for (const StateID BA_traget : BA_trans) {
            // Compute target position
            const StateID target = product.getProductID(KS_target_ID, BA_traget);
            // Store the transition
            product.addTransition(ID, target, trans_const);
         }
      }
   }

public:
   /**
    * Create the the synchronous product of the provided BA and UKS.
    */
   ProductStructure buildProduct(UnparametrizedStructure  _structure, AutomatonStructure  _automaton) const {
      ProductStructure product(move(_structure), move(_automaton));
      const size_t state_count = product.getStructure().getStateCount() * product.getAutomaton().getStateCount();
      size_t state_no = 0;

      // Creates states and their transitions
      for (size_t KS_ID = 0; KS_ID < product.getStructure().getStateCount(); KS_ID++) {
         for (size_t BA_ID = 0; BA_ID < product.getAutomaton().getStateCount(); BA_ID++) {
            output_streamer.output(verbose_str, "Building product state: " + to_string(++state_no) + "/" + to_string(state_count) + ".", OutputStreamer::no_newl | OutputStreamer::rewrite_ln);
            createProductState(KS_ID, BA_ID,  product);
         }
      }
      output_streamer.clear_line(verbose_str);

      // output_streamer.output(verbose_str, "Product automaton has " + toString(structure.getStateCount() * automaton.getStateCount()) + " states with " + toString(transition_count) + " transitions.");
      return product;
   }
};

#endif // PARSYBONE_PRODUCT_BUILDER_INCLUDED
