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
#include "../results/result_storage.hpp"

class ProductAnalyzer {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const ParametrizedStructure & structure; // Structure from the product
    const AutomatonStructure & automaton; // Automaton from the product
	const FunctionsStructure & functions; // Functions from the product
	const SplitManager & split_manager; // Split manager that holds information about current round
	ResultStorage & results; // Place to store the obtained data

	std::vector<std::vector<std::size_t>> functions_values;
	std::vector<std::size_t> current_color;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Compute a vector that stores all the values for each regulatory context
	 *
	 * @return vector of vectors of values
	 */
	const std::vector<std::vector<std::size_t>> getValues() const {
		std::vector<std::vector<std::size_t>> parameter_values;
		for (std::size_t function_num = 0; function_num < functions.getFunctionsCount(); function_num++) 
			// Get all the values for a function
			parameter_values.push_back(functions.getPossibleValues(function_num));
		return parameter_values;
	}

	/**
	 * Get vector with the lowest value for each function
	 *
	 * @return vector of values
	 */
	std::vector<std::size_t> getBottomValues() const {
		std::vector<std::size_t> bottom_values;
		for (std::size_t function_num = 0; function_num < functions.getFunctionsCount(); function_num++) 
			// Get the lowes value for the function
			bottom_values.push_back(functions.getPossibleValues(function_num).front());
		return bottom_values;
	}

	/**
	 * Creates a color string in the form [context_11, context_12, context_21 ...]
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
	std::vector<std::string> getColors(Parameters result_parameters) const {	
		// Vector to fill
		std::vector<std::string> colors;
		std::vector<std::size_t> work_color = current_color;
		// Change the order of values to from right to left
		result_parameters = swap(result_parameters);
		if (split_manager.lastRound()) 
			result_parameters >>= (getParamsetSize() - (split_manager.getRoundRange().second - split_manager.getRoundRange().first));

		// Cycle through all round colors
		for (std::size_t col_num = split_manager.getRoundRange().first; col_num < split_manager.getRoundRange().second; col_num++) {
			// Output current values
			if (result_parameters % 2) 
				colors.push_back(createColorString(work_color));

			// Increase values
			result_parameters >>= 1;
			iterateColor(work_color);
		}
		return colors;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTING FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	ProductAnalyzer(const ProductAnalyzer & other);            // Forbidden copy constructor.
	ProductAnalyzer& operator=(const ProductAnalyzer & other); // Forbidden assignment operator.

public:
	/**
	 * Get reference data and create final states that will hold all the computed data
	 */
	ProductAnalyzer(const ProductStructure & _product, const SplitManager & _split_manager, ResultStorage & _results) 
		           : structure(_product.getKS()), automaton(_product.getBA()), functions(_product.getFunc()), 
				     split_manager(_split_manager), results(_results)  {
		functions_values = std::move(getValues());
		current_color = std::move(getBottomValues());
	} 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// RESULT FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void storeResults(const std::size_t state_num, const Parameters parameters) {
		// Store results for this 
		results.addColoring(state_num, parameters, getColors(parameters));
	}
};

#endif
