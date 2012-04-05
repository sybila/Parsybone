/*
 * Copyright (C) 2012 - Adam Streck
 *
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 *
 * ParSyBoNe is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 *
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno.
 * See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_OUTPUT_MANAGER_INCLUDED
#define PARSYBONE_OUTPUT_MANAGER_INCLUDED

#include "../reforging/functions_structure.hpp"
#include "../parsing/model.hpp"
#include "results.hpp"

class OutputManager {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Provided with constructor
	const SplitManager & split_manager;
	const Results & results;
	const FunctionsStructure & functions_structure;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	OutputManager(const OutputManager & other);            // Forbidden copy constructor.
	OutputManager& operator=(const OutputManager & other); // Forbidden assignment operator.

public:
	OutputManager(const Results & _results, const FunctionsStructure & _functions_structure, const SplitManager & _split_manager) 
		         : results(_results), functions_structure(_functions_structure), split_manager(_split_manager) { } 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	/**
	 * @return vector of values for each function
	 */
	const std::vector<std::vector<std::size_t>> getValues() const {
		std::vector<std::vector<std::size_t>> parameter_values;
		for (std::size_t function_num = 0; function_num < functions_structure.getFunctionsCount(); function_num++) 
			parameter_values.push_back(functions_structure.getPossibleValues(function_num));
		return parameter_values;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OUTPUT FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	 * display given parameters in the form [fun1, fun2, ...]
	 */
	void outputColors() const {
		// Get a vector of all values for all the functions
		std::vector<std::vector<std::size_t>> all_values = std::move(getValues());
		// Create a vector currently storing lowes value for each function
		std::vector<std::size_t> current_value(all_values.size(), 0);
		for (std::size_t function_num = 0; function_num < current_value.size(); function_num++) {
			current_value[function_num] = all_values[function_num][0];
		}
		
		Parameters result_parameters;
		SplitManager splitting = split_manager;
		// Cycle through parameters
		for (std::size_t parameter_num = 0; parameter_num < functions_structure.getParametersCount(); parameter_num++) {
			if (parameter_num % getParamsetSize() == 0) {
				result_parameters = swap(results.getMergedParameters(parameter_num / getParamsetSize()));
				if (splitting.lastRound()) 
					result_parameters >>= (getParamsetSize() - splitting.getRoundRange().second - splitting.getRoundRange().first);
				else
					splitting.increaseRound();
			}

			// Output current values
			if (result_parameters % 2) {
				output_streamer.output(data, "[", OutputStreamer::no_newl);
				for (auto it = current_value.begin(); it != current_value.end() - 1; it++) {
					output_streamer.output(data, *it, OutputStreamer::no_newl).output(",", OutputStreamer::no_newl);
				}
				output_streamer.output(data, current_value.back(), OutputStreamer::no_newl).output("]");
			}
			result_parameters >>= 1;

			// Iterate target values
			for (std::size_t value_num = 0; value_num < current_value.size(); value_num++) {
				if (current_value[value_num] < all_values[value_num].back()) {
					current_value[value_num]++;
					break;
				}
				else { 
					current_value[value_num] = all_values[value_num].front();
				}
			}
		}
	}

public:
	/**
	 * main output function - displays number of parameters and parameters themselves if needed
	 *
	 * @param colors	if true, coloring of individuall final states will be shown
	 */
	void basicOutput() const {
		// Display amount of all colors
		output_streamer.output(data, "Total number of parameters is ", OutputStreamer::no_newl).output(results.countParameters(), OutputStreamer::no_newl)
			           .output(" out of ", OutputStreamer::no_newl).output(results.getParametersCount(), OutputStreamer::no_newl).output("");
		// Display amount of colors of states
		/*if (user_options.verbose) {
			for (std::size_t state_num = 0; state_num < results.getStatesCount(); state_num++) {
				output_streamer.output(data, "State BA", OutputStreamer::no_newl).output(results.getBANum(state_num), OutputStreamer::no_newl).output(", KS", OutputStreamer::no_newl)
					           .output(results.getKSNum(state_num), OutputStreamer::no_newl).output(" is colored with parameters");
			}
		}*/
		// display the colors
		if (user_options.show_final_coloring) 
			outputColors();
	}
};

#endif