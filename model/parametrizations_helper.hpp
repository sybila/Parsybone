/*
* Copyright (C) 2012-2013 - Adam Streck
* This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
* ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
* ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
* For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
*/

#ifndef PARAMETRIZATIONS_HELPER_HPP
#define PARAMETRIZATIONS_HELPER_HPP

#include "../model/model_translators.hpp"
#include "../auxiliary/data_types.hpp"
#include "../auxiliary/formulae_resolver.hpp"

class ParametrizationsHelper {

public:
	/**
	 * @brief isSubordinate returns true if the current context is the same as the
	 * compared context only with a higher activity value in specificed regulator.
	 */
	static bool isSubordinate(const vector<Model::Regulation> &reguls, const Model::Parameter &current, const Model::Parameter &compare, const SpecieID source_ID) {
		for (const Model::Regulation & regul : reguls) {
			const SpecieID regul_ID = regul.source;
			// All the regulations must have the same requirements, except for the one
			// with the specified source, which must connect on the value.
			if (regul_ID != source_ID) {
				if (current.requirements.find(regul_ID)->second.back() != compare.requirements.find(regul_ID)->second.back()) {
					return false;
				}
			}
			else if (current.requirements.find(regul_ID)->second.front() != compare.requirements.find(regul_ID)->second.back() + 1) {
				return false;
			}

		}
		return true;
	}

	/**
	 * Return true if the given parameter's context is dependent on the given regulation.
	 */
	static bool containsRegulation(const Model::Parameter &param, const Model::Regulation &regul) {
		return param.requirements.find(regul.source)->second.front() == regul.threshold;
	}

	static size_t contextsWithRegul(const vector<Model::Parameter> &params, const Model::Regulation &regul) {
		size_t count = 0;
		for (const Model::Parameter & param : params)
			count += containsRegulation(param, regul);
		return count;
	}

	/**
	 * Compute and store boundaries on possible context values - used for
	 * iterations.
	 */
	static void getBoundaries(const Model::Parameters &params,
		Levels &bottom_color, Levels &top_color) {
		for (auto param : params) {
			bottom_color.push_back(param.targets.front());
			top_color.push_back(param.targets.back());
		}
	}

	/**
	 * @brief getPossibleCount compute the number of possible colors
	 */
	static ParamNo getPossibleCount(const Model::Parameters &params) {
		ParamNo colors_num = 1;
		for (auto param : params) {
			if (numeric_limits<ParamNo>::max() / param.targets.size() < colors_num)
				throw overflow_error("Possible number of parametrizations exceeds "
				"2^64. Constrain the parametrization space "
				"manually.");
			colors_num *= param.targets.size();
		}
		return colors_num;
	}

	/**
	 * @brief fitsConditions
	 * @param sat conditions set by a regulation.
	 */
	static bool fitsConditions(const Model::EdgeConstFunc &sat,
		const bool activating, const bool inhibiting) {
		size_t comb_type =
			static_cast<size_t>(activating)+2 * static_cast<size_t>(inhibiting);
		switch (comb_type) {
		case 0:
			return sat.none;
		case 1:
			return sat.act;
		case 2:
			return sat.inh;
		case 3:
			return sat.both;
		}
		return false;
	}
};

#endif // PARAMETRIZATIONS_HELPER_HPP
