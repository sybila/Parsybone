/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warranty. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARAMETER_HELPER_HPP
#define PARAMETER_HELPER_HPP

#include "model.hpp"
#include "regulation_helper.hpp"

class ParameterHelper {
	/**
	 * @brief getTargetValues  computes exact target values possible in given context.
	 * @param autoreg index of the regulation that goes from itself
	 */
	static Levels getTargetValues(const Model & model, const map<SpecieID, Levels> & all_thrs, const Levels & thrs_comb, const size_t autoreg, const SpecieID t_ID) {
		Levels targets = model.getBasalTargets(t_ID);

		// If there is the loop restriction
		if (model.restrictions.bound_loop && autoreg != INF) {
			size_t self_thrs = thrs_comb[autoreg];
			Levels thresholds = (all_thrs.find(t_ID))->second;
			size_t bottom_border = 0u < self_thrs ? thresholds[self_thrs - 1] : 0u;
			size_t top_border = thresholds.size() > self_thrs ? thresholds[self_thrs] : model.getMax(t_ID) + 1;
			Levels new_targets;

			// Add levels that are between the thresholds and one below/above if corresponds to the original.
			if (targets.front() < bottom_border)
				new_targets.push_back(bottom_border - 1);
			for (const auto target : targets)
			if (target >= bottom_border && target < top_border)
				new_targets.push_back(target);
			if (targets.back() >= top_border)
				new_targets.push_back(top_border);

			targets = new_targets;
		}

		return targets;
	}

	/**
	 * @brief getSingleParam creates a parameter for a single context.
	 * @return
	 */
	static void addSingleParam(Model & model, const map<SpecieID, Levels> & all_thrs, const Levels & thrs_comb, const SpecieID t_ID, const size_t autoreg_ID) {
		// Empty data to fill.
		string context;
		map<StateID, Levels> requirements;

		// Loop over all the sources.
		for (auto source_num : crange(thrs_comb.size())) {
			// Find the source details and its current threshold
			string source_name = ModelTranslators::getRegulatorsNames(model, t_ID)[source_num];
			StateID source_ID = ModelTranslators::getRegulatorsIDs(model, t_ID)[source_num];
			auto thresholds = all_thrs.find(source_ID)->second;

			// Find activity level of the current threshold.
			ActLevel threshold = (thrs_comb[source_num] == 0) ? 0 : thresholds[thrs_comb[source_num] - 1];

			// Add current regulation as present.
			string regulation_name = source_name + ":" + to_string(threshold);

			// Add the regulation to the source
			context += regulation_name + ",";

			// Find in which levels the specie must be for the regulation to occur.
			ActLevel next_th = (thrs_comb[source_num] == thresholds.size()) ? model.getMax(source_ID) + 1 : thresholds[thrs_comb[source_num]];
	
			requirements.insert(make_pair(source_ID, vrange(threshold, next_th)));
		}

		model.addParameter(t_ID, move(context.substr(0, context.length() - 1)), move(requirements),
			move(getTargetValues(model, all_thrs, thrs_comb, autoreg_ID, t_ID)));
	}

public:
	/**
	 * @brief createParameters Creates a description of kinetic parameters.
	 */
	static void createParameters(Model & model, const SpecieID t_ID) {
		auto all_thrs = ModelTranslators::getThresholds(model, t_ID);
		Levels bottom, thrs_comb, top;
		size_t autoreg = INF;

		// These containers hold number of thresholds per regulator.
		for (auto & source_thresholds : all_thrs) {
			bottom.push_back(0);
			thrs_comb.push_back(0);
			top.push_back(source_thresholds.second.size());
			if (source_thresholds.first == t_ID)
				autoreg = thrs_comb.size() - 1;
		}

		// Loop over all the contexts.
		do {
			addSingleParam(model, all_thrs, thrs_comb, t_ID, autoreg);
		} while (iterate(top, bottom, thrs_comb));
	}

	/**
	 * @brief fillParameters   fill idividual parameter values based on user specifications.
	 */
	static void fillParameters(Model & model) {
		for (const SpecieID ID : crange(model.species.size())) {
			createParameters(model, ID);
		}
	}
};

#endif // PARAMETER_HELPER_HPP
