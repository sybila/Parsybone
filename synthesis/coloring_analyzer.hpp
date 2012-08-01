/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_COLORING_ANALYZER_INCLUDED
#define PARSYBONE_COLORING_ANALYZER_INCLUDED

#include "../construction/construction_holder.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ColoringAnalyzer is used to store individual final states together with their coloring and to further provide necessary data for the output as requested.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ColoringAnalyzer {
	friend class SynthesisManager;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const ParametrizationsHolder & parametrizations; ///< Reference to actuall explicit parametrizations that are further printed

	/// Vector that stores individual states after the coloring procedure
	std::map<StateID, Paramset> colorings;

	ColorNum parameter_begin; ///< Ordinal number of the first parametrization in this round.
	ColorNum parameter_end; ///< Ordinal number of the last parametrization used in this round.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTING FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		
	ColoringAnalyzer(const ColoringAnalyzer & other); ///< Forbidden copy constructor.
	ColoringAnalyzer& operator=(const ColoringAnalyzer & other); ///< Forbidden assignment operator.

	/**
	 * Just past the reference and null variables
	 */
	ColoringAnalyzer(const ConstructionHolder & holder) : parametrizations(holder.getParametrizations())  {
		parameter_begin = parameter_end = 0;
	} 


	/**
	 * Store requested results for a give state of product
	 *
	 * @param state_coloring	ID of a single state together with its coloring after the synthesis
	 */
	void storeResults(const Coloring & state_coloring) {
		// Store state and its parameters
		colorings.insert(state_coloring);
	}

	/**
	 * Iterates color until it responds to the first parameter of this round
	 *
	 * @param round_range	first and one behind last parameter of this round
	 */
	void strartNewRound(const Range & round_range) {
		if (round_range.first < parameter_begin) // Error check
			throw std::runtime_error("Round start value is lower than start of previous round.");

		colorings.clear();
		parameter_end = round_range.second;
		parameter_begin = round_range.first;
	}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// REFORMING GETTERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @param result_parameters	mask of parametrizations that were synthetised
	 *
	 * @return ordinal number of the parametrizations that are feasible
	 */
	const std::vector<ColorNum> buildNumbers(const Paramset result_parameters) const  {
		// Vector to fill
		std::vector<ColorNum> numbers;
		// Store a mask for each color with just its bit on, other off
		  Paramset color_mask = paramset_helper.getLeftOne(parameter_end - parameter_begin);

		// Cycle through all round colors
		for (ColorNum col_num = parameter_begin; col_num < parameter_end; col_num++) {
			// Output current values
			if (result_parameters & color_mask)
				numbers.push_back(col_num);

			// Move the mask
			color_mask >>= 1;
		}
		return numbers;
	}

	/**
	 * Obtain colors given parameters in the form [fun1, fun2, ...] for specified parameters
	 *
	 * @param result_parameters	mask of parametrizations that were synthetised
	 *
	 * @return vector of masks and strings of feasible colors in this round
	 */
	const std::vector<std::string> buildStrings(const Paramset result_parameters) const {
		// Vector to fill
		std::vector<std::string> strings;
		// Store a mask for each color with just its bit on, other off
		  Paramset color_mask = paramset_helper.getLeftOne(parameter_end - parameter_begin);

		// Cycle through all round colors
		for (ColorNum col_num = parameter_begin; col_num < parameter_end; col_num++) {
			// Output current values
			if (result_parameters & color_mask)
				strings.push_back(parametrizations.createColorString(col_num));

			// Move the mask
			color_mask >>= 1;
		}
		return strings;
	}

public:
	/**
	 * Computes a vector of strings of acceptable colors from this round with their output, as requested by user.
	 *
	 * @return vector of strings with numbers of parametrizations and possible their explicit form
	 */
	const std::vector<std::string> getOutput() const {
		// Vector to fill and return
		std::vector<std::string> results;

		// Get vectors of values and their iterators
		auto numbers = std::move(getNumbers());
		auto numbers_it = numbers.begin();
		auto strings = std::move(getStrings());
		auto strings_it = strings.begin();

		// Go through all acceptable parametrizations and recreate them into strings
		while (numbers_it != numbers.end()) {
			std::string result(toString(*numbers_it));
			numbers_it++;
			// If the parametrization string is required on the output
			if (user_options.coloring()) {
				result.append(*strings_it);
				strings_it++;
			}
			results.push_back(result);
		}

		return results;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		
	/**
	 * Obtain colors given parameters in the form [fun1, fun2, ...] for required state
	 *
	 * @param ID	index of the state to get the mask from
	 *
	 * @return vector of numbers and strings of colors
	 */
	const std::vector<std::string> getStrings(const StateID ID) const {
		return buildStrings(getMask(ID));
	}

	/**
	 * Obtain colors given parameters in the form [fun1, fun2, ...] for all parameters in this round
	 *
	 * @return vector of numbers and strings of colors
	 */
	const std::vector<std::string> getStrings() const {
		return buildStrings(getMask());
	}

	/**
	 * @param ID	index of the state to get the mask from
	 *
	 * @return ordinal number of the parametrizations that are acceptable
	 */
	const std::vector<ColorNum> getNumbers(const StateID ID) const {
		auto coloring = colorings.find(ID);
		if (coloring != colorings.end())
			return buildNumbers(coloring->second);
		else
			return buildNumbers(0);
	}

	/**
	 * @return ordinal numbers of the parametrizations that are acceptable in this round
	 */
	const std::vector<ColorNum> getNumbers() const {
		return buildNumbers(getMask());
	}

	/**
	 * @param ID	index of the state to get the mask from
	 *
	 * @return coloring of the given state or 0 if the state is not present
	 */
	const Paramset getMask(const StateID ID) const {
		auto coloring = colorings.find(ID);
		if (coloring != colorings.end())
			return coloring->second;
		else
			return 0;
	}

	/**
	 * Compute merge of all final colors, creating a coloring with all feasible colors in this round
	 *
	 * @return all feasible colors in this round
	 */
	const Paramset getMask() const {
		Paramset all = 0;
		for (auto coloring_it = colorings.begin(); coloring_it != colorings.end(); coloring_it++) {
			all |= coloring_it->second;
		}
		return all;
	}
};

#endif // PARSYBONE_COLORING_ANALYZER_INCLUDED
