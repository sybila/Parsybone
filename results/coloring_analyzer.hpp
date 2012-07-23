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
// ColoringAnalyzer is used to get polished and formatted data from the product.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../construction/construction_holder.hpp"

class ColoringAnalyzer {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// References
	const ParametrizationsHolder & parametrizations; // Functions from the product

	// DATA STORAGE
	std::vector<Coloring> colorings;

	// Numbers of currently used colors
	std::vector<std::size_t> subcolor_nums;
	std::vector<std::size_t> max_colors;

	//// Used only for a single round
	ColorNum parameter_begin;
	ColorNum parameter_end;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COLOR BASED FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Creates a color string in the form [context_11, context_12, context_21 ...]
	 *
	 * @param color	vector of contexts for the color
	 */
	const std::string createColorString(std::vector<std::size_t> color_parts) const {
		std::string color_str = "[";
		// Cycle through all values except last
		for (SpecieID ID = 0; ID < parametrizations.getSpecieNum(); ID++) {
			auto color = parametrizations.getColor(ID, color_parts[ID]);
			for (auto it = color.begin(); it != color.end(); it++) {
				color_str += boost::lexical_cast<std::string, std::size_t>(*it);
				color_str += ",";
			}
		}
		*(color_str.end() - 1) = ']';
		
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
		for (SpecieID ID = 0; ID < parametrizations.getSpecieNum(); ID++) {
			subcolor_nums.push_back(0);
			max_colors.push_back(parametrizations.getColorsNum(ID) - 1);
		}
	}
	
	ColoringAnalyzer(const ColoringAnalyzer & other);            // Forbidden copy constructor.
	ColoringAnalyzer& operator=(const ColoringAnalyzer & other); // Forbidden assignment operator.

public:
	/**
	 * Get reference data and create final states that will hold all the computed data
	 */
	ColoringAnalyzer(const ConstructionHolder & holder)
					  : parametrizations(holder.getParametrizations())  {
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

		colorings.clear();
		parameter_end = round_range.second;
		parameter_begin = round_range.first;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OUTPUT FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
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
// REFORMING GETTERS
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
	const std::vector<std::pair<std::size_t, std::string> > getColors(Parameters result_parameters) const {
		// Vector to fill
		std::vector<std::pair<std::size_t, std::string> > colors;

		// Change the order of values to: from right to left
		result_parameters = swap(result_parameters, getParamsetSize() - static_cast<std::size_t>(parameter_end - parameter_begin));
		// Store a mask for each color with just its bit on, other off
		Parameters color_mask = 1 << ((parameter_end - parameter_begin) - 1);

		// Cycle through all round colors
		for (ColorNum col_num = parameter_begin; col_num < parameter_end; col_num++) {
			// Output current values
			if (result_parameters % 2)
				colors.push_back(std::make_pair(color_mask, parametrizations.createColorString(col_num)));

			// Increase values
			color_mask >>= 1;
			result_parameters >>= 1;
		}
		return colors;
	}

	/**
	 * Obtain colors given parameters in the form [fun1, fun2, ...] for all parameters in this round
	 *
	 * @return vector of numbers and strings of colors
	 */
	const std::vector<std::pair<std::size_t, std::string> > getColors() const {
		return getColors(getUnion());
	}
};

#endif
