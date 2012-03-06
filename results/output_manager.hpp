/*
 * Copyright (C) 2012 - Adam Streck
 *
 * This file is part of PoSeIDoN (Parameter Synthetizer for Discrete Networks) verification tool
 *
 * Poseidon is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Poseidon is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 *
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno.
 * See http://sybila.fi.muni.cz/ .
 */

#ifndef POSEIDON_OUTPUT_MANAGER_INCLUDED
#define POSEIDON_OUTPUT_MANAGER_INCLUDED

#include <iostream>

#include "../reforging/functions_structure.hpp"
#include "../parsing/model.hpp"
#include "results.hpp"

class OutputManager {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Provided with constructor
	const UserOptions & user_options;
	const Results & results;
	const FunctionsStructure & functions_structure;
	std::ostream & output_stream;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	OutputManager(const OutputManager & other);            // Forbidden copy constructor.
	OutputManager& operator=(const OutputManager & other); // Forbidden assignment operator.

public:
	OutputManager(const UserOptions & _user_options, std::ostream & _output_stream, const Results & _results, const FunctionsStructure & _functions_structure) 
		: user_options(_user_options), output_stream(_output_stream), results(_results), functions_structure(_functions_structure) {} // Default empty constructor

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	/**
	 * @return vector of values for each function
	 */
	const std::vector<std::vector<std::size_t>> getValues() {
		std::vector<std::vector<std::size_t>> parameter_values;
		for (std::size_t function_num = 0; function_num < functions_structure.getFunctionsCount(); function_num++) 
			parameter_values.push_back(functions_structure.getPossibleValues(function_num));
		return parameter_values;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OUTPUT FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*void outputParameters(const Parameters & result_parameters) {
		std::vector<std::vector<std::size_t>> all_values = std::move(getValues());
		std::vector<std::size_t> current_value(all_values.size(), 0);
		for (std::size_t function_num = 0; function_num < current_value.size(); function_num++) {
			current_value[function_num] = all_values[function_num][0];
		}
		
		for (std::size_t parameter_num = 0; parameter_num < result_parameters.size(); parameter_num++) {
			if (result_parameters[parameter_num]) {
				output_stream << "[";
				for (auto it = current_value.begin(); it != current_value.end() - 1; it++) {
					output_stream << *it <<	",";
				}
				output_stream << current_value.back() << "]\n";
			}
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
	}*/

public:
	void basicOutput() {
		output_stream << "Total number of parameters: "  << results.countParameters() << " out of: " << functions_structure.getParametersCount() << ".\n";
		
		/*for (std::size_t coloring_num = 0; coloring_num < results.getColoringsCount(); coloring_num++) {
			output_stream << "State: " << results.getColoring(coloring_num).first << " is colored with " << results.getColoring(coloring_num).second.count() << " parameters.\n";
		}

		output_stream << "All the parameters:\n";
		outputParameters(result_parameters);*/
		
		/*for (std::size_t coloring_num = 0; coloring_num < results.getColoringsCount(); coloring_num++) {
			const Coloring & coloring = results.getColoring(coloring_num);
			output_stream << "Coloring of the state: " << coloring.first << " is: " << coloring.second << ".\n";
		}*/
	}
};

#endif