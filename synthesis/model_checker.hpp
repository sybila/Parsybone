/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_MODEL_CHECKER_INCLUDED
#define PARSYBONE_MODEL_CHECKER_INCLUDED

#include "PunyHeaders/common_functions.hpp"

#include "../construction/construction_holder.hpp"
#include "color_storage.hpp"
#include "paramset_helper.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Main class of the computation - responsible for the CMC procedure.
///
/// ModelChecker class solves the parameter synthesis problem by iterative transfer of feasible parametrizations from initial states to final ones.
/// Functions in model checker use many supporting variables and therefore are quite long, it would not make sense to split them, though.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ModelChecker {
	// Information
   const ProductStructure & product; ///< Product on which the computation will be conducted.
   Range synthesis_range; ///< First and one beyond last color to be computed in this round.
   StateID starting_state; ///< State from which the synthesis goes during the general LTL search.

	// Coloring storage
   ColorStorage & storage; ///< Class that actually stores colors during the computation.
   ColorStorage next_round_storage; ///< Class that stores updated colors for next round (prevents multiple transitions through one BFS round).
   set<StateID> updates; ///< Set of states that need to spread their updates.
   set<StateID> next_updates; ///< Updates that are sheduled forn the next round.

	// BFS boundaries
   Paramset starting; ///< Mask of parameters provided for this round.
   Paramset to_find; ///< Mask of parameters that are still not found.
   Paramset restrict_mask; ///< Mask of parameters that are secure to left out.
   vector<size_t> BFS_reach; ///< In which round this color was found.
   size_t BFS_level; ///< Number of current BFS level during coloring, starts from 0, meaning 0 transitions.

	// Other
   vector<bool> BA_presence; ///< Vector that marks IDs of BA states under which self-loop is possible this round, used to update only with "sink" parametrizations.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTING METHODS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	/**
	 * Main function of coloring - creates intersection of passing and transition colors to create and update color.
	 *
	 * @param passed	parameters that are passed through the transition
	 * @param step_size	how many parameters share the same value for given function
	 * @param transitive_values	mask of all values from which those that have false are non-transitive
	 */
   void passParameters(Paramset & passed, const size_t step_size, const vector<bool> & transitive_values) const {
      // INITIALIZATION OF VALUES FOR POSITIONING
      // Number of the first parameter
      ParamNum param_num = synthesis_range.first;
      // First value might not bet 0 - get it from current parameter position
      size_t value_num = (param_num / step_size) % transitive_values.size();
      // Mask that will be created
      register Paramset temporary = 0;

      // COMPUTATION OF MASK
      // List through all the paramters
      while (true) {
         // List through ALL the target values
         for (; value_num < transitive_values.size(); value_num++) {
            // Get size of the step for current value
            size_t bits_in_step = min<size_t>((step_size - (param_num % step_size)), static_cast<size_t>(synthesis_range.second - param_num));
            // Move the mask so new value data can be add
            temporary <<= bits_in_step;
            // If transitive, add ones for the width of the step
            if (transitive_values[value_num]) {
               Paramset add = INF;
               add >>= (paramset_helper.getSetSize() - bits_in_step);
               temporary |= add;
            }
            // If we went throught the whole size, end
            if ((param_num += bits_in_step) == synthesis_range.second) {
               // Create interection of source parameters and transition parameters
               passed &= temporary;
               return;
            }
         }
         // Reset the value
         value_num = 0;
      }
   }

	/**
    * From all the updates pick the one from the state with most bits.
	 *
	 * @return	index of the state to start an update from
	 */
	StateID getStrongestUpdate() const {
		// Reference value
		register StateID ID = 0;
		register Paramset current = 0;
		// Cycle throught the updates
		for (auto update_it = updates.begin(); update_it != updates.end(); update_it++) {
			Paramset test = storage.getColor(*update_it);
			// Compare with current data - if better, replace
			if (test != current) {
				if (test == (current | test)) {
					ID = *update_it;
					current = test;
				}
			}
		}
		return ID;
	}

	/**
	 * For each found color add this BFS level as a first when the state was updated, if it was not already found.
    *
	 * @param colors	current coloring
	 */
	void markLevels(const Paramset colors) {
		// If all is found, end
      if (!to_find || !(to_find & colors))
			return;

		// Which are new
		Paramset store = to_find & colors;
		// Remove currently found
		to_find &= ~store;

		// Store those that were found in this round
		for (int color_pos = static_cast<int>(paramset_helper.getSetSize() - 1); color_pos >= 0 ; store >>= 1, color_pos--) {
			if (store % 2)
				BFS_reach[color_pos] = BFS_level;		
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COLORING METHODS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
    * Get stripped parameters for each unique edge (if there are multi-edges, intersect their values).
	 *
	 * @param ID	ID of the source state in the product
	 * @param parameters	parameters that will be distributed
	 *
	 * @return vector of passed parameters together with their targets
	 */
	vector<Coloring> broadcastParameters(const StateID ID, const Paramset parameters) {
		// To store parameters that passed the transition but were not yet added to the target
		vector<Coloring> param_updates;
		param_updates.reserve(product.getTransitionCount(ID));

		size_t KS_state = product.getKSID(ID);
		Paramset loop_params = INF; // Which of the parameters allow only to remain in this state

		// Cycle through all the transition
		for (size_t trans_num = 0; trans_num < product.getTransitionCount(ID); trans_num++) {
         StateID target_ID = product.getTargetID(ID, trans_num);

         // Parameters to pass through the transition
         Paramset passed = parameters;

			// From an update strip all the parameters that can not pass through the transition - color intersection on the transition
			passParameters(passed, product.getStepSize(ID, trans_num), product.getTransitive(ID, trans_num));

			// Test if it is a possibility for a loop, if there is nothing outcoming, add to self-loop (if it is still possible)
         if (KS_state == product.getKSID(target_ID) ) {
            loop_params &= passed;
            if(loop_params) {
               StateID BA_ID = product.getBAID(target_ID);
               BA_presence[BA_ID] = true;
            }
			}
			// Else add normally and remove from the loop
			else if (passed) {
				loop_params &= ~passed; // Retain only others within a loop
				param_updates.push_back(make_pair(target_ID, passed));
			}
		}	

      // If there is a self-loop, add it for all the BA states (its an intersection of transitional parameters for independent loops)
		for(StateID BA_state = 0; BA_state < BA_presence.size(); BA_state++) {
			if (BA_presence[BA_state]) {
				if (loop_params) {
					StateID target = product.getProductID(KS_state, BA_state) ;
					param_updates.push_back(Coloring(target, loop_params));
				}
				// Null the value
				BA_presence[BA_state] = false;
			}
		}

		// Return all filled updates
		return param_updates;
	}
	
	/**
	 * From the source distribute its parameters and newly colored neighbours shedule for update.
	 *
	 * @param ID	ID of the source state in the product
	 * @param parameters	parameters that will be distributed
	 */
	void transferUpdates(const StateID ID, const Paramset parameters) {
		// Get passed colors, unique for each sucessor
		vector<Coloring> update = move(broadcastParameters(ID, parameters));

		// For all passed values make update on target
		for (auto update_it = update.begin(); update_it != update.end(); update_it++) {
			// Skip empty updates
			if (paramset_helper.none(update_it->second))
				continue;

			// If something new is added to the target, schedule it for an update
			if (storage.soft_update(update_it->first, update_it->second)) {
				// Determine what is necessary to update
				if (user_options.analysis())
					next_round_storage.update(ID, update_it->first, update_it->second);
				else
					next_round_storage.update(update_it->first, update_it->second);
				next_updates.insert(update_it->first);
			}
		}
	}

	/**
	 * Main coloring function - passes parametrizations from newly colored states to their neighbours.
	 * Executed as an BFS - in rounds.
	 */
	void doColoring() {
		// While there are updates, pass them to succesing vertices
		do  {
			// Within updates, find the one with most bits
         StateID ID = getStrongestUpdate();
			// Check if this is not the last round
         if (user_options.timeSeries()) {
				if (product.isFinal(ID))
               markLevels(storage.getColor(ID));
				else
					transferUpdates(ID, storage.getColor(ID) & restrict_mask);
			}
         else
            transferUpdates(ID, storage.getColor(ID));

			// Erase completed update from the set
			updates.erase(ID);

         // If there this round is finished, but there are still paths to find
         if (updates.empty() && to_find ) {
            updates = move(next_updates);
            storage.addFrom(next_round_storage);
            restrict_mask = to_find;
            BFS_level++; // Increase level
			}
		} while (!updates.empty());

		// After the coloring, pass cost to the coloring (and computed colors = starting - not found)		
      if (user_options.timeSeries())
         storage.setResults(BFS_reach, ~to_find & starting);
      else if (starting_state != ~static_cast<size_t>(0))
         storage.setResults(storage.getColor(starting_state));
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTING METHODS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
    * Sets/resets all coloring reference values.
	 *
    * @param parameters starting parameters for the cycle detection
	 * @param _range	range of parameters for this coloring round
	 * @param _updates	states that are will be scheduled for an update in this round
	 */
	void prepareCheck(const Paramset parameters, const Range & _range, set<StateID> start_updates = set<StateID>()) {
		starting = to_find = restrict_mask = parameters; // Store which parameters are we searching for
      updates = move(start_updates); // Copy starting updates
		synthesis_range = _range; // Copy range of this round

		BFS_level = 0; // Set sterting number of BFS
		next_updates.clear(); // Ensure emptiness of the next round
		BFS_reach.clear();
		BFS_reach.resize(paramset_helper.getSetSize(), INF); // Begin with infinite reach (symbolized by INF)
      next_round_storage.reset(); // Copy starting values
	}

	ModelChecker(const ModelChecker & other); ///< Forbidden copy constructor.
	ModelChecker& operator=(const ModelChecker & other); ///< Forbidden assignment operator.

public:
	/**
    * Constructor, passes the data and sets up auxiliary storage.
	 */
   ModelChecker(const ConstructionHolder & holder, ColorStorage & _storage) : product(holder.getProduct()), storage(_storage) {
      BA_presence.resize(holder.getAutomatonStructure().getStateCount(), false);
      next_round_storage = storage; // Create an identical copy of the storage.
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STARTERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Start a new coloring round for cycle detection from a single state.
	 *
	 * @param ID	ID of the state to start cycle detection from
	 * @param parameters	starting parameters for the cycle detection
	 * @param _range	range of parameters for this coloring round
	 */
	void startColoring(const StateID ID, const Paramset parameters, const Range & _range) {
		prepareCheck(parameters, _range);
		transferUpdates(ID, parameters); // Transfer updates from the start of the detection
      starting_state = ID;
		doColoring();
	}

	/**
    * Start a new coloring round for coloring of the nodes from the set of inital ones.
	 *
	 * @param parameters	starting parameters to color the structure with
	 * @param _updates	states that are will be scheduled for an update in this round
	 * @param _range	range of parameters for this coloring round
	 */
	void startColoring(const Paramset parameters, const set<StateID> & _updates, const Range & _range){
		prepareCheck(parameters, _range, _updates);
      starting_state = ~static_cast<size_t>(0);
		doColoring();
	}
};

#endif // PARSYBONE_MODEL_CHECKER_INCLUDED
