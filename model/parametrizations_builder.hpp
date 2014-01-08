/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
*/

#ifndef PARSYBONE_PARAMETRIZATIONS_BUILDER_INCLUDED
#define PARSYBONE_PARAMETRIZATIONS_BUILDER_INCLUDED

#include "../auxiliary/common_functions.hpp"
#include "../auxiliary/formulae_resolver.hpp"
#include "../auxiliary/data_types.hpp"
#include "parametrizations_helper.hpp"
#include "constraint_space.hpp"
#include "model_translators.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Class that computes feasible parametrizations for each specie from
/// the edge constrains and stores them in a ParametrizationHolder object.
///
/// This construction may be optimized by including the warm-start constraint
/// satisfaction.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ParametrizationsBuilder {
	/* Create constraint space on parametrizations for the given specie and enumerate and store all the solutions. */
	static void createKinetics(Model &model, const SpecieID ID) {
		// Build the space
		ConstraintSpace *constraint_space = new ConstraintSpace(model.getParameters(ID).size(), model.getMax(ID));

		// Impose constraints
		size_t param_no = 0;
		for (auto &param : model.getParameters(ID))
			constraint_space->remove_targets(param.targets, param_no++);
		constraint_space->add_edge_cons(model.getRegulations(ID), model.getParameters(ID));

		// Conduct search
		DFS<ConstraintSpace> search(constraint_space);
		delete constraint_space;
		while (ConstraintSpace *space = search.next()) {
			model.species[ID].subcolors.push_back(space->getSolution());
			delete space;
		}
	}

public:
	/**
	 * Entry function of parsing, tests and stores subcolors for all the species.
	 */
	static void buildParametrizations(Model &model) {
		// Cycle through species
		for (SpecieID ID = 0; ID < model.species.size(); ID++) {
			output_streamer.output(verbose_str, "Testing edge constraints for specie: " + to_string(ID + 1) + "/" 
				+ to_string(model.species.size()) + ".", OutputStreamer::no_newl | OutputStreamer::rewrite_ln);
			createKinetics(model, ID);
		}

		output_streamer.clear_line(verbose_str);
		output_streamer.output(verbose_str, "", OutputStreamer::no_out | OutputStreamer::rewrite_ln | OutputStreamer::no_newl);
	}
};

#endif // PARSYBONE_PARAMETRIZATIONS_BUILDER_INCLUDED
