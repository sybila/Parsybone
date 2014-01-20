/*
* Copyright (C) 2012-2013 - Adam Streck
* This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
* ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
* ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
* For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
*/

#ifndef PARSYBONE_LABELING_BUILDER_INCLUDED
#define PARSYBONE_LABELING_BUILDER_INCLUDED

#include "model_translators.hpp"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Parses contrints as specified in the model and return a formula built from these constraints.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ConstraintReader {
	/* Specie can start with a letter or with _ */
	static bool initiatesContext(const char ch) {
		return static_cast<bool>(isalpha(ch)) || ch == '_';
	}

	/* Context may contain specie-characters or number or colon */
	static bool belongsToContext(const char ch) {
		return initiatesContext(ch) || static_cast<bool>(isdigit(ch)) || ch == ':';
	}

	static string addParenthesis(string expr) {
		return string("(" + expr + ")");
	}

	/* Take the original constraint and replace contexts specified with their canonic version. */
	static string formatConstraint(string original, const Model & model, const SpecieID ID) {
		string result;

		size_t start = INF;
		for (const size_t pos : scope(original)) {
			if (start == INF) {
				if (initiatesContext(original[pos])) {
					start = pos;
				}
				else {
					result.push_back(original[pos]);
				}
			}
			else {
				if (!belongsToContext(original[pos])) {
					string context = original.substr(start, pos - start);
					context = ModelTranslators::makeCanonic(model, context, ID);
					result.append(context);
					result.push_back(original[pos]);
					start = INF;
				}
			}
		}

		return result;
	}
public:

	static string readConstraint(const Model & model, const SpecieID ID) {
		string formula; // Resulting formula specifying all the constraints
		
		formula = "tt";

		for (const string constraint : model.species[ID].par_cons) {
			formula.append(" & " + addParenthesis(formatConstraint(constraint, model, ID)));
		}
		
		return addParenthesis(formula);
	}
};
#endif // PARSYBONE_LABELING_BUILDER_INCLUDED
