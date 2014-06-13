/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warranty. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */


#ifndef EXPLICIT_FILTER_HPP
#define EXPLICIT_FILTER_HPP

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief This class takes parametrizations from the database and rememberes those that are allowed by the database.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../auxiliary/data_types.hpp"
#include "../model/model_translators.hpp"
#include "../kinetics/kinetics_translators.hpp"
#include "../auxiliary/SQLAdapter.hpp"

class ExplicitFilter {
	struct SingleFilter{
		vector<size_t> columns;
		Configurations parametrizations;
	};

	vector<SingleFilter> filters;

public:
	ExplicitFilter() {}

	// @brief prepare add parametrizations that are allowed by given database
	void prepare(const Kinetics & kinetics, SQLAdapter & sql_adapter) {
		// Test for emptyness 
		sql_adapter.accessTable(PARAMETRIZATIONS_TABLE);
		if (sql_adapter.getRow<ActLevel>({ 0 }).empty())
			return;

		// Find relevant columns
		SingleFilter this_filter;
		this_filter.columns = sql_adapter.getColumnsByExpression(PARAMETRIZATIONS_TABLE, regex("K_.*")).second;
		auto names = sql_adapter.getColumnsByExpression(PARAMETRIZATIONS_TABLE, regex("K_.*")).first;

		// Obtain the parametrizations from the database
		sql_adapter.accessTable(PARAMETRIZATIONS_TABLE);
		Levels parametrization = sql_adapter.getRow<ActLevel>(this_filter.columns);
		while (!parametrization.empty()) {
			this_filter.parametrizations.emplace_back(move(parametrization));
			parametrization = sql_adapter.getRow<ActLevel>(this_filter.columns);
		}

		filters.emplace_back(move(this_filter));
	}

	/**
	 * @return true iff the parametrization is not filtered out.
	 */
	inline bool isAllowed(const Kinetics & kinetics, const ParamNo param_no) {
		if (filters.empty())
			return true;

		// Test the current parametrization against all the filters
		const Levels parametrization = KineticsTranslators::createParamVector(kinetics, param_no);
		for (auto & filter : filters) {
			// A decision saying whether the parametrization is equal to the some in the filter
			auto is_equal = [&parametrization](const Levels & values) {
				for (size_t i : cscope(parametrization))
					if (values[i] != parametrization[i])
						return false;
				return true;
			};
			// True if a match with something in the database is found
			if (find_if(WHOLE(filter.parametrizations), is_equal) == filter.parametrizations.end()) {
				return false;
			}
		}
		return true;
	}
};

#endif // EXPLICIT_FILTER_HPP
