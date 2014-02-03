/*
* Copyright (C) 2012-2013 - Adam Streck
* This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
* ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
* ParSyBoNe is released without any warranty. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
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
	 * @brief isSubordinate returns true if the current context is the same as the compared context only with a higher activity value in specificed regulator.
	 */
	static bool isSubordinate(const vector<Model::Regulation> &reguls, const Model::Parameter &current, const Model::Parameter &compare, const SpecieID source_ID) {
		for (const Model::Regulation & regul : reguls) {
			const SpecieID regul_ID = regul.source;
			// All the regulations must have the same requirements, except for the one with the specified source, which must connect on the value.
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
};

#endif // PARAMETRIZATIONS_HELPER_HPP
