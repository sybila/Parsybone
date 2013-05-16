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
#include "labeling_holder.hpp"
#include "unparametrized_structure.hpp"

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
   const BasicStructure & basic_structure; ///< Provider of basic KS data.
   const LabelingHolder & regulatory_functions; ///< Provider of implicit functions.
   UnparametrizedStructure & structure; ///< KipkeStructure to fill.

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
	bool testRegulators(const vector<StateID> & source_species, const Configurations & source_values, const Levels & state_levels) {
		// List throught regulating species of the function
		for (size_t regulator_num = 0; regulator_num < source_species.size(); regulator_num++) {
			bool is_included = false; // Remains false if the specie level is not in allowed range
			const StateID ID = source_species[regulator_num]; // real ID of the regulator

			// Does current level of the specie belongs to the levels that are required?
			for_each(source_values[regulator_num].begin(), source_values[regulator_num].end(), [&]
				(size_t feasible_level) {
					if (feasible_level == state_levels[ID])
						is_included = true;
			});

			// If specie does not meet requirments, do not even continue.
			if (!is_included) {
				return false;
			}
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
	size_t getActiveFunction(const SpecieID specie_ID, const Levels & state_levels) {
		// Source species that will be tested
		const vector<size_t> & source_species = regulatory_functions.getSourceSpecies(specie_ID);

		// Cycle until the function is found
		bool found = false;
		for (size_t regul_num = 0; regul_num < regulatory_functions.getRegulationsCount(specie_ID); regul_num++) {
			const auto source_vals = regulatory_functions.getSourceValues(specie_ID, regul_num);

			found = testRegulators(source_species, source_vals, state_levels);

			if (found) 
				return regul_num;
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
	bool fillFunctions(const StateID ID, const StateID neighbour_index, const Levels & state_levels,
									ParamNum & step_size, vector<bool> & transitive_values) {
		// Get ID of the regulated specie
		const size_t specie_ID = basic_structure.getSpecieID(ID, neighbour_index);

		// Find out which function is currently active
		size_t function_num = getActiveFunction(specie_ID, state_levels);

		// Fill the step size
		step_size = regulatory_functions.getStepSize(specie_ID, function_num);

		// Fill data about transitivity using provided values
      transitive_values = fillTransitivityData(basic_structure.getDirection(ID, neighbour_index),  state_levels[specie_ID],
                                                        regulatory_functions.getPossibleValues(specie_ID, function_num));

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
	void addTransitions(const StateID ID, const Levels & state_levels) {
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
	
	UnparametrizedStructureBuilder(const UnparametrizedStructureBuilder & other); ///< Forbidden copy constructor.
	UnparametrizedStructureBuilder& operator=(const UnparametrizedStructureBuilder & other); ///< Forbidden assignment operator.

public:
	/**
    * Constructor just attaches the references to data holders.
	 */
	UnparametrizedStructureBuilder(const BasicStructure & _basic_structure, const LabelingHolder & _regulatory_functions, UnparametrizedStructure & _structure)
        : basic_structure(_basic_structure), regulatory_functions(_regulatory_functions), structure(_structure)  {
	}

	/**
    * Create the states from the model and fill the structure with them.
	 */
	void buildStructure() {
		output_streamer.output(verbose_str, "Merging the kinetic functions and the basic Kripke structure into a parametrized Kripke structure.");

		// Recreate all the states of the simple structure
		for(StateID ID = 0; ID < basic_structure.getStateCount(); ID++) {

			// Create a new state from the known data
			const Levels & state_levels = basic_structure.getStateLevels(ID);
			const string & label = basic_structure.getString(ID);
			structure.addState(ID, state_levels, label);

			// Add all the transitions
			addTransitions(ID, state_levels);
		}
	}
};

#endif // PARSYBONE_UNPARAMETRIZED_STRUCTURE_BUILDER_INCLUDED
