/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_COLOR_STORAGE_INCLUDED
#define PARSYBONE_COLOR_STORAGE_INCLUDED

#include "PunyHeaders/common_functions.hpp"

#include "../auxiliary/data_types.hpp"
#include "../construction/construction_holder.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief An auxiliary class to the ProductStructure and stores colors and possibly predecessors for individual states of the product during the computation.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ColorStorage {
	struct State {
      StateID ID; ///< Unique ID of the state.
      Paramset parameters; ///< Bits for each color in this round marking its presence or absence in the state.

      /// Holder of the computed information for a single state.
      State(const StateID _ID) : ID(_ID), parameters(0) { }
	};
	
	vector<State> states; ///< Vector of states that correspond to those of Product Structure and store coloring data.
   vector<size_t> cost_val; ///< This vector stores so-called COST value i.e. number of steps required to reach the final state in TS. If it is not reachable, cost is set to INF.
   Paramset acceptable; ///< Additional value that stores paramset computed in this round.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREATION METHODS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/**
    * Constructor allocates necessary memory for further usage (this memory is not supposed to be freed until endo of the computation).
	 * Every state has predecessors and succesors allocated for EVERY other state, this consumes memory but benefits the complexity of operations.
	 *
	 * @param states_count	number of states the structure the data will be saved for has
	 */
   ColorStorage(const ConstructionHolder & holder) {
      // Create states
      const ProductStructure & product = holder.getProduct();
      for (StateID ID = 0; ID < product.getStateCount(); ID++) {
         states.push_back(ID);
		}

      // Set additional storage
      cost_val = vector<size_t>(ParamsetHelper::getSetSize(), INF); // Set all to max. value
		acceptable = 0;
	}

	ColorStorage() = default; ///< Empty constructor for an empty storage.

   /**
    * Function adds values from specified source without explicitly copying them, only through bitwise or (storages must be equal).
    */
   void addFrom(const ColorStorage & other) {

      auto m_state_it = states.begin();
      auto o_state_it = other.states.begin();

      while (m_state_it != states.end()) {
         // Copy from paramset
         m_state_it->parameters |= o_state_it->parameters;

         m_state_it++; o_state_it++;
      }
   }

	/**
	 * Sets all values for all the states to zero. Allocated memory remains.
	 */ 
	void reset() {
		// Clear each state
      for (auto & state:states)
			// Reset merged parameters
			state.parameters = 0;
	}

	/**
    * Fills after time series check finished.
	 *
	 * @param new_cost	a vector of lenght |parameter_set| containing cost values. If the value does not exist (state is not reachable), use INF
	 */
	void setResults(const vector<size_t> & new_cost, const Paramset resulting) {
		cost_val = new_cost;
		acceptable = resulting;
	}

   /**
    * Fills after a general LTL check finished.
    *
    * @param new_cost	a vector of lenght |parameter_set| containing cost values. If the value does not exist (state is not reachable), use INF
    */
   void setResults(const Paramset resulting) {
      acceptable = resulting;
   }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COLORING HANDLERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
    * Add passed colors to the state.
	 *
	 * @param ID	index of the state to fill
	 * @param parameters to add - if empty, add all, otherwise use bitwise or
	 * 
    * @return  true if there was an actuall update
	 */
	inline bool update(const StateID ID, const Paramset parameters) {
		// If nothing is new return false
		if (states[ID].parameters == (parameters | states[ID].parameters))
			return false;
		// Add new parameters and return true
		states[ID].parameters |= parameters;
		return true;
	}

	/**
	 * Return true if the state would be updated, false otherwise.
	 *
	 * @param ID	index of the state to fill
	 * @param parameters to add - if empty, add all, otherwise use bitwise or
	 *
    * @return  true if there would be an update
	 */
	inline bool soft_update(const StateID ID, const Paramset parameters) {
		if (states[ID].parameters == (parameters | states[ID].parameters))
			return false;
		else
			return true;
	}


	/**
    * Removes given paramset from the coloring of the given state.
	 */
	void remove(const StateID ID, const Paramset remove) {
		states[ID].parameters &= ~remove;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   /**
    * @return  max finite cost among parametrizations used this round
    */
   size_t getMaxDepth () const {
      size_t depth = 0;
      for (const auto val:cost_val)
              depth = max((val == INF) ? 0 : val, depth);

      return depth;
   }

	/**
	 * @param ID	index of the state to ask for parameters
	 * 
    * @return  parameters assigned to the state
	 */
	inline const Paramset & getColor(const StateID ID) const {
		return states[ID].parameters;
	}

	/**
	 * @param states	indexes of states to ask for parameters
	 *
    * @return  queue with all colorings of states
	 */
	const vector<Coloring> getColor(const vector<StateID> & states) const {
		// Queue tates colored in basic coloring
		vector<Coloring> colors;

		// Get the states and their colors
		for (const auto ID:states)
			colors.push_back(Coloring(ID, getColor(ID)));

		// Return final vertices with their positions
		return colors;
	}

	/**
	 * @param number of the parametrization relative in this round
	 *
    * @return  Cost value of a particular parametrization
	 */
	size_t getCost(size_t position) const {
		return cost_val[position];
	}

	/**
    * @return  Cost value of all the parametrizations from this round
	 */
	const vector<size_t> & getCost() const {
		return cost_val;
	}

	/**
    * @return  mask of parametrizations that are computed acceptable in this round
	 */
	const Paramset & getAcceptable() const {
		return acceptable;
	}
};

#endif // PARSYBONE_COLOR_STORAGE_INCLUDED
