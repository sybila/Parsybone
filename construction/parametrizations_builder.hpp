/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_PARAMETRIZATIONS_BUILDER_INCLUDED
#define PARSYBONE_PARAMETRIZATIONS_BUILDER_INCLUDED

#include "../auxiliary/data_types.hpp"
#include "../parsing/model.hpp"
#include "parametrizations_holder.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Class that computes feasible parametrizations for each specie from edge constrains.
/// @attention subcolor means partial parametrizatrization ~ full parametrization of a single specie
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ParametrizationsBuilder {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const Model & model; ///< Model that is referenced
	ParametrizationsHolder & parametrizations; ///< Holder of parametrizations that will be filled.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TESTING FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	/** 
	 * Test specific constrain on given color - this function checks both observability and the edge constrain
	 *
	 * @param is_observable	stores true if the regulation is observable
	 * @param ID	ID of the specie that undergoes the test
     * @param param_num	index of tested parameter
     * @param regul_num	index of regulation whose constrains are tested
	 * @param subcolor	coloring for this specie that is tested
	 *
	 * @return	true if constrains are satisfied
	 */
    bool testConstrains(bool & is_observable, const SpecieID ID, const std::size_t param_num, const std::size_t regul_num, const std::vector<std::size_t> & subcolor) const {
		// Get reference data
        const std::vector<Model::Regulation> & regulations = model.getRegulations(ID);
        const std::vector<Model::Parameter> & parameters = model.getParameters(ID);

        // Copy mask of the regulation and turn of tested regulation
        std::vector<bool> other(parameters[param_num].first);
        other[regul_num] = false;

        // Cycle through regulations again until you find context just without current regulation
		std::size_t regul_comp = 0;
		while (true) {
			// If context is missing
            if (regul_comp >= parameters.size())
				throw std::runtime_error("Not fount other complementary regulation for some regulation.");
			// If context is found
            if (parameters[regul_comp].first == other)
				break;
			regul_comp++;
		}

		// Test observability
        is_observable = is_observable | (subcolor[param_num] != subcolor[regul_comp]);

		// Test if the requirements are satisfied, if not, return false
        if (((regulations[regul_num].constrain == pos_cons) && (subcolor[param_num] < subcolor[regul_comp]))
			|| 
            ((regulations[regul_num].constrain == neg_cons) && (subcolor[param_num] > subcolor[regul_comp])))
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
        const std::vector<Model::Regulation> & regulations = model.getRegulations(ID);
        const std::vector<Model::Parameter> & parameters = model.getParameters(ID);
		
        // Cycle through regulation
        for (std::size_t regul_num = 0; regul_num < regulations.size(); regul_num++) {
			// Skip if there are no requirements
            if (regulations[regul_num].constrain == none_cons && !regulations[regul_num].observable)
				continue;
			bool is_observable = false;

			// Cycle through regulations and test constrains
            for (std::size_t param_num = 0; param_num < parameters.size(); param_num++) {
                // Skip if the regulation does not contain requested regulation
                if (!parameters[param_num].first[regul_num])
					continue;

				// Test contrains and return false, if sign constrain is not satisfied
                if(!testConstrains(is_observable, ID, param_num, regul_num, subcolor))
					return false;
			}

			// Check observability, if it is required
            if (!is_observable && regulations[regul_num].observable)
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
	void testColors(ParametrizationsHolder::SpecieColors && valid, const SpecieID ID, const std::size_t colors_num, const std::vector<std::size_t> & bottom_color, const std::vector<std::size_t> & top_color) {
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
		parametrizations.colors.push_back(std::move(valid));
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
        auto parameters = model.getParameters(ID);
		std::size_t colors_num = 1;
		
		// Cycle through regulations
        for (std::size_t regul_num = 0; regul_num < parameters.size(); regul_num++) {
			// If the target value is parametrized, add all the values
            if (parameters[regul_num].second < 0) {
				bottom_color[regul_num] = model.getMin(ID);
				top_color[regul_num] = model.getMax(ID);
				colors_num *= (model.getMax(ID) + 1);
			}
			// Otherwise add just given value
			else  {
                bottom_color[regul_num] = top_color[regul_num] = parameters[regul_num].second;
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
		ParametrizationsHolder::SpecieColors valid;
		valid.ID = ID;

		// Reference data
        auto parameters = model.getParameters(ID);
		std::size_t colors_num; // How many colors will be tested (number of all combinations)
		
		// Create boundaries for iteration
        std::vector<std::size_t> bottom_color(parameters.size());
        std::vector<std::size_t> top_color(parameters.size());
		valid.possible_count = colors_num = getBoundaries(ID, bottom_color, top_color);
		
		// Test all the subcolors and save feasible
		testColors(std::move(valid), ID, colors_num, bottom_color, top_color);
	}

	ParametrizationsBuilder(const ParametrizationsBuilder & other); ///< Forbidden copy constructor.
	ParametrizationsBuilder& operator=(const ParametrizationsBuilder & other); ///< Forbidden assignment operator.

public:
	ParametrizationsBuilder(const Model & _model, ParametrizationsHolder & _parametrizations)
		: model(_model), parametrizations(_parametrizations) { } ///< Empty default constructor

	/**
	 * Entry function of parsing, tests and stores subcolors for all the species
	 */
	void buildParametrizations() {
		output_streamer.output(verbose_str, "Creating the parametrization space. ");

		// Cycle through species
		for (SpecieID ID = 0; ID < model.getSpeciesCount(); ID++) {
			createKinetics(ID);
		}
	}
};

#endif // PARSYBONE_PARAMETRIZATIONS_BUILDER_INCLUDED
