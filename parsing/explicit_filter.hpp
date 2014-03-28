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
		vector<size_t> relevant_indeces;
		Configurations parametrizations;
	};

	vector<SingleFilter> filters;

	// @return a vector of column numbers in database, ordered in the same way as paremeters of the model
	vector<size_t> getColumns(const Kinetics & kinetics, SQLAdapter & sql_adapter) {
		vector<size_t> result;
		vector<string> names = sql_adapter.readColumnNames(PARAMETRIZATIONS_TABLE, regex("K_.*"));

		// Test each specie for all the parameters on incoming regulators
		for (const SpecieID ID : cscope(kinetics.species)) {
			const Kinetics::Params & params = kinetics.species[ID].params;
			// Add the column number if the context was found, INF otherwise
			for (const int param_no : cscope(params)) {
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

	// @ remove replace for INF all the values corresponding to non-functional parameters
	vector<size_t> removeNonFunctional(const Kinetics & kinetics, vector<size_t> columns) {
		size_t param_no = 0;
		for (auto & specie : kinetics.species) {
			for (auto & param : specie.params) {
				if (!param.functional)
					columns[param_no] = INF;
				param_no++;
			}
		}
		return columns;
	}

	// @ remove replace for INF all the values corresponding to don't care values in the database (marked by -1)
	vector<size_t> removeDontCares(SQLAdapter & sql_adapter, vector<size_t> columns) {
		vector<size_t> disabled(columns.size());

		size_t param_no = 0;
		sql_adapter.accessTable(PARAMETRIZATIONS_TABLE);
		Levels first_row = sql_adapter.getRow<ActLevel>(columns);
		for (size_t i : cscope(columns)) {
			disabled[i] = first_row[i] == -1 ? INF : columns[i];
		}

		return disabled;
	}

public:
	ExplicitFilter() {}

	// @brief prepare add parametrizations that are allowed by given database
	void prepare(const Kinetics & kinetics, SQLAdapter & sql_adapter) {
		// Find relevant columns
		vector<size_t> columns = getColumns(kinetics, sql_adapter);
		columns = removeDontCares(sql_adapter, move(columns));
		columns = removeNonFunctional(kinetics, move(columns));

		// Store which parameters are checked
		SingleFilter this_filter;
		for (size_t i : cscope(columns))
			if (columns[i] != INF)
				this_filter.relevant_indeces.emplace_back(i);

		// Find which parameters in database are required
		auto rem_end = remove_if(WHOLE(columns), [](const size_t val){return val == INF; });
		columns.resize(distance(columns.begin(), rem_end));

		// Obtain the parametrizations from the database
		sql_adapter.accessTable(PARAMETRIZATIONS_TABLE);
		Levels parametrization = sql_adapter.getRow<ActLevel>(columns);
		while (!parametrization.empty()) {
			this_filter.parametrizations.emplace_back(move(parametrization));
			parametrization = sql_adapter.getRow<ActLevel>(columns);
		}

		// Remove redundancies
		sort(WHOLE(this_filter.parametrizations));
		auto new_end = unique(WHOLE(this_filter.parametrizations));
		this_filter.parametrizations.resize(distance(begin(this_filter.parametrizations), new_end));

		filters.emplace_back(move(this_filter));
	}

	//int getCount(const Kinetics & kinetics, const Levels & parametrization, SQLAdapter & database) {
	//	string query = "SELECT COUNT(*) FROM " + PARAMETRIZATIONS_TABLE + " WHERE ";
	//	vector<string> matches;
	//	size_t param_no = 0;
	//	for (auto & specie : kinetics.species) {
	//		for (auto & param : specie.params) {
	//			if (parametrization[param_no] != -1) {
	//				string context = KineticsTranslators::makeConcise(param, specie.name);
	//				matches.emplace_back("(" + context + "=" + to_string(parametrization[param_no]) + " OR " + context + "=-1)");
	//			}
	//			param_no++;
	//		}
	//	}
	//	query += alg::join(matches, " AND ");
	//	database.safePrepare(query);

	//	vector<int> count = database.getRow<int>({ 0 });
	//	return count.front();
	//}

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
			auto is_equal = [&filter, &parametrization](const Levels & values) {
				for (size_t i : cscope(filter.relevant_indeces))
					if (values[i] != parametrization[filter.relevant_indeces[i]])
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
