/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_COLORING_ANALYZER_INCLUDED
#define PARSYBONE_COLORING_ANALYZER_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product analyzer is used to get polished and formatted data from the product.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../reforging/product_structure.hpp"

class ColoringAnalyzer {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// References
	const FunctionsStructure & functions; // Functions from the product

	// DATA STORAGE
	std::vector<Coloring> colorings;

	// Used throughout full computation - values for each function
	std::vector<std::size_t> bottom_values;
	std::vector<std::size_t> top_values;

	// Used only for a single round
	std::vector<std::size_t> current_color;
	std::size_t parameter_begin;
	std::size_t parameter_end;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COLOR BASED FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Increment values in curren_color so we get next color in the ordering
	 *
	 * @param color	vector of contexts for the color
	 */
	void iterateColor(std::vector<std::size_t> & color) const {
		// If there is a posibility to increaset the value (from left to right), increase it and end, otherwise null it and continue
		for (std::size_t value_num = 0; value_num < color.size(); value_num++) {
			// Increase and end
			if (color[value_num] < top_values[value_num]) {
				color[value_num]++;
				return;
			}
			// Null 
			else { 
				color[value_num] = bottom_values[value_num];
			}
		}
	}
	
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
	
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTING FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	/**
	 * Compute a vector that with lowests and one with highest values for each regulatory context.
	 */
	void computeBoundaries() {
		// Cycle through all functions
		for (std::size_t specie_num = 0; specie_num != functions.getSpeciesCount(); specie_num++) {
			for (std::size_t function_num = 0; function_num < functions.getRegulationsCount(specie_num); function_num++) {
				// Get bottom and top values for a function
				bottom_values.push_back(functions.getPossibleValues(specie_num, function_num).front());
				top_values.push_back(functions.getPossibleValues(specie_num, function_num).back());
			}
		}
		current_color = bottom_values;
	}
	
	ColoringAnalyzer(const ColoringAnalyzer & other);            // Forbidden copy constructor.
	ColoringAnalyzer& operator=(const ColoringAnalyzer & other); // Forbidden assignment operator.

public:
	/**
	 * Get reference data and create final states that will hold all the computed data
	 */
	ColoringAnalyzer(const ProductStructure & _product) 
		           : functions(_product.getFunc())  {
		computeBoundaries();
		parameter_begin = parameter_end = 0;
	} 

	/**
	 * Iterates color until it responds to the first parameter of this round
	 *
	 * @param round_range	first and one behind last parameter of this round
	 */
	void strartNewRound(const Range & round_range) {
		// Error check
		if (round_range.first < parameter_begin)
			throw std::runtime_error("Round start value is lower than start of previous round.");
		
		// Clear storage
		colorings.clear();

		// Iterate reference values for the state up till first state of this round
		parameter_end = round_range.second;
		for (; parameter_begin < round_range.first; parameter_begin++) {
			iterateColor(current_color);
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OUTPUT FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	/*
	 * Output all colors from this round.
	 */
	void display() const {
		// Get strings for all the colors
		auto colors = getColors();

		// Output colors
		for (auto color_it = colors.begin(); color_it != colors.end(); color_it++) {
			output_streamer.output(results_str, color_it->second);
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STORING FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Store requested results for a give state of product
	 */
	void storeResults(const Coloring & results) {
		// Store state and its parameters
		colorings.push_back(results);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		
	/**
	 * Compute merge of all final colors creating a coloring with all feasible colors
	 *
	 * @return all feasible colors in this round
	 */
	const Parameters getUnion() const {
		Parameters all = 0;
		for (auto coloring_it = colorings.begin(); coloring_it != colorings.end(); coloring_it++) {
			all |= coloring_it->second;
		}
		return all;
	}

	/**
	 * Obtain colors given parameters in the form [fun1, fun2, ...] for specified parameters
	 *
	 * @param result_parameters	parameters to use
	 * 
	 * @return vector of masks and strings of feasible colors
	 */
	std::vector<std::pair<std::size_t, std::string> > getColors(Parameters result_parameters) const {	
		// Vector to fill
		std::vector<std::pair<std::size_t, std::string> > colors;
		std::vector<std::size_t> work_color = current_color;
		// Change the order of values to: from right to left
		result_parameters = swap(result_parameters, (parameter_end - parameter_begin));
		Parameters color_mask = (1 << (getParamsetSize() - (parameter_end - parameter_begin)));

		// Cycle through all round colors
		for (std::size_t col_num = parameter_begin; col_num < parameter_end; col_num++) {
			// Output current values
			if (result_parameters % 2)
				colors.push_back(std::make_pair(color_mask, createColorString(work_color)));

			// Increase values
			result_parameters >>= 1;
			color_mask >>= 1;
			iterateColor(work_color);
		}
		return colors;
	}

	/**
	 * Obtain colors given parameters in the form [fun1, fun2, ...] for all parameters in this round
	 *
	 * @return vector of numbers and strings of colors
	 */
	std::vector<std::pair<std::size_t, std::string> > getColors() const {	
		return getColors(getUnion());
	}
};

#endif
