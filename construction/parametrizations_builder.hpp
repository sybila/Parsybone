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
#include "../auxiliary/common_functions.hpp"
#include "../parsing/model.hpp"
#include "parametrizations_holder.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Class that computes feasible parametrizations for each specie from edge constrains and stores them in a ParametrizationHolder object.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ParametrizationsBuilder {
   const Model & model; ///< Model that is referenced.
	ParametrizationsHolder & parametrizations; ///< Holder of parametrizations that will be filled.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TESTING METHODS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	/** 
    * Test specific constrain on given color - this function checks both observability and the edge constrain.
	 *
    * @param is_observable stores true if the regulation is observable
	 * @param ID	ID of the specie that undergoes the test
    * @param param_num	index of tested parameter
    * @param regul_num	index of regulation whose constrains are tested
	 * @param subcolor	coloring for this specie that is tested
	 *
	 * @return	true if constrains are satisfied
	 */
	void testConstrains(bool & mon_plus, bool & mon_minus, const SpecieID ID, const std::size_t param_num, const std::size_t regul_num, const std::vector<std::size_t> & subcolor) const {
      // Get reference data
      const std::vector<Model::Parameter> & parameters = model.getParameters(ID);

      // Copy mask of the regulation and turn of tested regulation
      std::vector<bool> other(parameters[param_num].first);
      other[regul_num] = false;

      // Cycle through regulations again until you find context just without current regulation
      std::size_t regul_comp;
      for (regul_comp = 0; regul_comp <= parameters.size(); regul_comp++) {
         // If context is found, break, remembering its number
         if (parameters[regul_comp].first == other)
            break;
		}
		if (regul_comp >= parameters.size())
			throw std::runtime_error("Not fount other complementary regulation for some regulation.");

		// Assign monotonicity values
		mon_plus &= subcolor[param_num] >= subcolor[regul_comp];
		mon_minus &= subcolor[param_num] <= subcolor[regul_comp];
	}
	
	/**
	 * Return true if the label (edge constrain) of the regulation is satisfied, false otherwise. All labels can be resolved based only on whether mon+ and mon- are true.
	 * @param	mon_plus	true if the parametrization satisfies mon+
	 * @param	mon_minus	true if the parametrization satisfies mon-
	 * @param	label	canonical form of edge label given as a string
	 *
	 * @return	true if the edge constrain is satisfied
	 */
	bool resolveLabel(const bool & mon_plus, const bool & mon_minus, const std::string label) const {
		// Define further constants
		const bool obs_plus = !mon_minus;
		const bool obs_minus = !mon_plus;

		// Find the constrain and return its valuation
		if (label.compare(Label::mon_plus) == 0)
			return mon_plus;
		else if (label.compare(Label::mon_minus) == 0)
			return mon_minus;
		else if (label.compare(Label::mon) == 0)
			return mon_plus | mon_minus;
		else if (label.compare(Label::obs_plus) == 0)
			return obs_plus;
		else if (label.compare(Label::obs_minus) == 0)
			return obs_minus;
		else if (label.compare(Label::obs) == 0)
			return obs_plus | obs_minus;
		else if (label.compare(Label::plus) == 0)
			return obs_plus & mon_plus;
		else if (label.compare(Label::minus) == 0)
			return obs_minus & mon_minus;
		else if (label.compare(Label::plus_minus) == 0)
			return obs_plus & obs_minus;
		else {
			throw std::invalid_argument("resolveLabel(" + toString(mon_plus) + ", " + toString(mon_minus) + ", " + label + ") failed");
			return false;
		}
	}

	/**
	 * Tests if given subparametrization on a given specie can satisfy given requirements.
	 *
	 * @param ID	ID of the specie to test contexts in
	 * @param subcolor	unique valuation of all regulatory contexts
	 *
	 * @return	true if the subparametrization is feasible
	 */
	bool testSubparametrization (const SpecieID ID, const std::vector<std::size_t> & subparam) const {
		// get referecnces to Specie data
      const std::vector<Model::Regulation> & regulations = model.getRegulations(ID);
      const std::vector<Model::Parameter> & parameters = model.getParameters(ID);
		
		// Cycle through all species's regulators
      for (std::size_t regul_num = 0; regul_num < regulations.size(); regul_num++) {
         // Skip if there are no requirements (free label)
         if (regulations[regul_num].label.compare(Label::free) == 0)
            continue;

         // Set up initial satisfiability
         bool mon_plus = true, mon_minus = true;
         // For each parameter containing the reugulator in parametrization control its satisfaction
         for (std::size_t param_num = 0; param_num < parameters.size(); param_num++) {
            // Skip if the contexts does not contain requested regulation
            if (!parameters[param_num].first[regul_num])
               continue;

            // Control satisfiability of the basic constrains
            testConstrains(mon_plus, mon_minus, ID, param_num, regul_num, subparam);
         }

			// Test obtained knowledge agains the label itself - return false if the label is not satisfied
			if (!resolveLabel(mon_plus, mon_minus, regulations[regul_num].label))
				return false;
		}

		// If everything has passed, return true
		return true;
	}

	/**
    * Test all possible subcolors and saves valid.
	 *
	 * @param valid	data storage to save in
	 * @param ID	ID of currently used specie
	 * @param colors_num	how many colors are there together
	 * @param bottom_color	low bound on possible contexts
	 * @param top_color	top bound on possible contexts
	 */
    void testColors(ParametrizationsHolder::SpecieColors && valid, const SpecieID ID, const std::vector<std::size_t> & bottom_color, const std::vector<std::size_t> & top_color) {
		// Cycle through all possible subcolors for this specie
		std::vector<std::size_t> subcolor(bottom_color);

		// Cycle through all colors
		do {
			// Test is it is feasieble, if so, save it
			if (testSubparametrization(ID, subcolor))
				valid.push_back(subcolor);
		} while (iterate(top_color, bottom_color, subcolor));

		if (valid.subcolors.empty())
			throw std::runtime_error(std::string("No valid parametrization found for the specie ").append(toString(ID)));

		// Add computed subcolors
		parametrizations.colors.push_back(std::move(valid));
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION METHODS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	/**
    * Compute and store boundaries on possible context values - used for iterations.
	 *
	 * @param ID	ID of currently used specie
	 * @param bottom_color	low bound on possible contexts
	 * @param top_color	top bound on possible contexts
	 *
	 * @return	how many colors are there together
	 */
	std::size_t getBoundaries(const SpecieID ID, std::vector<std::size_t> & bottom_color, std::vector<std::size_t> & top_color) {
		// Obtain all regulations
      auto parameters = model.getParameters(ID);
		std::size_t colors_num = 1;
		
		// Cycle through regulations
      for (std::size_t regul_num = 0; regul_num < parameters.size(); regul_num++) {
         // If the target value is unknown, add all the values
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
		
		// Create boundaries for iteration
      std::vector<std::size_t> bottom_color(parameters.size());
      std::vector<std::size_t> top_color(parameters.size());
      valid.possible_count = getBoundaries(ID, bottom_color, top_color);
		
		// Test all the subcolors and save feasible
      testColors(std::move(valid), ID, bottom_color, top_color);
	}

	ParametrizationsBuilder(const ParametrizationsBuilder & other); ///< Forbidden copy constructor.
	ParametrizationsBuilder& operator=(const ParametrizationsBuilder & other); ///< Forbidden assignment operator.

public:
	ParametrizationsBuilder(const Model & _model, ParametrizationsHolder & _parametrizations)
      : model(_model), parametrizations(_parametrizations) { } ///< Empty default constructor.

	/**
    * Entry function of parsing, tests and stores subcolors for all the species.
	 */
	void buildParametrizations() {
      output_streamer.output(verbose_str, "Creating the parametrization space.");

		// Cycle through species
		for (SpecieID ID = 0; ID < model.getSpeciesCount(); ID++) {
			createKinetics(ID);
		}
	}
};

#endif // PARSYBONE_PARAMETRIZATIONS_BUILDER_INCLUDED
