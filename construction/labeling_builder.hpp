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
   /**
    * Creates the kinetic parameters in explicit form from the model information. All feasible parameters for the specie are then stored in the FunctionsStructure.
    * @param ID	ID of the specie to compute the kinetic parameters for
    * @param step_size	number for steps between parametrization change of this specie - this value grows with each successive specie.
    */
   static void addRegulations(const Model & model, const SpecieID t_ID, ParamNum & step_size,  LabelingHolder & holder) {
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
         auto possible_values = model.getTargetVals(t_ID, param_num);

         // pass the function to the holder.
         holder.addRegulatoryFunction(t_ID, step_size, possible_values, source_values);
      }

      // Display stats
      output_streamer.output(verbose_str, "Specie " + model.getName(t_ID) + " has " + toString(tparams.size()) + " regulatory contexts with "
                             + toString(model.getSubcolors(t_ID).size()) + " possible parametrizations out of "
                             + toString(ParametrizationsHelper::getPossibleCount(model.getParameters(t_ID)) + "."));

      // Increase step size for the next function
      step_size *= model.getSubcolors(t_ID).size();
   }

public:
	/**
	 * For each specie recreate all its regulatory functions (all possible labels)
	 */
   static LabelingHolder buildLabeling(const Model & model) {
      LabelingHolder holder;

		ParamNum step_size = 1; // Variable necessary for encoding of colors

		// Cycle through all the species
		for (auto ID:range(model.getSpeciesCount())) {
			// Add specie
         holder.addSpecie(model.getName(ID), ID, model.getRegulatorsIDs(ID));
			
			// Add regulations for this specie
         addRegulations(model, ID, step_size, holder);
		}

		// Set the number by what would be step size for next function
      holder.parameter_count = step_size;

      return holder;
	}
};
#endif // PARSYBONE_LABELING_BUILDER_INCLUDED
