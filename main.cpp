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

#include <memory>
#include <iostream>

#include "parsing/model_parser.hpp"
#include "reforging/basic_structure_builder.hpp"
#include "reforging/functions_builder.hpp"
#include "reforging/parametrized_structure_builder.hpp"
#include "reforging/automaton_builder.hpp"
#include "coloring/model_checker.hpp"
#include "results/results.hpp"
#include "results/output_manager.hpp"

// porgram-related data
const float program_version = 1.0;
float model_file_version;

// Clocks - dependendent on the achitecture. 
#ifdef __GNUC__
#include <sys/time.h>
/**
 * @return	time in miliseconds
 */
long long my_clock() {
	timeval tv;
	gettimeofday(&tv, 0);
	return tv.tv_sec*1000 + tv.tv_usec/1000;
}
#else
#include <windows.h>
/**
 * @return	time in miliseconds
 */
long long my_clock() {
	return GetTickCount();
}
#endif

/**
 * main 
 */
int main(int argc, char* argv[]) {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STEP ONE:
// Parse input information.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// As an input and output the program will use, can vary based on user input
	std::istream * input_stream  = &std::cin;
	std::ifstream input_file;
	std::ostream * output_stream = &std::cout;
	std::ofstream output_file;

	// structure that holds user-specified options
	UserOptions user_options = {0};

	// Model that will be obtained from the input
	Model model;
	
	try {
		// Parse arguments
		for (int arg_n = 1; arg_n < argc; arg_n++) { 
			std::string arg = argv[arg_n];

			if (arg == "-b")
				user_options.show_base_coloring = true;

			else if (arg == "-c")
				user_options.show_counterexamples = true;

			else if (arg == "-g")
				user_options.guarantee = true;

			else if (arg == "-v")
				user_options.verbose = true;

			else if (arg == "-n")
				user_options.negative_check = true;

			else if (arg == "-s")
				user_options.control_semantics = true;

			// Redirecting input to a file by a parameter
			else if (arg == "-f") {
				input_file.open(argv[++arg_n], std::ios::in);
				if (input_file.fail())
					throw (std::invalid_argument(std::string("Wrong input filename: ").append(arg).c_str()));
				input_stream = &input_file;
			}

			// Redirecting output to a file by a parameter
			else if (arg == "-F") {
				output_file.open(argv[++arg_n], std::ios::out);
				if (output_file.fail())
					throw (std::invalid_argument(std::string("Wrong output filename: ").append(arg).c_str()));
				output_stream = &output_file;
			}

			else 
				throw (std::invalid_argument(std::string("Wrong argument: ").append(arg).c_str()));
		}

		// Parse model file
		*output_stream << "Parsing started.\n";
		ModelParser model_parser(user_options, *input_stream, model);
		model_file_version = model_parser.parseInput();

	} catch (std::exception & e) {
		std::cerr << "Error occured while parsing input: " << e.what() << ". \n";
		return 1;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STEP TWO:
// Control the semantics.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STEP THREE:
// Create the model structures.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Long-life data structures - empty constructors
	AutomatonStructure automaton; // Set of transitions - controlling automaton
	ParametrizedStructure parametrized_structure; // Kripke structure that has transitions labelled with functions
	BasicStructure basic_structure;   // Kripke structure built from the network
	FunctionsStructure functions_structure; // Implicit reprezentation of regulatory functions

	// Data creation
	try {
		*output_stream << "Data building started.\n";

		// Parametrized Structure building
		BasicStructureBuilder basic_structure_builder(user_options, model, basic_structure);
		basic_structure_builder.buildStructure();
		FunctionsBuilder functions_builder(user_options, model, functions_structure);
		functions_builder.buildFunctions();
		ParametrizedStructureBuilder parametrized_structure_builder(user_options, basic_structure, functions_structure, parametrized_structure);
		parametrized_structure_builder.buildStructure();

		// Automata Structure building
		AutomatonBuilder automaton_builder(user_options, model, automaton);
		automaton_builder.buildAutomaton();
	} catch (std::exception & e) {
		std::cerr << "Error occured while building data structures: " << e.what() << ". \n";
		return 3;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STEP FOUR:
// Model-check and synthetize parameters.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	Results results;
	try {
		long long start_time = my_clock();
		*output_stream << "Coloring started.\n";
		ModelChecker model_checker(user_options, parametrized_structure, automaton, results);
		model_checker.computeResults();
		*output_stream << "Coloring ended after: " << (my_clock() - start_time) / 1000.0 << " seconds.\n";
	} catch (std::exception & e) {
		std::cerr << "Error occured while syntetizing the parameters: " << e.what() << ". \n";
		return 4;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STEP FIVE:
// Analyze results and provide the output.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	try {
		*output_stream << "Output started.\n";
		OutputManager output_manager(user_options, *output_stream, results, functions_structure);
		output_manager.basicOutput();
	} catch (std::exception & e) {
		std::cerr << "Error occured during output of the results: " << e.what() << ". \n";
		return 5;
	}
	return 0;
}
