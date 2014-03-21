/*
* Copyright (C) 2012-2013 - Adam Streck
* This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
* ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
* ParSyBoNe is released without any warranty. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
* For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
*/

#pragma once

#include "../model/model.hpp"
#include "../kinetics/kinetics.hpp"

namespace KineticsTranslators {
	// @return the number of parametrizations
	inline ParamNo getSpaceSize(const Kinetics & kinetics) {
		return accumulate(begin(kinetics.species), end(kinetics.species), static_cast<ParamNo>(1), [](const ParamNo A, const Kinetics::Specie & B) -> ParamNo {
			return A * B.col_count;
		});
	}

	// @return 
	const Levels getSpecieVals(const Kinetics & kinetics, ParamNo number) {
		// Prepare storage vector
		Levels specie_vals(kinetics.species.size());
		auto reverse_val_it = specie_vals.rbegin();

		// Go through colors backwards
		ParamNo divisor = getSpaceSize(kinetics);
		for (auto kin_it = kinetics.species.rbegin(); kin_it != kinetics.species.rend(); kin_it++, reverse_val_it++) {
			// lower divisor value
			divisor /= kin_it->col_count;
			// pick a number for current specie
			*reverse_val_it = static_cast<ActLevel>(number / divisor);
			// take the rest for next round
			number = number % divisor;
		}

		return specie_vals;
	}

	// @return parametrizations string in the form "(val_1,...,val_n)".
	const string createParamString(const Kinetics & kinetics, ParamNo number) {
		// compute numbers of partial parametrizations for each component
		const Levels color_parts = getSpecieVals(kinetics, number);

		string color_str = "(";
		for (const SpecieID ID : cscope(kinetics.species)) {
			for (auto & param : kinetics.species[ID].params) {
				// There may be more contexts than values due to the fact that some are not functional. These are assigned the value -1.
				if (param.functional) 
					color_str += to_string(param.target_in_subcolor[color_parts[ID]]);
				else 
					color_str += "-1";

				color_str += ",";
			}
		}

		// Change the last value
		color_str[color_str.size() - 1]  = ')';
		return color_str;
	}

	/**
	* @return representation of the parametrization used by the database
	*/
	const string makeConcise(const Kinetics::Param & param, const string target_name) {
		string context = "K_" + target_name + "_";
		for (auto values : param.requirements)
			context += to_string(values.second.front());
		return context;
	}

	//TODO TURNED OFF
	set<ParamNo> findMatching(const Kinetics & kinetics, const Levels & param_vals) {
		/*set<ParamNo> matching;
		size_t begin = 0; //< Used to denote current range of the parametrization

		// Test subparametrizations for all species.
		for (const SpecieID ID : cscope(kinetics.species)) {
		vector<ParamNo> submatch;

		// Try to match all the subcolors fo the current specie
		for (const size_t subolor_no : crange(kinetics.species[ID].)) {
		bool valid = true;
		// For the match to occur, all values must either be equal or defined irellevant.
		for (const size_t value_no : cscope(model.species[ID].subcolors[subolor_no])) {
		if (param_vals[value_no + begin] != INF_SHORT && param_vals[value_no + begin] != model.species[ID].subcolors[subolor_no][value_no]) {
		valid = false;
		break;
		}
		}
		if (valid)
		submatch.push_back(subolor_no * model.getStepSize(ID));
		}

		// At least one subparametrization must be found for each specie, if not end.
		if (submatch.empty())
		return set<ParamNo>();

		// Create the results as all possible combinations.
		set<ParamNo> old = matching.empty() ? set<ParamNo>({ 0 }) : move(matching);
		for (const ParamNo o_match : old) {
		for (const ParamNo m_match : submatch) {
		matching.insert(o_match + m_match);
		}
		}

		// Move the beginning for the next specie
		begin += model.species[ID].parameters.size();
		}
		return matching;*/
		return set<ParamNo>{};
	}


	/**
	* @return  the parameter that has the given context

	const Kinetics::Param & matchContext(const Model & model, const Kinetics & kinetics, const string & context, const SpecieID t_ID) {
	const string canonic = ModelTranslators::makeCanonic(model, context, t_ID);
	for (auto & param : kinetics.species[t_ID].params)
	if (param.context.compare(canonic) == 0)
	return param;
	throw runtime_error("Failed to match the context " + context + " for the specie " + to_string(t_ID));
	}*/
}