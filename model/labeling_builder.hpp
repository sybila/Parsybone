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
#include "parametrizations_builder.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Creates a labeled graph representation of gene regulatory network and stores it within a LabelingHolder object.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class LabelingBuilder {
   /**
    * This function returns a vector containing target value for a given regulatory contexts for ALL the contexts allowed (in lexicographical order).
    * @param ID	ID of the specie that is regulated
    * @param param_num	ordinal number of the kinetic parameter (in a lexicographical order)
    * @return	vector with a target value for a given specie and regulatory context for each subcolor (parametrization of the single specie)
    */
   static vector<size_t> getTargetVals(const Model & model, const SpecieID ID, const size_t param_num) {
      //Data to fill
      vector<size_t> all_target_vals;
      all_target_vals.reserve(model.species[ID].subcolors.size());

      // Store values for given regulation
      for (const auto & subcolor : model.species[ID].subcolors) {
         all_target_vals.push_back(subcolor[param_num]);
      }

      return all_target_vals;
   }

   /**
    * Creates the kinetic parameters in explicit form from the model information. All feasible parameters for the specie are then stored in the FunctionsStructure.
    * @param ID	ID of the specie to compute the kinetic parameters for
    * @param step_size	number for steps between parametrization change of this specie - this value grows with each successive specie.
    */
   static void addRegulations(Model & model, const SpecieID ID, ParamNum & step_size) {
      // get referecnces to Specie data
      vector<Model::Parameter> & params = model.species[ID].parameters;

      // Go through regulations of a specie - each represents a single function
      for (auto param_no:scope(params)) {
         Configurations source_values;
         // Compute allowed values for each regulating specie for this function to be active
         for (auto source_num:params[param_no].requirements) {
            source_values.push_back(source_num.second);
         }

         // Add target values (if input negative, add all possibilities), if positive, add current requested value
         params[param_no].possible_values = getTargetVals(model, ID, param_no);
         params[param_no].step_size = step_size;
      }

      // Display stats
      output_streamer.output(verbose_str, "Specie " + model.getName(ID) + " has " + toString(params.size()) + " regulatory contexts with "
                             + toString(model.getSubcolors(ID).size()) + " possible parametrizations out of "
                             + toString(ParametrizationsHelper::getPossibleCount(model.getParameters(ID))) + ".");

      // Increase step size for the next function
      step_size *= model.getSubcolors(ID).size();
   }

public:
	/**
	 * For each specie recreate all its regulatory functions (all possible labels)
	 */
   static void buildLabeling(Model & model) {
		ParamNum step_size = 1; // Variable necessary for encoding of colors

		// Cycle through all the species
      for (auto ID:range(model.species.size())) {
         addRegulations(model, ID, step_size);
		}
	}
};
#endif // PARSYBONE_LABELING_BUILDER_INCLUDED
