/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_LABELING_BUILDER_INCLUDED
#define PARSYBONE_LABELING_BUILDER_INCLUDED

#include "PunyHeaders/common_functions.hpp"

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
   void addRegulations(const SpecieID t_ID, ParamNum & step_size) const {
      // get referecnces to Specie data
      const auto & tparams = model.getParameters(t_ID);

      // Go through regulations of a specie - each represents a single function
      for (auto param_num:range(tparams.size())) {
         Configurations source_values;
         // Compute allowed values for each regulating specie for this function to be active
         for (auto source_num:tparams[param_num].requirements) {
            source_values.push_back(source_num.second);
         }

         // Add target values (if input negative, add all possibilities), if positive, add current requested value
         auto possible_values = parametrizations.getTargetVals(t_ID, param_num);

         // pass the function to the holder.
         labeling_holder.addRegulatoryFunction(t_ID, step_size, possible_values, source_values);
      }

      // Display stats
      output_streamer.output(verbose_str, "Specie " + model.getName(t_ID) + " has " + toString(tparams.size()) + " regulatory contexts with "
                             + toString(parametrizations.getColorsNum(t_ID)) + " possible parametrizations out of " + toString(parametrizations.getAllColorsNum(t_ID)) + ".");

      // Increase step size for the next function
      step_size *= parametrizations.getColorsNum(t_ID);
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

		ParamNum step_size = 1; // Variable necessary for encoding of colors

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
