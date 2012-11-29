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
	 * Creates the kinetic parameters in explicit form from the model information.
	 * All feasible parameters for the specie are then stored in the FunctionsStructure.
	 *
	 * @param ID	ID of the specie to compute the kinetic parameters for
	 * @param step_size	number for steps between parametrization change of this specie - this value grows with each successive specie.
	 */
	void addRegulations(const SpecieID ID, size_t & step_size) const {
		// get referecnces to Specie data
		const auto & tparams = model.getTParams(ID);

		// Go through regulations of a specie - each represents a single function
		for (auto param:tparams) {
			Configurations source_values;
			// Compute allowed values for each regulating specie for this function to be active
			for (auto source_num:param.requirements) {
				source_values.push_back(source_num.second);
			}

			// Add target values (if input negative, add all possibilities), if positive, add current requested value
			auto possible_values = param.target;

			// pass the function to the holder.
			labeling_holder.addRegulatoryFunction(ID, step_size, possible_values, source_values);
		}

		// Display stats
		string specie_stats = "Specie " + model.getName(ID) + " has " + toString(tparams.size()) + " regulatory contexts with "
                                 + toString(parametrizations.getColorsNum(ID)) + " total possible parametrizations.";
		output_streamer.output(stats_str, specie_stats, OutputStreamer::tab);

		// Increase step size for the next function
		step_size *= parametrizations.getColorsNum(ID);
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

		size_t step_size = 1; // Variable necessary for encoding of colors 

		// Cycle through all the species
		for (auto ID:range(model.getSpeciesCount())) {
			// Add specie
			labeling_holder.addSpecie(model.getName(ID), ID, model.getRegulatorsIDs(ID));
			
			// Add regulations for this specie
			addRegulations(ID, step_size);
		}

		// Set the number by what would be step size for next function
		labeling_holder.parameter_count = step_size;
	}
};
#endif // PARSYBONE_LABELING_BUILDER_INCLUDED
