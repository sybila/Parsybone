/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_PRODUCT_ANALYZER_INCLUDED
#define PARSYBONE_PRODUCT_ANALYZER_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product analyzer is used to get polished and formatted data from the product.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../reforging/product_structure.hpp"
#include "../reforging/color_storage.hpp"
#include "../results/result_storage.hpp"

class ProductAnalyzer {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const ProductStructure & product; // Product itself
	const ParametrizedStructure & structure; // Structure from the product
    const AutomatonStructure & automaton; // Automaton from the product
	const FunctionsStructure & functions; // Functions from the product
	const ColorStorage & storage;
	ResultStorage & results; // Place to store the obtained data

	// Used throughout full computation
	std::vector<std::vector<std::size_t>> functions_values;

	// Used only for a single round
	std::vector<std::size_t> current_color;
	std::size_t parameter_begin;
	std::size_t parameter_end;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Creates a color string in the form [context_11, context_12, context_21 ...]
	 *
	 * @param color	vector of contexts for the color
	 */
	const std::string createColorString(std::vector<std::size_t> color) const {
		std::string color_str = "[";
		// Cycle through all values except last
		for (auto it = color.begin(); it != color.end() - 1; it++) {
			color_str += boost::lexical_cast<std::string, std::size_t>(*it);
			color_str += ",";
		}
		// Add the last one
		color_str += boost::lexical_cast<std::string, std::size_t>(color.back());
		color_str += "]";
		return color_str;
	}

	/**
	 * Increment values in curren_color so we get next color in the ordering
	 *
	 * @param color	vector of contexts for the color
	 */
	void iterateColor(std::vector<std::size_t> & color) const {
		// If there is a posibility to increaset the value (from left to right), increase it and end, otherwise null it and continue
		for (std::size_t value_num = 0; value_num < color.size(); value_num++) {
			// Increase and end
			if (color[value_num] < functions_values[value_num].back()) {
				color[value_num]++;
				return;
			}
			// Null 
			else { 
				color[value_num] = functions_values[value_num].front();
			}
		}
	}
	
	/**
	 * obtain colors given parameters in the form [fun1, fun2, ...]
	 * this function also causes current_color to change (iterate)
	 *
	 * @return vector of strings with colors
	 */
	std::vector<std::pair<std::size_t, std::string>> getColors(Parameters result_parameters) const {	
		// Vector to fill
		std::vector<std::pair<std::size_t, std::string>> colors;
		std::vector<std::size_t> work_color = current_color;
		// Change the order of values to: from right to left
		result_parameters = swap(result_parameters, (parameter_end - parameter_begin));

		// Cycle through all round colors
		for (std::size_t col_num = parameter_begin; col_num < parameter_end; col_num++) {
			// Output current values
			if (result_parameters % 2) 
				colors.push_back(std::make_pair(col_num,createColorString(work_color)));

			// Increase values
			result_parameters >>= 1;
			iterateColor(work_color);
		}
		return colors;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTING FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	/**
	 * Compute a vector that stores all the values for each regulatory context
	 *
	 * @return vector of vectors of values
	 */
	const std::vector<std::vector<std::size_t>> getValues() const {
		std::vector<std::vector<std::size_t>> parameter_values;
		for (std::size_t specie_num = 0; specie_num != functions.getSpeciesCount(); specie_num++) {
			for (std::size_t function_num = 0; function_num < functions.getRegulationsCount(specie_num); function_num++) {
				// Get all the values for a function
				parameter_values.push_back(functions.getPossibleValues(specie_num, function_num));
			}
		}
		return parameter_values;
	}

	/**
	 * Get vector with the lowest value for each function
	 *
	 * @return vector of values
	 */
	std::vector<std::size_t> getBottomValues() const {
		std::vector<std::size_t> bottom_values;
		for (std::size_t specie_num = 0; specie_num != functions.getSpeciesCount(); specie_num++) {
			for (std::size_t function_num = 0; function_num < functions.getRegulationsCount(specie_num); function_num++) {
				// Get the lowes value for the function
				bottom_values.push_back(functions.getPossibleValues(specie_num, function_num).front());
			}
		}
		return bottom_values;
	}	
	
	ProductAnalyzer(const ProductAnalyzer & other);            // Forbidden copy constructor.
	ProductAnalyzer& operator=(const ProductAnalyzer & other); // Forbidden assignment operator.

public:
	/**
	 * Get reference data and create final states that will hold all the computed data
	 */
	ProductAnalyzer(const ProductStructure & _product, const ColorStorage & _storage, ResultStorage & _results) 
		           : product(_product), storage(_storage), structure(_product.getKS()), automaton(_product.getBA()), functions(_product.getFunc()), results(_results)  {
		functions_values = std::move(getValues());
		current_color = std::move(getBottomValues());
		parameter_begin = parameter_end = 0;
	} 

	/**
	 * Iterates color until it responds to the first parameter of this round
	 *
	 * @param round_range	first and one behind last parameter of this round
	 */
	void setRange(const Range & round_range) {
		if (round_range.first < parameter_begin)
			throw std::runtime_error("Round start value is lower than start of previous round.");
		parameter_end = round_range.second;
		for (; parameter_begin < round_range.first; parameter_begin++) {
			iterateColor(current_color);
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// RESULT FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Store requested results for a give state of product
	 */
	void storeResults(const std::size_t state_num, const bool use_colors) {
		// Get parameters from the state
		Parameters parameters = storage.getParameters(state_num);
		// Store results for this 
		if (use_colors)
			results.addColoring(state_num, parameters, getColors(parameters));
		else
			results.addColoring(state_num, parameters);
	}

	/**
	 * Pick final states from the product and store them with their parameters in the queue of colorings
	 *
	 * @return queue with all colorings of final states
	 */
	const std::vector<Coloring> getFinalColoring() const {
		// Queue tates colored in basic coloring
		std::vector<Coloring> final_colorings; 

		// Get the states and their colors
		std::for_each(product.getFinals().begin(), product.getFinals().end(), [&](std::size_t state_index) {
			final_colorings.push_back(Coloring(state_index, storage.getParameters(state_index)));
		});

		// Return final vertices with their positions
		return final_colorings;
	}
};

#endif
