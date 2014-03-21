/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warranty. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_LABELING_BUILDER_INCLUDED
#define PARSYBONE_LABELING_BUILDER_INCLUDED

#include "../auxiliary/common_functions.hpp"
#include "parametrizations_builder.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief This serves for converting subparametrizations into individual kinetic parameters.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class LabelingBuilder {
	// Get a column of the subparametrization table
	static Levels getTargetVals(const Configurations & subcolors, const size_t param_num) {
		Levels result;
		result.reserve(subcolors.size());

		// Store values for given regulation
		for (auto & subcolor : subcolors)
			result.push_back(subcolor[param_num]);

		return result;
	}

public:
	/**
	 * For each specie recreate all its regulatory functions (all possible labels)
	 */
	static void buildLabeling(const Model & model, Kinetics & kinetics) {
		ParamNo step_size = 1; // Variable necessary for encoding of colors

		// Cycle through all the species
		for (auto ID : crange(model.species.size())) {
			if (model.species[ID].spec_type == Model::Input) continue;
			Kinetics::Specie & specie = kinetics.species[model.species[ID].name];

			// Display stats
			output_streamer.output(verbose_str, "Specie " + model.species[ID].name + " has " + to_string(specie.params.size()) + " parameters with "
				+ to_string(specie.subcolors.size()) + " parametrizations out of " + to_string(model.species[ID].max_value + 1) + " ^ " + to_string(specie.params.size()));

			// Go through regulations of a specie - each represents a single function
			for (auto & param : specie.params) {;

				// Add target values (if input negative, add all possibilities), if positive, add current requested value
				param.second.target_in_subcolor = getTargetVals(specie.subcolors);
			}
		}
	}
};
#endif // PARSYBONE_LABELING_BUILDER_INCLUDED
