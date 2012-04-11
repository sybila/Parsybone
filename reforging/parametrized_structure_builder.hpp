/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_PARAMETRIZED_STRUCTURE_BUILDER_INCLUDED
#define PARSYBONE_PARAMETRIZED_STRUCTURE_BUILDER_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ParametrizedStructureBuilder creates the ParametrizedStructure from the model data.
// States are read from the basic structure and passed to the parametrized structure, then the transitions are added.
// Each transition is supplemented with a label - mask of transitive values and the its function ID.
// This expects semantically correct data from BasicStructure and FunctionsStructure.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "basic_structure.hpp"
#include "functions_structure.hpp"
#include "parametrized_structure.hpp"

class ParametrizedStructureBuilder {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Provided with constructor
	const BasicStructure & basic_structure; // Provider of basic KS data
	const FunctionsStructure & regulatory_functions; // Provider of implicit functions
	ParametrizedStructure & structure; // KipkeStructure to fill

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTING FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	 * Test wheather the current state corresponds to the requirments put on values of the specified species.
	 * 
	 * @param source_species	Species that can possibly regulate the target
	 * @param source_values	In which levels the species have to be for regulation to be active
	 * @param state_levels	activation levels of the current state
	 *
	 * @return true it the state satisfy the requirments
	 */
	const bool testRegulators(const std::vector<std::size_t> & source_species, const std::vector<std::vector<std::size_t>> & source_values, const Levels & state_levels) {
		// List throught regulating species of the function
		for (std::size_t regulator_num = 0; regulator_num < source_species.size(); regulator_num++) {
			bool is_included = false; // Remains false if the specie level is not in allowed range
			const std::size_t specie_ID = source_species[regulator_num]; // real ID of the regulator

			// Does current level of the specie belongs to the levels that are required?
			std::for_each(source_values[regulator_num].begin(), source_values[regulator_num].end(),	[&state_levels,&is_included,&source_species,specie_ID]
				(std::size_t feasible_level) {
					if (feasible_level == state_levels[specie_ID])
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
	 * @state_levels	species level of the state we are currently at
	 *
	 * @return function that might lead to the next state
	 */
	const std::size_t getActiveFunction(const std::size_t state_ID, const std::size_t neighbour_index, const Levels & state_levels) {
		// Positions between which the function has to be
		std::size_t function_num = regulatory_functions.getSpecieBegin(basic_structure.getSpecieID(state_ID, neighbour_index));
		const std::size_t search_to = regulatory_functions.getSpecieBegin(basic_structure.getSpecieID(state_ID, neighbour_index) + 1);
		
		// Cycle until the function is found
		bool found = false;
		do {
			found = testRegulators(regulatory_functions.getSourceSpecies(function_num), regulatory_functions.getSourceValues(function_num), state_levels);
			if (found) return function_num;
		} while (++function_num < search_to); 

		throw std::length_error("Function for some state has not been found.");
	}

	/**
	 * Creates a mask of transitivity for all the target values of the current function.
	 * 
	 * @param direction	the way the specie's value changes
	 * @param current_specie_level	current value the specie has in this state
	 * @param possible_values	all the values the function can have
	 *
	 * @return mask of transitivity - false means the value is not allowed for this transition
	 */
	std::vector<bool> fillTransitivityData(const Direction direction, const std::size_t current_specie_level, const std::vector<std::size_t> & possible_values){
		// Vector to fill
		std::vector<bool> transitive_values;
		// Based on direction of the change create a mask of transitivity for all parameter values
		if (direction == up_dir) {
			std::for_each(possible_values.begin(), possible_values.end(),[current_specie_level, &transitive_values](const std::size_t value){
				if (value < current_specie_level + 1) // If the value is not at least as high as the target
					transitive_values.push_back(false);
				else transitive_values.push_back(true);
			});
		}
		else if (direction == stay_dir){
			std::for_each(possible_values.begin(), possible_values.end(),[current_specie_level, &transitive_values](const std::size_t value){
				if (value != current_specie_level) // If the value is not same as the target
					transitive_values.push_back(false);
				else transitive_values.push_back(true);
			});
		}
		else if (direction == down_dir) {
			std::for_each(possible_values.begin(), possible_values.end(),[current_specie_level, &transitive_values](const std::size_t value){
				if (value > current_specie_level - 1) // If the value is not at least as low as the target
					transitive_values.push_back(false);
				else transitive_values.push_back(true);
			});
		}
		return transitive_values;
	}

	/**
	 * Fill properties of an implicit function that is connected to specified transition
	 * 
	 * @param state_ID	ID of this state in KS
	 * @param neighbour_index	index of the neighbour state. Specie that change is used to determine wich function to use.
	 * @param state_levels	species level of the state we are currently at
	 * Data to fill:
	 * @param function_num	ID of the active function
	 * @param step_size	step size of the function
	 * @param possible_values	all parameter values of the function
	 * @param transitive_values	those parameter values that does not cause transition
	 *
	 * @return true if there is a possibility of transition, false otherwise
	 */
	const bool fillFunctions(const std::size_t state_ID, const std::size_t neighbour_index, const Levels & state_levels, 
		                     std::size_t & function_num, std::size_t & step_size, std::vector<bool> & transitive_values) {
		// Find out which function is currently active
		function_num = getActiveFunction(state_ID, neighbour_index, state_levels);

		// Fill step size
		step_size = regulatory_functions.getStepSize(function_num);

		// Fill data about transitivity using provided values
		transitive_values = std::move(fillTransitivityData(basic_structure.getDirection(state_ID, neighbour_index), 
			                                                   state_levels[basic_structure.getSpecieID(state_ID, neighbour_index)], 
			                                                   regulatory_functions.getPossibleValues(function_num)));

		// Check if there even is a transition
		for (auto it = transitive_values.begin(); it != transitive_values.end(); it++) {
			if (*it == true)
				return true;
		}
		return false;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTING FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ParametrizedStructureBuilder(const ParametrizedStructureBuilder & other);            // Forbidden copy constructor.
	ParametrizedStructureBuilder& operator=(const ParametrizedStructureBuilder & other); // Forbidden assignment operator.

public:
	/**
	 * Constructor just attaches the references to data holders
	 */
	ParametrizedStructureBuilder(const BasicStructure & _basic_structure, const FunctionsStructure & _regulatory_functions, ParametrizedStructure & _structure) 
		: regulatory_functions(_regulatory_functions), basic_structure(_basic_structure), structure(_structure)  {
	}

	/**
	 * Create the states from the model and fill the structure with them.
	 */
	void buildStructure() {
		output_streamer.output(stats_str, "Merging functions and Kripke structure into Parametrized Kripke structure.");
		// Just pass the value
		structure.setStepSizes(regulatory_functions.getStepSizes());

		// Recreate all the states of the simple structure
		for(std::size_t state_num = 0; state_num < basic_structure.getStateCount(); state_num++) {
			// Create a new state from the known data
			const Levels & state_levels = basic_structure.getStateLevels(state_num);
			structure.addState(state_num, state_levels);

			// For each existing neighbour add a transition to the newly created state
			for (std::size_t neighbour_index = 0; neighbour_index < basic_structure.getTransitionsCount(state_num); neighbour_index++) {
				// Data to fill
				std::size_t target_ID = basic_structure.getTargetID(state_num, neighbour_index); // ID of the state the transition leads to
				std::size_t step_size = 1; // How many bits of a parameter space bitset is needed to get from one targe value to another
				std::size_t function_num = ~0; // ID of the active function - if ~0, no function is active
				std::vector<bool> transitive_values; // Which of possible are used (in this case)

				// Fill data about the transition and check if it is even feasible
				if (fillFunctions(state_num, neighbour_index, state_levels, function_num, step_size, transitive_values)) {
					// Add the transition
					structure.addTransition(state_num, target_ID, function_num, step_size, std::move(transitive_values));
				}
			}
		}
	}
};

#endif