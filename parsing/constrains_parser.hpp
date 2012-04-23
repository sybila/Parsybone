/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_CONSTRAINS_PARSER_INCLUDED
#define PARSYBONE_CONSTRAINS_PARSER_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class that computes feasible regulatory functions from edge constrains.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../auxiliary/data_types.hpp"
#include "model.hpp"

class ConstrainsParser {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const Model & model;
	
	struct SpecieColors {
		std::vector<std::vector<std::size_t> > subcolors;
		std::size_t colors_num;

		void push_back (std::vector<std::size_t> subcolor) {
			subcolors.push_back(subcolor);
		}
	};

	std::vector<SpecieColors> colors;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	
	bool testSubcolor (const SpecieID ID, const std::vector<std::size_t> & subcolor) const {
		// get referecnces to Specie data
		const std::vector<Model::Interaction> & interactions = model.getInteractions(ID);
		const std::vector<Model::Regulation> & regulations = model.getRegulations(ID);
		
		for (std::size_t inter_num = 0; inter_num < interactions.size(); inter_num++) {
			if (interactions[inter_num].constrain == none_cons)
				continue;
			for (std::size_t regul_num = 0; regul_num < regulations.size(); regul_num++) {
				if (!regulations[regul_num].first[inter_num])
					continue;
				else {
					std::vector<bool> other(regulations[regul_num].first);
					other[inter_num] = false;
					std::size_t regul_comp = 0;
					while (true) {
						if (regul_comp >= regulations.size())
							throw std::runtime_error("Not fount other complementary regulation for some regulation.");
						if (regulations[regul_comp].first == other)
							break;
						regul_comp++;
					}
					if (((interactions[inter_num].constrain == pos_cons) && (subcolor[regul_num] < subcolor[regul_comp]))
						|| 
						((interactions[inter_num].constrain == neg_cons) && (subcolor[regul_num] > subcolor[regul_comp])))
					return false;
				}
			}
		}
		return true;
	}
	
	void createContexts(const SpecieID ID) {
		// Data to fill
		SpecieColors valid;

		// How many to test
		std::size_t colors_num = model.getRegulations(ID).size() * (model.getMax(ID) + 1);
		valid.colors_num = colors_num;

		// Create subcolor with zero values
		std::vector<std::size_t> subcolor(model.getRegulations(ID).size(), 0);

		// Cycle 
		for (std::size_t subcolor_num = 0; subcolor_num < colors_num; subcolor_num++) {
			// Test
			if (testSubcolor(ID, subcolor))
				valid.push_back(subcolor);

			// Iterate subcolor
			for (std::size_t context_num = 0; context_num < subcolor.size(); context_num++) {
				// Increase and end
				if (subcolor[context_num] < model.getMax(ID)) {
					subcolor[context_num]++;
					break;
				}
				// Null 
				else { 
					subcolor[context_num] = model.getMin(ID);
				}
			}
		}

		// Add
		colors.push_back(std::move(valid));
	}

	ConstrainsParser(const ConstrainsParser & other);            // Forbidden copy constructor.
	ConstrainsParser& operator=(const ConstrainsParser & other); // Forbidden assignment operator.

public:
	ConstrainsParser(const Model & _model) : model(_model) { }

	/**
	 * Entry function of parsing
	 */
	void parseConstrains() { 
		// Cycle through species
		for (SpecieID ID = 0; ID < model.getSpeciesCount(); ID++) {
			createContexts(ID);
		}
	}
};

#endif