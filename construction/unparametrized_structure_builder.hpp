/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_UNPARAMETRIZED_STRUCTURE_BUILDER_INCLUDED
#define PARSYBONE_UNPARAMETRIZED_STRUCTURE_BUILDER_INCLUDED

#include "basic_structure.hpp"
#include "unparametrized_structure.hpp"
#include "../model/model_translators.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Creates a UnparametrizedStructure as a composition of a BasicStructure and ParametrizationsHolder.
///
/// UnparametrizedStructureBuilder creates the UnparametrizedStructure from the model data.
/// States are read from the basic structure and passed to the unparametrized structure, then the transitions are added.
/// Each transition is supplemented with a label - mask of transitive values and the its function ID.
/// This expects semantically correct data from BasicStructure and FunctionsStructure.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class UnparametrizedStructureBuilder {
   // Provided with the constructor
   const Model & model;
   const BasicStructure & basic_structure; ///< Provider of basic KS data.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TESTING METHODS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Test wheather the current state corresponds to the requirements put on values of the specified species.
	 * 
    * @param source_species	species that can possibly regulate the target
    * @param source_values	in which levels the species have to be for regulation to be active
	 * @param state_levels	activation levels of the current state
	 *
    * @return true it the state satisfy the requirements
	 */
   bool testRegulators(const map<StateID, Levels> requirements, const Levels & state_levels) {
		// List throught regulating species of the function
      for (auto regul:requirements) {
         if (count(regul.second.begin(), regul.second.end(), state_levels[regul.first]) == 0)
            return false;
		}

		// Return true if all species passed.
		return true;
	}

	/**
	 * Obtain index of the function that might lead to the specified state based on current activation levels of the species and target state.
	 * 
	 * @param neighbour_num	index of the neighbour state. Specie that change is used to determine wich function to use.
    * @param state_levels	species level of the state we are currently at
	 *
	 * @return function that might lead to the next state
	 */
   size_t getActiveFunction(const SpecieID ID, const Levels & state_levels) {
		// Cycle until the function is found
		bool found = false;
      for (size_t param_no = 0; param_no < model.getParameters(ID).size(); param_no++) {
         found = testRegulators(model.getParameters(ID)[param_no].requirements, state_levels);

			if (found) 
            return param_no;
		} 
		throw runtime_error("Active function in some state not found.");
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FILLING METHODS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Creates a mask of transitivity for all the target values of the current function.
	 * 
	 * @param direction	the way the specie's value changes
	 * @param current_specie_level	current value the specie has in this state
	 * @param possible_values	all the values the function can have
	 *
	 * @return mask of transitivity - false means the value is not allowed for this transition
	 */
	vector<bool> fillTransitivityData(const Direction direction, const size_t current_specie_level, const Levels & possible_values){
		// Vector to fill
		vector<bool> transitive_values;
		// Based on direction of the change create a mask of transitivity for all parameter values
		if (direction == up_dir) {
			for_each(possible_values.begin(), possible_values.end(),[current_specie_level, &transitive_values](const size_t value){
				if (value < current_specie_level + 1) // If the value is not at least as high as the target
					transitive_values.push_back(false);
				else transitive_values.push_back(true);
			});
		}
		else if (direction == stay_dir){
			for_each(possible_values.begin(), possible_values.end(),[current_specie_level, &transitive_values](const size_t value){
				if (value != current_specie_level) // If the value is not same as the target
					transitive_values.push_back(false);
				else transitive_values.push_back(true);
			});
		}
		else if (direction == down_dir) {
			for_each(possible_values.begin(), possible_values.end(),[current_specie_level, &transitive_values](const size_t value){
				if (value > current_specie_level - 1) // If the value is not at least as low as the target
					transitive_values.push_back(false);
				else transitive_values.push_back(true);
			});
		}
		return transitive_values;
	}

	/**
    * Fill properties of the specified transition for the CMC procedure.
	 * 
	 * @param ID	ID of this state in KS
	 * @param neighbour_index	index of the neighbour state. Specie that change is used to determine wich function to use.
	 * @param state_levels	species level of the state we are currently at
	 * @param function_num	ID of the active function
	 * @param step_size	step size of the function
	 * @param transitive_values	those parameter values that does not cause transition
	 *
	 * @return true if there is a possibility of transition, false otherwise
	 */
   bool fillFunctions(const StateID state, const StateID neighbour_index, const Levels & state_levels,
									ParamNum & step_size, vector<bool> & transitive_values) {
		// Get ID of the regulated specie
      const size_t ID = basic_structure.getSpecieID(state, neighbour_index);

		// Find out which function is currently active
      size_t param_no = getActiveFunction(ID, state_levels);

		// Fill the step size
      step_size = model.species[ID].parameters[param_no].step_size;

		// Fill data about transitivity using provided values
      transitive_values = fillTransitivityData(basic_structure.getDirection(state, neighbour_index), state_levels[ID], model.species[ID].parameters[param_no].possible_values);

		// Check if there even is a transition
		for (auto it = transitive_values.begin(); it != transitive_values.end(); it++) {
			if (*it == true)
				return true;
		}
		return false;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTING METHODS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
    * For each existing neighbour add a transition to the newly created state
    *
	 * @param ID	state from which the transitions should lead
	 * @param state_levels	activation levels in this state
	 */
   void addTransitions(const StateID ID, const Levels & state_levels, UnparametrizedStructure & structure) {
		// Go through all the original transitions
		for (size_t trans_num = 0; trans_num < basic_structure.getTransitionCount(ID); trans_num++) {
			// Data to fill
			StateID target_ID = basic_structure.getTargetID(ID, trans_num); // ID of the state the transition leads to
			ParamNum step_size = 1; // How many bits of a parameter space bitset is needed to get from one targe value to another
			vector<bool> transitive_values; // Which of possible are used (in this case)

			// Fill data about the transition and check if it is even feasible
			if (fillFunctions(ID, trans_num, state_levels, step_size, transitive_values)) {
				// Add the transition
				structure.addTransition(ID, target_ID, step_size, move(transitive_values));
			}
		}	
	}
public:
	/**
    * Constructor just attaches the references to data holders.
	 */
   UnparametrizedStructureBuilder(const Model & _model, const BasicStructure & _basic_structure)
        : model(_model), basic_structure(_basic_structure) {
	}

	/**
    * Create the states from the model and fill the structure with them.
	 */
   UnparametrizedStructure buildStructure() {
      UnparametrizedStructure structure;
      const size_t state_count = basic_structure.getStateCount();
      size_t state_no = 0;

		// Recreate all the states of the simple structure
		for(StateID ID = 0; ID < basic_structure.getStateCount(); ID++) {
         output_streamer.output(verbose_str, "Building model state: " + toString(++state_no) + "/" + toString(state_count) + ".", OutputStreamer::no_newl | OutputStreamer::rewrite_ln);

			// Create a new state from the known data
			const Levels & state_levels = basic_structure.getStateLevels(ID);
			const string & label = basic_structure.getString(ID);
			structure.addState(ID, state_levels, label);

			// Add all the transitions
         addTransitions(ID, state_levels, structure);
		}

      output_streamer.output(verbose_str, string(' ', 100), OutputStreamer::no_out | OutputStreamer::rewrite_ln | OutputStreamer::no_newl);

      return structure;
	}
};

#endif // PARSYBONE_UNPARAMETRIZED_STRUCTURE_BUILDER_INCLUDED
