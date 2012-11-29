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
#include "../parsing/formulae_parser.hpp"
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
	 * @param activating	a reference to the variable storing true iff the regulation has observable activating effect
	 * @param inhibiting a reference to the variable storing true iff the regulation has observable inhibiting effect
	 * @param ID	ID of the specie that undergoes the test
    * @param param_num	index of tested parameter
    * @param regul_num	index of regulation whose constrains are tested
	 * @param subcolor	coloring for this specie that is tested
	 *
	 * @return	true if constrains are satisfied
	 */
	void testConstrains(bool & activating, bool & inhibiting, const SpecieID ID, const size_t param_num, const size_t regul_num, const vector<size_t> & subcolor) const {
      // Get reference data
      const vector<Model::Parameter> & parameters = model.getParameters(ID);

      // Copy mask of the regulation and turn of tested regulation
      vector<bool> other(parameters[param_num].first);
      other[regul_num] = false;

      // Cycle through regulations again until you find context just without current regulation
      size_t regul_comp;
      for (regul_comp = 0; regul_comp <= parameters.size(); regul_comp++) {
         // If context is found, break, remembering its number
         if (parameters[regul_comp].first == other)
            break;
		}
		if (regul_comp >= parameters.size())
			throw runtime_error("Not fount other complementary regulation for some regulation.");

		// Assign regulation aspects
		activating |= subcolor[param_num] > subcolor[regul_comp];
		inhibiting |= subcolor[param_num] < subcolor[regul_comp];
	}
	
	/**
	 * Return true if the label (edge constrain) of the regulation is satisfied, false otherwise. All labels can be resolved based only on whether mon+ and mon- are true.
	 * @param	activating	true if the parametrization satisfies +
	 * @param	inhibiting	true if the parametrization satisfies -
	 * @param	label	canonical form of edge label given as a string
	 *
	 * @return	true if the edge constrain is satisfied
	 */
	bool resolveLabel(const bool & activating, const bool & inhibiting, const string label) const {
		// Fill the atomic propositions
		FormulaeParser::Vals values;
		values.insert(FormulaeParser::Val("+", activating));
		values.insert(FormulaeParser::Val("-", inhibiting));

		string formula;

		// Find the constrain and return its valuation
		if (label.compare(Label::Activating) == 0)
			formula = "+";
		else if (label.compare(Label::ActivatingOnly) == 0)
			formula = "(+ & !-)";
		else if (label.compare(Label::Inhibiting) == 0)
			formula = "-";
		else if (label.compare(Label::InhibitingOnly) == 0)
			formula = "(- & !+)";
		else if (label.compare(Label::NotActivating) == 0)
			formula = "!+";
		else if (label.compare(Label::NotInhibiting) == 0)
			formula = "!-";
		else if (label.compare(Label::Observable) == 0)
			formula = "(+ | -)";
		else if (label.compare(Label::NotObservable) == 0)
			formula = "!(+ | -)";
		else
			formula = label;

		return (FormulaeParser::resolve(values, formula));
	}

	/**
	 * Tests if given subparametrization on a given specie can satisfy given requirements.
	 *
	 * @param ID	ID of the specie to test contexts in
	 * @param subcolor	unique valuation of all regulatory contexts
	 *
	 * @return	true if the subparametrization is feasible
	 */
	bool testSubparametrization (const SpecieID ID, const vector<size_t> & subparam) const {
		// get referecnces to Specie data
      const vector<Model::Regulation> & regulations = model.getRegulations(ID);
      const vector<Model::Parameter> & parameters = model.getParameters(ID);
		
		// Cycle through all species's regulators
      for (size_t regul_num = 0; regul_num < regulations.size(); regul_num++) {
         // Skip if there are no requirements (free label)
         if (regulations[regul_num].label.compare(Label::Free) == 0)
            continue;

         // Prepare variables storing info about observable effects of this component
         bool activating = false, inhibiting = false;
         // For each parameter containing the reugulator in parametrization control its satisfaction
         for (size_t param_num = 0; param_num < parameters.size(); param_num++) {
            // Skip if the contexts does not contain requested regulation
            if (!parameters[param_num].first[regul_num])
               continue;

            // Control satisfiability of the basic constrains
            testConstrains(activating, inhibiting, ID, param_num, regul_num, subparam);
         }

			// Test obtained knowledge agains the label itself - return false if the label is not satisfied
			if (!resolveLabel(activating, inhibiting, regulations[regul_num].label))
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
    void testColors(ParametrizationsHolder::SpecieColors && valid, const SpecieID ID, const vector<size_t> & bottom_color, const vector<size_t> & top_color) {
		// Cycle through all possible subcolors for this specie
		vector<size_t> subcolor(bottom_color);

		// Cycle through all colors
		do {
			// Test is it is feasieble, if so, save it
			if (testSubparametrization(ID, subcolor))
				valid.push_back(subcolor);
		} while (iterate(top_color, bottom_color, subcolor));

		if (valid.subcolors.empty())
			throw runtime_error(string("No valid parametrization found for the specie ").append(toString(ID)));

		// Add computed subcolors
		parametrizations.colors.push_back(move(valid));
	}

	/**
	 * @param context regulatory context to test
	 * @param ID	ID of the regulated specie
	 *
	 * @return true if the context denotes self-regulation
	 */
	bool isSelfRegulation(const vector<bool> & context, const StateID ID,  size_t & position) {
		position = INF;
		for (size_t reg_num = 0; reg_num < context.size(); reg_num++) {
			if (context[reg_num]) {
				if (position == INF) {
					position = reg_num;
				}
				else {
					position = INF;
					return false;
				}
			}
		}

		if (position != INF && (model.getRegulations(ID))[ID].source == ID)
			 return true;

		return false;
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
	size_t getBoundaries(const SpecieID ID, vector<size_t> & bottom_color, vector<size_t> & top_color) {
		// Obtain all regulations
		auto parameters = model.getTParams(ID);
		size_t colors_num = 1;
		
		// Cycle through regulations
		for (auto param:parameters) {
			bottom_color.push_back(param.target.front());
			top_color.push_back(param.target.back());
			colors_num *= param.target.size();
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
		vector<size_t> bottom_color, top_color;
      valid.possible_count = getBoundaries(ID, bottom_color, top_color);
		
		// Test all the subcolors and save feasible
      testColors(move(valid), ID, bottom_color, top_color);
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
