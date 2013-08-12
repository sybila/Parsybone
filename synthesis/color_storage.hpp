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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief An auxiliary class to the ProductStructure and stores colors and possibly predecessors for individual states of the product during the computation.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ColorStorage {
	struct State {
      Paramset parameters; ///< Bits for each color in this round marking its presence or absence in the state.

      /// Holder of the computed information for a single state.
      State() : parameters(0) { }
	};
	
	vector<State> states; ///< Vector of states that correspond to those of Product Structure and store coloring data.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREATION METHODS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/**
    * Constructor allocates necessary memory for further usage (this memory is not supposed to be freed until endo of the computation).
	 * Every state has predecessors and succesors allocated for EVERY other state, this consumes memory but benefits the complexity of operations.
	 * @param states_count	number of states the structure the data will be saved for has
	 */
   ColorStorage(const ProductStructure & product) {
      // Create states
      const size_t STATE_COUNT = product.getStateCount();

      for (StateID ID = 0; ID < product.getStateCount(); ID++) {
         output_streamer.output(verbose_str, "Building storage state: " + toString(ID) + "/" + toString(STATE_COUNT) + ".", OutputStreamer::no_newl | OutputStreamer::rewrite_ln);
         states.push_back(State());
		}
      output_streamer.clear_line();
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
      for (State & state:states)
			// Reset merged parameters
			state.parameters = 0;
	}

	/**
    * Add passed colors to the state.
	 * @param ID	index of the state to fill
	 * @param parameters to add - if empty, add all, otherwise use bitwise or
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
	 * @param ID	index of the state to ask for parameters
    * @return  parameters assigned to the state
	 */
	inline const Paramset & getColor(const StateID ID) const {
		return states[ID].parameters;
	}

	/**
	 * @param states	indexes of states to ask for parameters
    * @return  queue with all colorings of states
	 */
	const vector<Coloring> getColor(const vector<StateID> & states) const {
		// Queue tates colored in basic coloring
		vector<Coloring> colors;

		// Get the states and their colors
      for (const StateID ID:states)
			colors.push_back(Coloring(ID, getColor(ID)));

		// Return final vertices with their positions
		return colors;
	}
};

#endif // PARSYBONE_COLOR_STORAGE_INCLUDED
