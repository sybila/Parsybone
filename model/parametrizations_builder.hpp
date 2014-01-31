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
#include "constraint_reader.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Class that computes feasible parametrizations for each specie from
/// the edge constrains and stores them in a ParametrizationHolder object.
///
/// This construction may be optimized by including the warm-start constraint
/// satisfaction.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ParametrizationsBuilder {
	/* initial constraining of the values to the predefined ones */
	static string addAllowed(const Levels &targets, const string & context) {
		string result = "ff ";

		// Add only present values
		for (const size_t val : targets)
			result += " | " + to_string(val) + " = " + context;

		return result;
	}

	/* Create an expression based on the conditions obtained from the edge label */
	static string replaceInLabel(const string & label, const string & plus, const string & minus) {
		string result = label;

		auto replace = [&result](const char symbol, const string & formula) -> void {
			size_t pos = result.find(symbol);
			while (pos != result.npos) {
				result.replace(pos, 1, formula);
				pos = result.find(symbol);
			}
		};

		replace('+', plus);
		replace('-', minus);

		return result;
	}

	/* For each regulation create a constraint corresponding to its label */
	static void createEdgeCons(const vector<Model::Regulation> & reguls, const vector<Model::Parameter> & params, Model::Regulation & regul, string & plus, string & minus) {
		plus = minus = "ff ";
		for (const size_t param_no : cscope(params)) {
			if (ParametrizationsHelper::containsRegulation(params[param_no], regul)) {
				for (const size_t compare_no : cscope(params)) {
					if (ParametrizationsHelper::isSubordinate(reguls, params[param_no], params[compare_no], regul.source)) {
						plus += " | " + params[param_no].context + " > " + params[compare_no].context;
						minus += " | " + params[param_no].context + " < " + params[compare_no].context;
					}
				}
			}
		}
	}

	static void addParenthesis(string & formula) {
		formula = "(" + formula + ")";
	}

	static string createFormula(const Model &model, const SpecieID ID) {
		string result = "tt ";

		for (Model::Regulation regul : model.getRegulations(ID)) {
			string plus, minus, label; 
			createEdgeCons(model.getRegulations(ID), model.getParameters(ID), regul, plus, minus);
			addParenthesis(plus);
			addParenthesis(minus);
			label = RegulationHelper::getLabel(regul.label);
			label = replaceInLabel(label, plus, minus);
			addParenthesis(label);
			result += " & " + label;
		}

		for (Model::Parameter param : model.getParameters(ID)) {
			string allowed;
			allowed = addAllowed(param.targets, param.context);
			addParenthesis(allowed);
			result += " & " + allowed;
		}
		
		return result;
	}

	/* Create constraint space on parametrizations for the given specie and enumerate and store all the solutions. */
	static void createKinetics(const SpecieID ID, const string formula, Model & model) {
		// Build the space
		vector<string> names;
		for (const Model::Parameter & param : model.getParameters(ID))
			names.push_back(param.context);
		
		ConstraintParser * cons_pars = new ConstraintParser(names.size(), model.getMax(ID));

		// Impose constraints
		cons_pars->applyFormula(names, formula);

		// Conduct search
		DFS<ConstraintParser> search(cons_pars);
		delete cons_pars;
		while (ConstraintParser *result = search.next()) {
			model.species[ID].subcolors.push_back(result->getSolution());
			delete result;
		}
	}

public:
	/**
	 * Entry function of parsing, tests and stores subcolors for all the species.
	 */
	static void buildParametrizations(Model &model) {
		// Cycle through species
		for (SpecieID ID = 0; ID < model.species.size(); ID++) {
			output_streamer.output(verbose_str, "Testing edge constraints for Specie: " + to_string(ID + 1) + "/"
				+ to_string(model.species.size()) + ".", OutputStreamer::no_newl | OutputStreamer::rewrite_ln);
			string formula = createFormula(model, ID) + " & " + ConstraintReader::consToFormula(model, ID);
			createKinetics(ID, formula, model);
		}

		output_streamer.clear_line(verbose_str);
		output_streamer.output(verbose_str, "", OutputStreamer::no_out | OutputStreamer::rewrite_ln | OutputStreamer::no_newl);
	}
};

#endif // PARSYBONE_PARAMETRIZATIONS_BUILDER_INCLUDED
