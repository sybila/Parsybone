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
// All feasible subcolors for each specie are stored with that specie
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../auxiliary/data_types.hpp"
#include "model.hpp"

class ConstrainsParser {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const Model & model;
	
	// Holds all the feasible subcolors for single Specie w.r.t. edge constrains
	struct SpecieColors {
		SpecieID ID;
		std::vector<std::vector<std::size_t> > subcolors; // Feasible subcolors
		std::size_t colors_num; // Total number of subcolors (even those unfesible)

		// Add new subcolor
		void push_back (std::vector<std::size_t> subcolor) {
			subcolors.push_back(subcolor);
		}
	};

	// Store all vectors of subcolors
	std::vector<SpecieColors> colors;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	/**
	 * Tests if given subcolor on given specie can satisfy given requirements
	 *
	 * @param ID	ID of the specie to test contexts in
	 * @param subcolor	unique valuation of all regulatory contexts
	 *
	 * @return	true if the regulation is feasible
	 */
	bool testSubcolor (const SpecieID ID, const std::vector<std::size_t> & subcolor) const {
		// get referecnces to Specie data
		const std::vector<Model::Interaction> & interactions = model.getInteractions(ID);
		const std::vector<Model::Regulation> & regulations = model.getRegulations(ID);
		
		// Cycle through interactions
		for (std::size_t inter_num = 0; inter_num < interactions.size(); inter_num++) {
			// Skip if there are no requirements
			if (interactions[inter_num].constrain == none_cons)
				continue;
			bool is_observable = false;

			// Cycle through regulations
			for (std::size_t regul_num = 0; regul_num < regulations.size(); regul_num++) {
				// Skip if the regulation does not contain requested interaction
				if (!regulations[regul_num].first[inter_num])
					continue;

				else {
					// Copy mask of the regulation
					std::vector<bool> other(regulations[regul_num].first);
					// Turn of tested interaction
					other[inter_num] = false;

					// Cycle through regulations again until you find context just without current interaction
					std::size_t regul_comp = 0;
					while (true) {
						// If context is missing
						if (regul_comp >= regulations.size())
							throw std::runtime_error("Not fount other complementary regulation for some regulation.");
						// If context is found
						if (regulations[regul_comp].first == other)
							break;
						regul_comp++;
					}

					// Test if the requirements are satisfied, if not, return false
					if (((interactions[inter_num].constrain == pos_cons) && (subcolor[regul_num] < subcolor[regul_comp]))
						|| 
						((interactions[inter_num].constrain == neg_cons) && (subcolor[regul_num] > subcolor[regul_comp])))
						return false;

					// Test observability
					if (subcolor[regul_num] != subcolor[regul_comp])
						is_observable = true;
				}
			}

			// Check observability
			if (!is_observable && interactions[inter_num].observable)
				return false;
		}
		return true;
	}
	
	/**
	 * For this specie, test all possible subcolors (all valuations of this specie contexts) and store those that satisfy edge labels.
	 *
	 * @param specie used in this round
	 */
	void createContexts(const SpecieID ID) {
		// Data to fill
		auto regulations = model.getRegulations(ID);
		SpecieColors valid;
		valid.ID = ID;

		// How many to test
		double max_val = static_cast<double>( + 1);
		std::size_t colors_num = 1;

		// Create subcolor with zero values
		std::vector<std::size_t> bottom_color(regulations.size());
		std::vector<std::size_t> top_color(regulations.size());

		for (std::size_t regul_num = 0; regul_num < regulations.size(); regul_num++) {
			if (regulations[regul_num].second < 0) {
				bottom_color[regul_num] = model.getMin(ID);
				top_color[regul_num] = model.getMax(ID);
				colors_num *= (model.getMax(ID) + 1);
			}
			else  {
				bottom_color[regul_num] = top_color[regul_num] = regulations[regul_num].second;
				colors_num *= 1;
			}
		}

		std::vector<std::size_t>  subcolor(bottom_color);
		valid.colors_num = colors_num;

		// Cycle through all possible subcolors for this specie
		for (std::size_t subcolor_num = 0; subcolor_num < colors_num; subcolor_num++) {
			// Test
			if (testSubcolor(ID, subcolor))
				valid.push_back(subcolor);

			// Iterate subcolor
			for (std::size_t regul_num = 0; regul_num < subcolor.size(); regul_num++) {
				// Increase and end
				if (subcolor[regul_num] < top_color[regul_num]) {
					subcolor[regul_num]++;
					break;
				}
				// Null and continue
				else { 
					subcolor[regul_num] = bottom_color[regul_num];
				}
			}
		}

		// Add computed subcolors
		colors.push_back(std::move(valid));
	}

	ConstrainsParser(const ConstrainsParser & other);            // Forbidden copy constructor.
	ConstrainsParser& operator=(const ConstrainsParser & other); // Forbidden assignment operator.

public:
	ConstrainsParser(const Model & _model) : model(_model) { }

	/**
	 * Entry function of parsing, tests and stores subcolors for all the species
	 */
	void parseConstrains() { 
		// Cycle through species
		for (SpecieID ID = 0; ID < model.getSpeciesCount(); ID++) {
			createContexts(ID);
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	/**
	 * @return	total number of species
	 */
	inline const std::size_t getSpecieNum() const {
		return colors.size();
	}	
	
	/**
	 * @return	total number of subcolors this specie could have (all regulatory contexts' combinations)
	 */
	inline const std::size_t getAllColorsNum(const SpecieID ID) const {
		return colors[ID].colors_num;
	}

	/**
	 * @return	total number of subcolors this specie has (allowed regulatory contexts' combinations)
	 */
	inline const std::size_t getColorsNum(const SpecieID ID) const {
		return colors[ID].subcolors.size();
	}

	/**
	 * @return	requested subcolor from the vector of subcolors of given specie
	 */
	inline const std::vector<std::size_t> & getColor(const SpecieID ID, const std::size_t color_num) const {
		return colors[ID].subcolors[color_num];
	}

	/**
	 * @return	total number of subcolors this specie has (allowed regulatory contexts' combinations)
	 */
	inline const std::vector<std::size_t> getTargetVals(const SpecieID ID, const std::size_t regul_num) const {
		//Data to fill
		std::vector<std::size_t> all_target_vals;
		all_target_vals.reserve(colors[ID].subcolors.size());

		// Store values for given regulation
		for (auto color_it = colors[ID].subcolors.begin(); color_it != colors[ID].subcolors.end(); color_it++) {
			all_target_vals.push_back((*color_it)[regul_num]);
		}

		return all_target_vals;
	}
};

#endif