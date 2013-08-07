/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_PRODUCT_INCLUDED
#define PARSYBONE_PRODUCT_INCLUDED

#include "../auxiliary/output_streamer.hpp"
#include "../construction/automaton_structure.hpp"
#include "../construction/unparametrized_structure.hpp"

/// Storing a single transition to a neighbour state together with its transition function.
struct ProdTransitiontion : public TransitionProperty {
   ParamNum step_size; ///< How many bits of a parameter space bitset is needed to get from one targe value to another.
   vector<bool> transitive_values; ///< Which values from the original set does not allow a trasition and therefore removes bits from the mask.

   ProdTransitiontion(const StateID target_ID, const ParamNum _step_size, const vector<bool>& _transitive_values)
      : TransitionProperty(target_ID), step_size(_step_size), transitive_values(_transitive_values) {} ///< Simple filler, assigns values to all the variables.
};

/// State of the product - same as the state of UKS but put together with a BA state.
struct ProdState : public AutomatonStateProperty<ProdTransitiontion> {
   StateID KS_ID; ///< ID of an original KS state this one is built from
   StateID BA_ID; ///< ID of an original BA state this one is built from
   Levels species_level; ///< species_level[i] = activation level of specie i in this state

   /// Simple filler, assigns values to all the variables
   ProdState(const StateID ID, const string && label, const StateID _KS_ID, const StateID _BA_ID, const bool initial, const bool final, const  Levels & _species_level)
      : AutomatonStateProperty<ProdTransitiontion>(initial, final, ID, move(label)), KS_ID(_KS_ID), BA_ID(_BA_ID), species_level(_species_level) {}
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
   size_t aut_state_count;
   size_t struct_state_count;

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // FILLING METHODS (can be used only from ProductBuilder)
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   /**
    * Add a new state, only with ID and levels.
    */
   inline void addState(const StateID KS_ID, const StateID BA_ID, const UnparametrizedStructure & structure, const AutomatonStructure & automaton) {
      // Create the state label
      string label = structure.getString(KS_ID);
      label[label.length() - 1] = ';';
      label += automaton.getString(BA_ID).substr(1);

      states.push_back(ProdState(getProductID(KS_ID, BA_ID), move(label), KS_ID, BA_ID, automaton.isInitial(BA_ID), automaton.isFinal(BA_ID), structure.getStateLevels(KS_ID)));
   }

   /**
    * Add a new transition with all its values.
    * @param ID	add data to the state with this IS
    */
   inline void addTransition(const StateID ID, const StateID target_ID, const size_t step_size, const vector<bool> & transitive_values) {
      states[ID].transitions.push_back(ProdTransitiontion(target_ID, step_size, transitive_values));
   }

public:
   ProductStructure() : aut_state_count(0), struct_state_count(0) {}
   ProductStructure(size_t _aut_state_count, size_t _struct_state_count) : aut_state_count(_aut_state_count), struct_state_count(_struct_state_count) {}

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // REFORMING GETTERS
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   /**
    * @return index of this combination of states in the product
    */
   inline StateID getProductID(const StateID KS_ID, const StateID BA_ID) const {
      return (KS_ID * aut_state_count + BA_ID);
   }

   /**
    * @return index of BA state form the product
    */
   inline StateID getBAID(const StateID ID) const {
      return states[ID].BA_ID;
   }

   /**
    * @return index of BA state form the product
    */
   inline StateID getKSID(const StateID ID) const {
      return states[ID].KS_ID;
   }

   /**
    * @param ID	ID of the state to get the data from
    * @return	species level
    */
   inline const Levels & getStateLevels(const StateID ID) const {
      return states[ID].species_level;
   }

   /**
    * @param ID	ID of the state to get the data from
    * @param transition_num index of the transition to get the data from
    * @return	number of neighbour parameters that share the same value of the function
    */
   inline size_t getStepSize(const StateID ID, const size_t transtion_num) const {
      return states[ID].transitions[transtion_num].step_size;
   }

   /**
    * @param ID	ID of the state to get the data from
    * @param transition_num index of the transition to get the data from
    * @return	target values that are includete in non-transitive parameters that have to be removed
    */
   inline const vector<bool> & getTransitive(const StateID ID, const size_t transtion_num) const {
      return states[ID].transitions[transtion_num].transitive_values;
   }
};

#endif // PARSYBONE_PRODUCT_INCLUDED
