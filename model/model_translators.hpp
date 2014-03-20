/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warranty. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef MODEL_TRANSLATORS_HPP
#define MODEL_TRANSLATORS_HPP

#include "model.hpp"

namespace ModelTranslators {
	/**
	 * @brief findID  obtain ID of the specie.
	 */
	SpecieID findID(const Model & model, const string & name) {
		for (auto & spec : model.species)
			if (spec.name.compare(name) == 0)
				return spec.ID;
		return INF;
	}

	/**
	 * @return	unique IDs of regulators of the specie
	 */
	vector<SpecieID> getRegulatorsIDs(const Model & model, const SpecieID ID) {
		set<SpecieID> IDs;
		for (auto regul : model.species[ID].regulations) {
			IDs.insert(regul.source);
		}
		return vector<SpecieID>(IDs.begin(), IDs.end());
	}

	/**
	 * @return	names of the regulators of the specie
	 */
	vector<string> getRegulatorsNames(const Model & model, const SpecieID ID) {
		auto regulators = getRegulatorsIDs(model, ID);
		vector<string> names;
		for (auto reg : regulators) {
			names.push_back(model.species[reg].name);
		}
		return names;
	}

	/**
	* @return	names of all the species
	*/
	vector<string> getAllNames(const Model & model) {
		vector<string> names;
		for (const Model::ModelSpecie & specie : model.species)
			names.push_back(specie.name);
		return names;
	}

	/**
	 * @brief getThresholds Finds a list of thresholds for each regulator of a given component.
	 * @param ID
	 * @return
	 */
	map<SpecieID, Levels > getThresholds(const Model & model, const SpecieID ID) {
		map<SpecieID, Levels > thresholds;
		for (auto reg : model.species[ID].regulations) {
			auto key = thresholds.find(reg.source);
			if (key == thresholds.end()) {
				thresholds.insert(make_pair(reg.source, Levels(1, reg.threshold)));
			}
			else {
				key->second.push_back(reg.threshold);
			}
		}

		for (auto ths : thresholds) {
			sort(ths.second.begin(), ths.second.end());
		}

		return thresholds;
	}

	/**
	 * @brief getSpaceSize
	 */
	ParamNo getSpaceSize(const Model & model) {
		ParamNo space_size = 1;
		for (const Model::ModelSpecie & spec : model.species)
			space_size *= spec.subcolors.size();

		return space_size;
	}

	/**
	 * @brief getSpecieVals
	 */
	const Levels getSpecieVals(const Model & model, ParamNo number) {
		// Prepare storage vector
		Levels specie_vals(model.species.size());
		auto reverse_val_it = specie_vals.rbegin();

		// Go through colors backwards
		ParamNo divisor = getSpaceSize(model);
		for (auto specie_it = model.species.rbegin(); specie_it != model.species.rend(); specie_it++, reverse_val_it++) {
			// lower divisor value
			divisor /= specie_it->subcolors.size();
			// pick a number for current specie
			*reverse_val_it = static_cast<ActLevel>(number / divisor);
			// take the rest for next round
			number = number % divisor;
		}

		return specie_vals;
	}

	/**
	 * @brief createColorString
	 */
	const string createParamString(const Model & model, ParamNo number) {
		// compute numbers of partial parametrizations for each component
		const Levels color_parts = getSpecieVals(model, number);

		string color_str = "(";
		// cycle through the species
		for (const SpecieID ID : crange(model.species.size())) {
			auto color = model.species[ID].subcolors[color_parts[ID]];
			auto color_it = color.begin();
			// fill partial parametrization of the specie
			for (const Model::Parameter param : model.species[ID].parameters) {
				// There may be more contexts than values due to the fact that some are not functional. These are assigned the value -1.
				if (param.functional) {
					color_str += to_string(*color_it);
					color_it++;
				}
				else {
					color_str += "-1";
				}
				
				color_str += ",";
			}
		}
		// Change the last value
		*(color_str.end() - 1) = ')';

		return color_str;
	}

	/**
	 * @return representation of the parametrization used by the database
	 */
	const string makeConcise(const Model::Parameter & param, const string target_name) {
		string context = "K_" + target_name + "_";
		for (auto values : param.requirements)
			context += to_string(values.second.front());
		return context;
	}

	/**
	 * @return numbers of all parametrizations that match the givene values in all the values that overlap
	 */
	set<ParamNo> findMatching(const Model & model, const Levels & param_vals) {
		set<ParamNo> matching;
		size_t begin = 0; //< Used to denote current range of the parametrization

		// Test subparametrizations for all species.
		for (const SpecieID ID : cscope(model.species)) {
			vector<ParamNo> submatch;

			// Try to match all the subcolors fo the current specie
			for (const size_t subolor_no : cscope(model.species[ID].subcolors)) {
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
		return matching;
	}

	/**
	 * @brief getThreshold  For a given regulator, find out what it's threshold in the given context is.
	 * @return  threshold value in the given context
	 */
	ActLevel getThreshold(const Model & model, const string & context, const SpecieID t_ID, const string & name, const size_t pos) {
		// Regulator not present.
		if (pos == context.npos)
			return 0;
		const size_t COLON_POS = pos + name.length(); // Where colon is supposed to be.
		auto thresholds = getThresholds(model, t_ID).find(findID(model, name))->second;

		// Regulator level not specified.
		if (context[COLON_POS] != ':') {
			// Control if the context is unambiguous.
			if (thresholds.size() > 1)
				throw runtime_error("Ambiguous context \"" + context + "\" - no threshold specified for a regulator " + name + " that has multiple regulations.");
			// If valid, add the threshold 1.
			return thresholds[0];
		}

		// There is not a threshold given after double colon.
		if (context[COLON_POS] == ':' && (COLON_POS == (context.npos - 1) || !isdigit(context[COLON_POS + 1])))
			throw runtime_error("No threshold given after colon in the context \"" + context + "\" of the regulator " + name);

		// Add a threshold if uniquely specified.
		string to_return;
		size_t number = 1;
		// Copy all the numbers
		while (isdigit(context[COLON_POS + number])) {
			to_return.push_back(context[COLON_POS + number]);
			number++;
		}
		// Check if the threshold is valid
		size_t thrs = boost::lexical_cast<size_t>(to_return);
		if (thrs != 0 && find(thresholds.begin(), thresholds.end(), thrs) == thresholds.end())
			throw runtime_error("The threshold value \"" + to_return + "\" is not valid for the context \"" + context + "\".");

		return thrs;
	}

	/**
	 * @brief formCanonicContext   Transforms the regulation specification into a canonic form (\forall r \in regulator [r:threshold,...]).
	 * @param context any valid context form as a string
	 * @return canonic context form
	 */
	static string makeCanonic(const Model & model, const string & context, const SpecieID t_ID) {
		string new_context; // new canonic form
		const auto names = getRegulatorsNames(model, t_ID);

		// Control correctness
		vector<string> reguls;
		split(reguls, context, is_any_of(","));
		for (const string regul : reguls) {
			string spec_name = regul.substr(0, min(regul.find(':'), regul.size()));
			if (find(names.begin(), names.end(), spec_name) == names.end())
				throw runtime_error("Unrecognized specie \"" + spec_name + "\" in the context \"" + context + "\".");
		}

		// For each of the regulator of the specie.
		for (const auto & name : names) {
			auto pos = context.find(name);
			ActLevel threshold = getThreshold(model, context, t_ID, name, pos);
			new_context += name + ":" + to_string(threshold) + ",";
		}

		// Remove the last comma and return
		return new_context.substr(0, new_context.length() - 1);
	}

	/**
	 * @return  the parameter that has the given context
	 */
	const Model::Parameter & matchContext(const Model & model, const string & context, const SpecieID t_ID) {
		const string canonic = makeCanonic(model, context, t_ID);
		for (const Model::Parameter & param : model.getParameters(t_ID))
			if (param.context.compare(canonic) == 0)
				return param;
		throw runtime_error("Failed to match the context " + context + " for the specie " + to_string(t_ID));
	}

	/**
	 * @return regulation with given parameters
	 */
	const Model::Regulation & findRegulation(const Model & model, const SpecieID t_ID, const SpecieID s_ID, const ActLevel threshold) {
		const auto & reguls = model.species[t_ID].regulations;
		for (const Model::Regulation & regul : reguls)
			if (regul.source == s_ID && regul.threshold == threshold)
				return regul;
		throw runtime_error("Failed to match the regulation " + to_string(s_ID) + " -" + to_string(threshold) + "-> " + to_string(t_ID));
	}

	// @return the maximal level in between the species
	const ActLevel getMaxLevel(const Model & model) {
		return rng::max_element(model.species, [](const Model::ModelSpecie & A, const Model::ModelSpecie & B) {
			return A.max_value < B.max_value;
		})->max_value;
	}
}
#endif // MODEL_TRANSLATORS_HPP
