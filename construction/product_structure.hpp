/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

// TODO: Include the source structures and just refer to their content.

#ifndef PARSYBONE_PRODUCT_INCLUDED
#define PARSYBONE_PRODUCT_INCLUDED

#include "../auxiliary/output_streamer.hpp"
#include "../construction/automaton_structure.hpp"
#include "../construction/unparametrized_structure.hpp"

/// Storing a single transition to neighbour state together with its transition function.
struct ProdTransitionion : public TransitionProperty {
   const TransConst & trans_const;

   ProdTransitionion(const StateID _target_ID, const TransConst & _trans_const)
      : TransitionProperty(_target_ID), trans_const(_trans_const) {}
};


/// State of the product - same as the state of UKS but put together with a BA state.
struct ProdState : public AutomatonStateProperty<ProdTransitionion> {
   const StateID KS_ID; ///< ID of an original KS state this one is built from
   const StateID BA_ID; ///< ID of an original BA state this one is built from
   const Levels & levels; ///< species_level[i] = activation level of specie i in this state

   /// Simple filler, assigns values to all the variables
   ProdState(const StateID ID, const StateID _KS_ID, const StateID _BA_ID, const bool initial, const bool final, const Levels & _species_level)
      : AutomatonStateProperty<ProdTransitionion>(initial, final, ID), KS_ID(_KS_ID), BA_ID(_BA_ID), levels(_species_level) {}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Holds a product structure - the one that is used in coloring procedure.
///
/// This is the final step of construction - a structure that is acutally used during the computation. For simplicity, it copies data from its predecessors (BA and UKS).
/// @attention States of product are indexed as (BA_state_count * KS_state_ID + BA_state_ID) - e.g. if 3-state BA state ((1,0)x(1)) would be at position 3*1 + 1 = 4.
///
/// ProductStructure data can be set only from the ProductBuilder object.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ProductStructure : public AutomatonInterface<ProdState> {
   friend class ProductBuilder;
   UnparametrizedStructure structure;
   AutomatonStructure automaton;

   /**
    * Add a new state, only with ID and levels.
    */
   inline void addState(const StateID KS_ID, const StateID BA_ID) {
      // Create the state label
      states.push_back(ProdState(getProductID(KS_ID, BA_ID), KS_ID, BA_ID, automaton.isInitial(BA_ID), automaton.isFinal(BA_ID), structure.getStateLevels(KS_ID)));
   }

   /**
    * Add a new transition with all its values.
    * @param ID	add data to the state with this IS
    */
   inline void addTransition(const StateID ID, const StateID target_ID, const TransConst & constraints) {
      states[ID].transitions.push_back(ProdTransitionion(target_ID, constraints));
   }

public:
   ProductStructure() = default;
   ProductStructure(UnparametrizedStructure _structure, AutomatonStructure _automaton) : structure(move(_structure)), automaton(move(_automaton)) {}

   const inline UnparametrizedStructure & getStructure() const {
      return structure;
   }

   const inline AutomatonStructure & getAutomaton() const {
      return automaton;
   }

   inline StateID getProductID(const StateID KS_ID, const StateID BA_ID) const {
      return (KS_ID * automaton.getStateCount() + BA_ID);
   }

   inline StateID getBAID(const StateID ID) const {
      return states[ID].BA_ID;
   }

   inline StateID getKSID(const StateID ID) const {
      return states[ID].KS_ID;
   }

   inline const Levels & getStateLevels(const StateID ID) const {
      return states[ID].levels;
   }

   inline size_t getStepSize(const StateID ID, const size_t transtion_num) const {
      return states[ID].transitions[transtion_num].trans_const.step_size;
   }

   inline const Levels & getTargets(const StateID ID, const size_t transtion_num) const {
      return states[ID].transitions[transtion_num].trans_const.targets;
   }

   inline Direction getOp(const StateID ID, const size_t transtion_num) const {
      return states[ID].transitions[transtion_num].trans_const.req_op;
   }

   inline ActLevel getVal(const StateID ID, const size_t transtion_num) const {
      return states[ID].transitions[transtion_num].trans_const.req_comp;
   }

   const string getString(const StateID ID) const {
      string label = "(";

      for (const ActLevel lev : states[ID].levels)
         label += toString(lev) + ",";

      label[label.length() - 1] = ';';
      label += toString(getBAID(ID)) + ")";

      return label;
   }
};

#endif // PARSYBONE_PRODUCT_INCLUDED
