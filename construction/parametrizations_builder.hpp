/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_PARAMETRIZATIONS_BUILDER_INCLUDED
#define PARSYBONE_PARAMETRIZATIONS_BUILDER_INCLUDED

#include "../parsing/model.hpp"
#include "constrains_parser.hpp"
#include "parametrizations_holder.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FunctionsBuilder builds forms for all possible parametrizations of a single specie from the implicit form in the model.
/// Functions are created from interactions and regulations data by obtaining exact levels of species in which the regulations are active.
/// Functions are built with some auxiliary precomputed data which fasten usage of the functions.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ParametrizationsBuilder {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Provided with constructor
	const Model & model; ///< Model that holds the data
	const ConstrainsParser & constrains; ///< Information about edge constrains
	ParametrizationsHolder & parametrizations_holder; ///< FunctionsStructure class to fill

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTING FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	/**
	 * Compute values that are required from all the species for regulation to be active
	 * - if the regulator is present, add activation values, if the regulator is not present, add the complement.
	 *
	 * @param interactions	all the regulators
	 * @param mask mask of the present regulators - true if the interaction is active
	 *
	 * @return	levels corresponding species (by vector index) have to be at
	 */
	std::vector<std::vector<std::size_t> > getSourceValues(const std::vector<Model::Interaction> & interactions, const std::vector<bool> & mask) const {
		// Data to return
		std::vector<std::vector<std::size_t> > source_values;

		// For each regulating interaction pass the information
		for (std::size_t interaction_num = 0; interaction_num < interactions.size(); interaction_num++) {
			std::vector<std::size_t> possible_levels; // In wich levels the regulating specie has to be for regulation to be active?
			
			// If regulation has to be active, store values from treshold above
			if (mask[interaction_num] == true) 
				for (std::size_t possible_level = interactions[interaction_num].threshold; possible_level <= model.getMax(interactions[interaction_num].source); possible_level++)
					possible_levels.push_back(possible_level);
			// Otherwise store levels below the treshold
			else
				for (std::size_t possible_level = 0; possible_level < interactions[interaction_num].threshold; possible_level++)
					possible_levels.push_back(possible_level);

			// Store computed values
			source_values.push_back(std::move(possible_levels));
		}

		return source_values;
	}
	
	/**
	 * Creates the kinetic parameters in explicit form from the model information.
	 * All feasible parameters for the specie are then stored in the FunctionsStructure.
	 *
	 * @param ID	ID of the specie to compute the kinetic parameters for
	 * @param step_size	number for steps between parametrization change of this specie - this value grows with each successive specie.
	 */
	void addRegulations(const SpecieID ID, std::size_t & step_size) const {
		// get referecnces to Specie data
		const std::vector<Model::Interaction> & interactions = model.getInteractions(ID);
		const std::vector<Model::Regulation> & regulations = model.getRegulations(ID);

		// Go through regulations of a specie - each represents a single function
		std::size_t regul_num = 0;
		for (auto regul_it = regulations.begin(); regul_it != regulations.end(); regul_it++, regul_num++) {
			// Compute allowed values for each regulating specie for this function to be active
			std::vector<std::vector<std::size_t> > source_values = std::move(getSourceValues(interactions, regul_it->first));

			// Add target values (if input negative, add all possibilities), if positive, add current requested value
			std::vector<std::size_t> possible_values = std::move(constrains.getTargetVals(ID, regul_num));

			// pass the function to the holder.
			parametrizations_holder.addRegulatoryFunction(ID, step_size, std::move(possible_values), std::move(source_values));
		}

		// Display stats
		std::string specie_stats = "Specie " + model.getName(ID) + " has " + boost::lexical_cast<std::string>(regulations.size())
				+ " regulators with " + boost::lexical_cast<std::string>(constrains.getColorsNum(ID)) + " possible regulatory contexts.";
		output_streamer.output(stats_str, specie_stats, OutputStreamer::tab);

		// Increase step size for the next function
		step_size *= constrains.getColorsNum(ID);
	}

	/**
	 * Get all the values possible for given specie (range [min, max])
	 *
	 * @param specie_ID	specie to get the values from
	 *
	 * @return vector of all specie levels
	 */
	std::vector<std::size_t> getPossibleValues(const std::size_t specie_ID) const {
		// Storage
		std::vector<std::size_t> possible_values;
		// Add all the values between 0 and max
		for (std::size_t possible_value = model.getMin(specie_ID); possible_value <= model.getMax(specie_ID); possible_value++)
			possible_values.push_back(possible_value);

		return possible_values;
	}

	/**
	 * Get IDs of the sources of regulations for this specie
	 *
	 * @param specie_ID	specie to get the values from
	 *
	 * @return vector of all species that have outcoming interaction to this specie
	 */
	std::vector<std::size_t> getSourceSpecies(const std::size_t specie_ID) const {
		// Storage
		std::vector<std::size_t> source_species;
		// Get reference
		const std::vector<Model::Interaction> & interactions = model.getInteractions(specie_ID);
		// Add all the values between 0 and max
		for (auto inter_it = interactions.begin(); inter_it != interactions.end(); inter_it++) 
			source_species.push_back(inter_it->source);

		return source_species;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTING FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ParametrizationsBuilder(const ParametrizationsBuilder & other); ///< Forbidden copy constructor.
	ParametrizationsBuilder& operator=(const ParametrizationsBuilder & other); ///< Forbidden assignment operator.

public:
	/**
	 * Constructor just attaches the references to data holders
	 */
	ParametrizationsBuilder(const Model & _model, const ConstrainsParser & _constrains, ParametrizationsHolder & _parametrizations_holder)
		: model(_model), constrains(_constrains), parametrizations_holder(_parametrizations_holder)  { }

	/**
	 * For each specie recreate all its regulatory functions (all possible parametrizations)
	 */
	void buildParametrizations() {
		// Display stats
		output_streamer.output(stats_str, "Costructing Regulatory kinetics for ", OutputStreamer::no_newl)
			           .output(model.getSpeciesCount(), OutputStreamer::no_newl).output(" species.");

		std::size_t step_size = 1; // Variable necessary for encoding of colors 

		// Cycle through all the species
		for (std::size_t specie_num = 0; specie_num < model.getSpeciesCount(); specie_num++) {

			// Compute data
			std::vector<std::size_t> possible_values = std::move(getPossibleValues(specie_num));
			std::vector<std::size_t> source_species = std::move(getSourceSpecies(specie_num));

			// Add specie
			parametrizations_holder.addSpecie(model.getName(specie_num), specie_num, std::move(possible_values), std::move(source_species));
			
			// Add regulations for this specie
			addRegulations(specie_num, step_size);
		}

		// Set the number by what would be step size for next function
		parametrizations_holder.parameter_count = step_size;
	}
};
#endif // PARSYBONE_PARAMETRIZATIONS_BUILDER_INCLUDED
