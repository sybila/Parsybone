/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_LABELING_BUILDER_INCLUDED
#define PARSYBONE_LABELING_BUILDER_INCLUDED

#include "../auxiliary/common_functions.hpp"
#include "../parsing/model.hpp"
#include "parametrizations_builder.hpp"
#include "labeling_holder.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Creates a labeled graph representation of gene regulatory network and stores it within a LabelingHolder object.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class LabelingBuilder {
	// Provided with constructor
   const Model & model; ///< Model that holds the data.
   const ParametrizationsHolder & parametrizations; ///< Precomputed partial parametrizations.
   LabelingHolder & labeling_holder; ///< FunctionsStructure class to fill.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTING METHODS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/**
	 * Compute values that are required from all the species for regulation to be active
	 * - if the regulator is present, add activation values, if the regulator is not present, add the complement.
	 * Regulator is active when lower_threshold <= activity level < upper threshold.
	 *
    * @param regulations	all the regulators
    * @param mask mask of the present regulators - true if the regulation is active
	 *
	 * @return	levels corresponding species (by vector index) have to be at
	 */
   std::vector<std::vector<std::size_t> > getSourceValues(const std::vector<Model::Regulation> & regulations, const std::vector<bool> & mask) const {
      // Data to return
      std::vector<std::vector<std::size_t> > source_values;

      // For each regulating regulation pass the information
      for (std::size_t regulation_num = 0; regulation_num < regulations.size(); regulation_num++) {
         std::vector<std::size_t> possible_levels; // In wich levels the regulating specie has to be for regulation to be active?
			
			// Store currtently used values - if there is no multiedge with higher threshold, consider one above maximal level to be the upper threshold
			SpecieID source = regulations[regulation_num].source;
			std::size_t lower_threshold = regulations[regulation_num].threshold; // First value activating the regulation
			std::size_t upper_threshold = model.getMax(source) + 1; // First value that is again deactivating the regulation

			// Obtain upper threshold from the other regulations
			forEach(regulations, [&upper_threshold,lower_threshold,source](const Model::Regulation & regulation) {
				if (source == regulation.source && lower_threshold != regulation.threshold)
						  upper_threshold = lower_threshold <= regulation.threshold && regulation.threshold <= upper_threshold ? regulation.threshold : upper_threshold;
			});

			// If regulation has to be active, store values from treshold above
			if (mask[regulation_num] == true) {
				for (std::size_t possible_level = lower_threshold; possible_level < upper_threshold; possible_level++)
               possible_levels.push_back(possible_level);
         }
			// Otherwise store levels below the treshold
			else {
				for (std::size_t possible_level = model.getMin(source); possible_level < lower_threshold; possible_level++)
					possible_levels.push_back(possible_level);
				for (std::size_t possible_level = upper_threshold; possible_level < model.getMax(source) + 1; possible_level++)
					possible_levels.push_back(possible_level);
			}

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
      const std::vector<Model::Regulation> & regulations = model.getRegulations(ID);
      const std::vector<Model::Parameter> & parameters = model.getParameters(ID);

		// Go through regulations of a specie - each represents a single function
		std::size_t regul_num = 0;
		for (auto param_it = parameters.begin(); param_it != parameters.end(); param_it++, regul_num++) {
			// Compute allowed values for each regulating specie for this function to be active
			std::vector<std::vector<std::size_t> > source_values = std::move(getSourceValues(regulations, param_it->first));

			// Add target values (if input negative, add all possibilities), if positive, add current requested value
			std::vector<std::size_t> possible_values = std::move(parametrizations.getTargetVals(ID, regul_num));

			// pass the function to the holder.
			labeling_holder.addRegulatoryFunction(ID, step_size, std::move(possible_values), std::move(source_values));
		}

		// Display stats
      std::string specie_stats = "Specie " + model.getName(ID) + " has " + toString(parameters.size()) + " regulatory contexts with "
                                 + toString(parametrizations.getColorsNum(ID)) + " total possible parametrizations.";
		output_streamer.output(stats_str, specie_stats, OutputStreamer::tab);

		// Increase step size for the next function
		step_size *= parametrizations.getColorsNum(ID);
	}

	/**
    * Get all the values possible for given specie (range [min, max]).
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
    * Get IDs of the sources of regulations for this specie.
	 *
	 * @param specie_ID	specie to get the values from
	 * @param duplicit	if true, multiple occurences of the same specie (multiedges) are conserved
	 *
    * @return vector of all species that have outcoming regulation to this specie
	 */
	std::vector<std::size_t> getSourceSpecies(const std::size_t specie_ID, const bool duplicit = true) const {
		// Storage
		std::vector<std::size_t> source_species;
		// Get reference
      const std::vector<Model::Regulation> & regulations = model.getRegulations(specie_ID);
		// Add all the values between 0 and max
      for (auto regul_it = regulations.begin(); regul_it != regulations.end(); regul_it++)
         if (source_species.empty() || (source_species.back() != regul_it->source) || duplicit)
            source_species.push_back(regul_it->source);

		return source_species;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTING METHODS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	LabelingBuilder(const LabelingBuilder & other); ///< Forbidden copy constructor.
	LabelingBuilder& operator=(const LabelingBuilder & other); ///< Forbidden assignment operator.

public:
	/**
	 * Constructor just attaches the references to data holders
	 */
	LabelingBuilder(const Model & _model, const ParametrizationsHolder & _parametrizations, LabelingHolder & _labeling_holder)
		: model(_model), parametrizations(_parametrizations), labeling_holder(_labeling_holder)  { }

	/**
	 * For each specie recreate all its regulatory functions (all possible labels)
	 */
	void buildLabeling() {
		// Display stats
		output_streamer.output(stats_str, "Costructing Regulatory kinetics for ", OutputStreamer::no_newl)
                     .output(model.getSpeciesCount(), OutputStreamer::no_newl).output(" species.");

		std::size_t step_size = 1; // Variable necessary for encoding of colors 

		// Cycle through all the species
		for (std::size_t specie_num = 0; specie_num < model.getSpeciesCount(); specie_num++) {

			// Compute data for independent regulations
			std::vector<std::size_t> possible_values = std::move(getPossibleValues(specie_num));
			std::vector<std::size_t> source_species = std::move(getSourceSpecies(specie_num, true));

			// Add specie
			labeling_holder.addSpecie(model.getName(specie_num), specie_num, std::move(possible_values), std::move(source_species));
			
			// Add regulations for this specie
			addRegulations(specie_num, step_size);
		}

		// Set the number by what would be step size for next function
		labeling_holder.parameter_count = step_size;
	}
};
#endif // PARSYBONE_LABELING_BUILDER_INCLUDED
