/*
 * Copyright (C) 2012 - Adam Streck
 *
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 *
 * ParSyBoNe is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 *
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno.
 * See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_FUNCTIONS_BUILDER_INCLUDED
#define PARSYBONE_FUNCTIONS_BUILDER_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FunctionsBuilder recreates functions from explicit form of the model into more utilizable apperance.
// Functions are created from interactions and regulations data by obtaining exact levels of species in which the regulations are active.
// Functions are built with some auxiliary precomputed data which fasten usage of the functions.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <memory>

#include "../parsing/model.hpp"
#include "functions_structure.hpp"
#include "../results/output_streamer.hpp"

class FunctionsBuilder {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Provided with constructor
	const UserOptions & user_options;
	const Model & model; // Model that holds the data
	FunctionsStructure & functions_structure; // FunctionsStructure class to fill

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTING FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	
	/**
	 * Compute values that are required from all the species for regulation to be active
	 * - if the regulator is present, add activation values, if the regulator is not present, add the complement.
	 *
	 * @param interactions	all the regulators
	 * @param mask mask of the present regulators - true if the interaction is active
	 * @param source_species	IDs of regulation species
	 * @param source_values		levels corresponding species (by vector index) have to be at
	 */
	void getInteractionValues(const std::vector<Model::Interaction> & interactions, const std::vector<bool> & mask, 
		                      std::vector<std::size_t> & source_species, std::vector<std::vector<std::size_t> > & source_values) {
		// For each regulating interaction pass the information
		for (std::size_t interaction_num = 0; interaction_num < interactions.size(); interaction_num++) {
			source_species.push_back(interactions[interaction_num].first); // Regulating specie ID
			std::vector<std::size_t> possible_levels; // In wich levels the regulating specie has to be for regulation to be active?
			
			// If regulation has to be active, store values from treshold above
			if (mask[interaction_num] == true) 
				for (std::size_t possible_level = interactions[interaction_num].second; possible_level <= model.getMax(interactions[interaction_num].first); possible_level++)
					possible_levels.push_back(possible_level);
			// Otherwise store levels below the treshold
			else
				for (std::size_t possible_level = 0; possible_level < interactions[interaction_num].second; possible_level++)
					possible_levels.push_back(possible_level);

			source_values.push_back(std::move(possible_levels));
		}
	}
	
	/**
	 * Compute values that the function can have as target values.
	 * 
	 * @param target_val	target value as specified in the model
	 * @param specie_ID	ID of the specie that is regulated by the function
	 *
	 * @return possible target values for given function
	 */
	std::vector<std::size_t> computePossibleValues(const int target_val, const std::size_t specie_ID) {
		std::vector<std::size_t> possible_values;
		// Add target values (if input negative, add all possibilities), if positive, add current requested value
		if (target_val >= 0 )
			possible_values.push_back(target_val);
		else
			for (std::size_t possible_value = 0; possible_value <= model.getMax(specie_ID); possible_value++) 
				possible_values.push_back(possible_value);
		return possible_values;
	}

	/**
	 * Creates the functions in explicit form from the model information.
	 */
	void buildFunctionsStructure() {
		for (std::size_t specie_num = 0; specie_num < model.getSpeciesCount(); specie_num++) {
			// Get information
			const std::vector<Model::Interaction> & interactions = model.getInteractions(specie_num);
			const std::vector<Model::Regulation> & regulations = model.getRegulations(specie_num);

			output_streamer.output(verbose, "Computing functions for specie ", OutputStreamer::no_newl).output(verbose, specie_num, OutputStreamer::no_newl)
				           .output(verbose, " with ", OutputStreamer::no_newl).output(verbose, interactions.size(), OutputStreamer::no_newl)
				           .output(verbose, " interactions and ", OutputStreamer::no_newl).output(verbose, regulations.size(), OutputStreamer::no_newl)
						   .output(verbose, " regulatory contexts.");
			// Go through regulations of a specie - each represents a single function
			for (auto it = regulations.begin(); it != regulations.end(); it++) {
				// Get data from interactions
				std::vector<std::size_t> source_species;
				std::vector<std::vector<std::size_t> > source_values;
				getInteractionValues(interactions, it->first, source_species, source_values);

				// Add target values (if input negative, add all possibilities), if positive, add current requested value
				std::vector<std::size_t> possible_values = std::move(computePossibleValues(it->second, specie_num));

				// pass the function to the holder.
				functions_structure.addRegulatoryFunction(specie_num, std::move(source_species), std::move(source_values), std::move(possible_values));
			}
		}
	}

	/**
	 * Computes iformation about where functions with common target starts and how big are steps in parameter set.
	 */
	void computeAuxiliaryData() {
		output_streamer.output(verbose, "Computing auxiliary data for functions.");
		// Aid variables
		std::size_t last_target = 0;
		std::size_t function_num = 0;
		std::size_t step_size = 1;

		// Add species begin when the target specie ends. Compute step lenght as an iterative multiplication by size of possibilities of previous functions.
		functions_structure.addSpecieBegin(0);
		for (function_num = 0; function_num < functions_structure.reg_functions.size(); function_num++) {
			// Store where it begins
			if (functions_structure.getTarget(function_num) != last_target) {
				functions_structure.addSpecieBegin(function_num);
				last_target = functions_structure.getTarget(function_num);
			}
			// Store how many combinations of target values of previous functions there are
			functions_structure.addStepSize(step_size);
			step_size *= functions_structure.getPossibleValues(function_num).size();
		}
		functions_structure.addSpecieBegin(function_num);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTING FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	FunctionsBuilder(const FunctionsBuilder & other);            // Forbidden copy constructor.
	FunctionsBuilder& operator=(const FunctionsBuilder & other); // Forbidden assignment operator.

public:
	/**
	 * Constructor just attaches the references to data holders
	 */
	FunctionsBuilder(const UserOptions & _user_options, const Model & _model, FunctionsStructure & _functions_structure) 
		: user_options(_user_options), model(_model), functions_structure(_functions_structure)  {	}

	/**
	 * Create the functions from the model 
	 */
	void buildFunctions() { 
		buildFunctionsStructure();
		computeAuxiliaryData();
	}
};
#endif