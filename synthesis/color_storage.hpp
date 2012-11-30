/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_COLOR_STORAGE_INCLUDED
#define PARSYBONE_COLOR_STORAGE_INCLUDED

#include "../auxiliary/data_types.hpp"
#include "../auxiliary/common_functions.hpp"
#include "../construction/construction_holder.hpp"
#include "paramset_helper.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief An auxiliary class to the ProductStructure and stores colors and possibly predecessors for individual states of the product during the computation.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ColorStorage {
	struct State {
      StateID ID; ///< Unique ID of the state.
      Paramset parameters; ///< Bits for each color in this round marking its presence or absence in the state.
      map<StateID, Paramset> predecessors; ///< Stores a predeccesor in the form (product_ID, parameters).
      map<StateID, Paramset> successors; ///< Stores succesors in the form (product_ID, parameters).

      /// Holder of the computed information for a single state.
      State(const StateID _ID) : ID(_ID), parameters(0) { }
	};
	
	vector<State> states; ///< Vector of states that correspond to those of Product Structure and store coloring data.
	/// This vector stores so-called COST value i.e. number of steps required to reach the final state in TS.
	/// If it is not reachable, cost is set to INF.
	vector<size_t> cost_val;
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

      // Add predecessors and succesors paramset storage, if necessary
      if (user_options.analysis()) {
         for (StateID ID = 0; ID < product.getStateCount(); ID++) {
            for (size_t trans_num = 0; trans_num < product.getTransitionCount(ID); trans_num++) {
               StateID target = product.getTargetID(ID, trans_num);
               states[ID].successors.insert(make_pair(target, 0));
               states[target].predecessors.insert(make_pair(ID, 0));
            }
         }
      }

      // Set additional storage
      cost_val = vector<size_t>(paramset_helper.getSetSize(), INF); // Set all to max. value
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

         if (user_options.analysis()) {
            // Copy from predecessors
            auto m_preds_it = m_state_it->predecessors.begin();
            auto o_preds_it = o_state_it->predecessors.begin();
            while (m_preds_it != m_state_it->predecessors.end()) {
               m_preds_it->second |= o_preds_it->second;
               m_preds_it++; o_preds_it++;
            }
            // Copy from successors
            auto m_succs_it = m_state_it->successors.begin();
            auto o_succs_it = o_state_it->successors.begin();
            while (m_succs_it != m_state_it->successors.end()) {
               m_succs_it->second |= o_succs_it->second;
               m_succs_it++; o_succs_it++;
            }
         }

         m_state_it++; o_state_it++;
      }
   }

	/**
	 * Sets all values for all the states to zero. Allocated memory remains.
	 */ 
	void reset() {
		// Clear each state
		for (auto & state:states) {
			// Reset merged parameters
			state.parameters = 0;
			// Reset parameters from predecessors, if there were new values
         if (user_options.analysis()) {
            for(auto & pred:state.predecessors) {
               pred.second = 0;
				}
				for(auto & succ:state.successors) {
					succ.second = 0;
				}
			}
		}
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
    * Add passed colors to the state.
	 *
	 * @param source_ID	index of the state that passed this update
	 * @param target_ID	index of the state to fill
	 * @param parameters to add - if empty, add all, otherwise use bitwise or
	 * 
    * @return  true if there was an actuall update
	 */
	inline bool update(const StateID source_ID, const StateID target_ID, const Paramset parameters) {
		// Mark parameters source and target
      states[target_ID].predecessors.find(source_ID)->second |= parameters;
      states[source_ID].successors.find(target_ID)->second |= parameters;
		// Make an actuall update
		return update(target_ID, parameters);
	}

	/**
    * Removes given paramset from the coloring of the given state.
	 */
	void remove(const StateID ID, const Paramset remove) {
		states[ID].parameters &= ~remove;
	}

	/**
    * Removes given paramset from the label of transitions to successors / from predecessors for a given state.
	 *
    * @param successors if true, use successors, predecessors otherwise
	 */
	void remove(const StateID source_ID, const Paramset remove, const bool successors) {
		// reference
		auto neigbours = successors ? states[source_ID].successors : states[source_ID].predecessors;
		for(auto & neighbour:neigbours)
			neighbour.second &= ~remove;
	}

	/**
    * Removes given paramset from the label of transitions to a single successor / from a single predecessor for a given state.
	 *
	 * @param target_ID	ID of the state target state to remove parameset from labelling
	 * @param successors	if true, use successors, predecessors otherwise
	 */
	void remove(const StateID source_ID, const StateID target_ID, const Paramset remove, const bool successors) {
		// reference
		auto neigbours = successors ? states[source_ID].successors : states[source_ID].predecessors;
      neigbours.find(target_ID)->second &=~ remove;
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
              depth = my_max((val == INF) ? 0 : val, depth);

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
	 * Get all the neigbours for this color from this state.
	 *
	 * @param ID	index of the state to ask for predecessors
	 * @param successors	true if successors are required, false if predecessors
	 * @param color_mask	if specified, restricts neighbour to only those that contain a subset of the parametrizations
	 *
    * @return  neigbours for given state
	 */
	inline const Neighbours getNeighbours(const StateID ID, const bool successors, const Paramset color_mask = INF) const {
		// reference
		auto neigbours = successors ? states[ID].successors : states[ID].predecessors;
		// Data to fill
		Neighbours color_neigh;

		// Add these from the color
		for(auto & neighbour:neigbours) {
			// Test if the color is present
         if ((neighbour.second & color_mask) != 0)
            color_neigh.push_back(neighbour.first);
      }

		return color_neigh;
	}

	/** 
    * Get all the labels on trasintions from given neighbours.
	 *
	 * @param ID	index of the state to ask for predecessors
    * @param successors true if successors are required, false if predecessors
	 * @param color_mask	if specified, restricts neighbour to only those that contain a subset of the parametrizations
	 *
    * @return  labelling on the neighbour labels
	 */
	inline const vector<Paramset> getMarking(const StateID ID, const bool successors, const Paramset color_mask = INF) const {
		// reference
		auto neigbours = successors ? states[ID].successors : states[ID].predecessors;

		vector<Paramset> restricted;
		// Add only those that contain the value
		for(auto & neighbour:neigbours) {
			// Test if the color is present
         if ((neighbour.second & color_mask) != 0)
            restricted.push_back(neighbour.second);
      }

		return restricted;
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
