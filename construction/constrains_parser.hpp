/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_CONSTRAINS_PARSER_INCLUDED
#define PARSYBONE_CONSTRAINS_PARSER_INCLUDED

#include "../auxiliary/data_types.hpp"
#include "../parsing/model.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Class that computes feasible parametrizations for each specie from edge constrains.
/// All feasible subcolors for each specie are stored with that specie.
/// @attention subcolor means partial parametrizatrization ~ full parametrization of a single specie
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ConstrainsParser {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const Model & model; ///< Model that is referenced
	
	/// Holds all the feasible subcolors for single Specie w.r.t. edge constrains
	struct SpecieColors {
		SpecieID ID; ///< Unique ID of the specie
		std::vector<std::vector<std::size_t> > subcolors; ///< Feasible subcolors of the specie
		std::size_t colors_num; ///< Total number of subcolors possible for the specie(even those unfesible)

		/// Add as new subcolor
		void push_back (std::vector<std::size_t> subcolor) {
			subcolors.push_back(subcolor);
		}
	};

	/// Store all vectors of subcolors
	std::vector<SpecieColors> colors;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TESTING FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	/** 
	 * Test specific constrain on given color - this function checks both observability and the edge constrain
	 *
	 * @param is_observable	stores true if the regulation is observable
	 * @param ID	ID of the specie that undergoes the test
	 * @param regul_num	index of tested regulation
	 * @param inter_num	index of interaction whose constrains are tested
	 * @param subcolor	coloring for this specie that is tested
	 *
	 * @return	true if constrains are satisfied
	 */
	bool testConstrains(bool & is_observable, const SpecieID ID, const std::size_t regul_num, const std::size_t inter_num, const std::vector<std::size_t> & subcolor) const {
		// Get reference data
		const std::vector<Model::Interaction> & interactions = model.getInteractions(ID);
		const std::vector<Model::Regulation> & regulations = model.getRegulations(ID);

		// Copy mask of the regulation and turn of tested interaction
		std::vector<bool> other(regulations[regul_num].first);
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

		// Test observability
		is_observable = is_observable | (subcolor[regul_num] != subcolor[regul_comp]);

		// Test if the requirements are satisfied, if not, return false
		if (((interactions[inter_num].constrain == pos_cons) && (subcolor[regul_num] < subcolor[regul_comp]))
			|| 
			((interactions[inter_num].constrain == neg_cons) && (subcolor[regul_num] > subcolor[regul_comp])))
			return false;

		return true;	
	}
	
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
			if (interactions[inter_num].constrain == none_cons && !interactions[inter_num].observable)
				continue;
			bool is_observable = false;

			// Cycle through regulations and test constrains
			for (std::size_t regul_num = 0; regul_num < regulations.size(); regul_num++) {
				// Skip if the regulation does not contain requested interaction
				if (!regulations[regul_num].first[inter_num])
					continue;

				// Test contrains and return false, if sign constrain is not satisfied
				if(!testConstrains(is_observable, ID, regul_num, inter_num, subcolor))
					return false;
			}

			// Check observability, if it is required
			if (!is_observable && interactions[inter_num].observable)
				return false;
		}

		// If everything has passed, return true
		return true;
	}

	/**
	 * Test all possible subcolors and saves valid
	 *
	 * @param valid	data storage to save in
	 * @param ID	ID of currently used specie
	 * @param colors_num	how many colors are there together
	 * @param bottom_color	low bound on possible contexts
	 * @param top_color	top bound on possible contexts
	 */
	void testColors(SpecieColors && valid, const SpecieID ID, const std::size_t colors_num, const std::vector<std::size_t> & bottom_color, const std::vector<std::size_t> & top_color) {
		// Cycle through all possible subcolors for this specie
		std::vector<std::size_t> subcolor(bottom_color);

		// Cycle through all colors
		for (std::size_t subcolor_num = 0; subcolor_num < colors_num; subcolor_num++) {
			// Test is it is feasieble, if so, save it
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	/**
	 * Compute and store boundaries on possible context values - used for iterations
	 *
	 * @param ID	ID of currently used specie
	 * @param bottom_color	low bound on possible contexts
	 * @param top_color	top bound on possible contexts
	 *
	 * @return	how many colors are there together
	 */
	const std::size_t getBoundaries(const SpecieID ID, std::vector<std::size_t> & bottom_color, std::vector<std::size_t> & top_color) {
		// Obtain all regulations
		auto regulations = model.getRegulations(ID);
		std::size_t colors_num = 1;
		
		// Cycle through regulations
		for (std::size_t regul_num = 0; regul_num < regulations.size(); regul_num++) {
			// If the target value is parametrized, add all the values
			if (regulations[regul_num].second < 0) {
				bottom_color[regul_num] = model.getMin(ID);
				top_color[regul_num] = model.getMax(ID);
				colors_num *= (model.getMax(ID) + 1);
			}
			// Otherwise add just given value
			else  {
				bottom_color[regul_num] = top_color[regul_num] = regulations[regul_num].second;
				colors_num *= 1;
			}
		}

		return colors_num;
	}

	/**
	 * For this specie, test all possible subcolors (all valuations of this specie contexts) and store those that satisfy edge labels.
	 *
	 * @param specie used in this round
	 */
	void createKinetics(const SpecieID ID) {
		// Data to fill
		SpecieColors valid;
		valid.ID = ID;

		// Reference data
		auto regulations = model.getRegulations(ID);
		std::size_t colors_num; // How many colors will be tested (number of all combinations)
		
		// Create boundaries for iteration
		std::vector<std::size_t> bottom_color(regulations.size());
		std::vector<std::size_t> top_color(regulations.size());
		valid.colors_num = colors_num = getBoundaries(ID, bottom_color, top_color);
		
		// Test all the subcolors and save feasible
		testColors(std::move(valid), ID, colors_num, bottom_color, top_color);
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
			createKinetics(ID);
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
	 * @param ID	ID of the specie to get the number from
	 *
	 * @return	total number of subcolors this specie could have (all regulatory contexts' combinations)
	 */
	inline const std::size_t getAllColorsNum(const SpecieID ID) const {
		return colors[ID].colors_num;
	}

	/**
	 * @param ID	ID of the specie to get the number from
	 *
	 * @return	total number of subcolors this specie has (allowed regulatory contexts' combinations)
	 */
	inline const std::size_t getColorsNum(const SpecieID ID) const {
		return colors[ID].subcolors.size();
	}

	/**
	 * @param ID	ID of the specie the requested subcolor belongs to
	 * @param color_num	ordinal number of the requested subcolor
	 *
	 * @return	requested subcolor from the vector of subcolors of given specie
	 */
	inline const std::vector<std::size_t> & getColor(const SpecieID ID, const std::size_t color_num) const {
		return colors[ID].subcolors[color_num];
	}

	/**
	 * This function returns a vector containing target value for a given regulatory contexts for ALL the contexts allowed (in lexicographical order).
	 *
	 * @param ID	ID of the specie that is regulated
	 * @param regul_num	ordinal number of the regulatory context (in a lexicographical order)
	 *
	 * @return	vector with a target value for a given specie and regulatory context for each subcolor (parametrization of the single specie)
	 */
	const std::vector<std::size_t> getTargetVals(const SpecieID ID, const std::size_t regul_num) const {
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
