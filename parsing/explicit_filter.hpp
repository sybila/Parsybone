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
	struct SingleFilter {
		SQLAdapter database; ///< Adapter to the filtering databases.
		vector<size_t> columns; ///< Which columns in the database correspond to the parameters.
		Levels last_parametrization; ///< The last parametrization tested (and also pointed to by the database).
	};
	vector<SingleFilter> filters;

	// @return a vector of column numbers in database, ordered in the same way as paremeters of the model (if not found, get pad by INF value)
	vector<size_t> getColumns(const Kinetics & kinetics, SQLAdapter & sql_adapter) {
		vector<size_t> result;
		vector<string> names = sql_adapter.readColumnNames(PARAMETRIZATIONS_TABLE, regex("K_.*"));

		// Test each specie for all the parameters on incoming regulators
		for (const SpecieID ID : cscope(kinetics.species)) {
			const Kinetics::Params & params = kinetics.species[ID].params;
			// Add the column number if the context was found, INF otherwise
			for (const size_t param_no : cscope(params)) {
				string column_name = KineticsTranslators::makeConcise(params[param_no], kinetics.species[ID].name);
				auto column_it = find(WHOLE(names), column_name);
				if (column_it == names.end())
					throw runtime_error("An column " + column_name + " not found in the filtering database " + sql_adapter.getName());
				else
					result.push_back(distance(names.begin(), column_it));
			}
		}

		if (result.size() != names.size())
			throw runtime_error("There are more contexts in the database " + sql_adapter.getName() + " than there are in the model");
		return result;
	}

public:
	ExplicitFilter() {}

	// @brief prepare add parametrizations that are allowed by given database
	void prepare(const Kinetics & kinetics, SQLAdapter sql_adapter) {
		SingleFilter filter;

		// Obtain columns that are being referenced.
		filter.columns = getColumns(kinetics, sql_adapter);
		// Query the respective table.
		sql_adapter.accessTable(PARAMETRIZATIONS_TABLE);
		// Obtain the database.
		filter.database = move(sql_adapter);
		// Bottom vector is empty
		filter.last_parametrization = Levels{numeric_limits<ActLevel>::min()};

		filters.emplace_back(move(filter));
	}

	bool isSmaller(const Levels & A, const Levels & B) {
		for (const size_t i : cscope(A))
			if (A[i] < B[i])
				return true;
			else if (A[i] > B[i])
				return false;
		return false;
	}

	bool isEqual(const Levels & A, const Levels & B) {
		if (A.size() != B.size())
			throw ("bad lenght");
		for (const size_t i : cscope(A))
			if ( A[i] != B[i])
				return false;
		return true;
	}

	/**
	 * @return true iff the parametrization is not filtered out.
	 */
	inline bool isAllowed(const Kinetics & kinetics, const ParamNo param_no) {
		if (filters.empty())
			return true;
		
		// Update each parametrization to the position that's equal or greater that the current parametrization and return false if it is not equal.
		Levels parametrization = KineticsTranslators::createParamVector(kinetics, param_no);
		for (auto & filter : filters) {
			while (!filter.last_parametrization.empty() && isSmaller(filter.last_parametrization, parametrization)) {
				Levels new_row = filter.database.getRow<ActLevel>(filter.columns);
				if (new_row < filter.last_parametrization && !new_row.empty())
					throw runtime_error("Filter \"" + filter.database.getName() + "\" is unordered");
				filter.last_parametrization = new_row;
			}
			if (filter.last_parametrization.empty() || !isEqual(filter.last_parametrization, parametrization))
				return false;
		}
		return true;
	}
};

#endif // EXPLICIT_FILTER_HPP
