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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This is the entry point of the program and also only .cpp file used.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <memory>
#include <iostream>

#include "auxiliary/time_manager.hpp"
#include "auxiliary/output_streamer.hpp"
#include "parsing/argument_parser.hpp"
#include "parsing/model_parser.hpp"
#include "reforging/basic_structure_builder.hpp"
#include "reforging/functions_builder.hpp"
#include "reforging/parametrized_structure_builder.hpp"
#include "reforging/automaton_builder.hpp"
#include "reforging/product_structure.hpp"
#include "reforging/product_builder.hpp"
#include "coloring/synthesis_manager.hpp"

// porgram-related data
const float program_version = 1.0;

/**
 * Linear execution of succesing parts of the parameter synthesis
 */
int main(int argc, char* argv[]) {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STEP ZERO:
// Create long-live objects for the whole computation.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	// Model that will be obtained from the input
	Model model;
	// structure that holds user-specified options, set to default values
	UserOptions user_options = {0};	user_options.process_number = 1; user_options.processes_count = 1;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STEP ONE:
// Parse input information.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	try {
		// Parse what is on the input
		parseArguments(user_options, argc, argv);

		// Parse the model
		output_streamer.output(verbose, "Model parsing started.", OutputStreamer::important);
		ModelParser model_parser(user_options, model);
		model_parser.parseInput();
	} 
	catch (std::exception & e) {
		output_streamer.output(fail, std::string("Error occured while parsing input: ").append(e.what()));
		return 1;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STEP TWO:
// Control the semantics.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STEP THREE:
// Create Functions (implicit reprezentation of regulatory contexts of species)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	FunctionsStructure functions_structure;
	try {
		output_streamer.output(verbose, "Functions building started.", OutputStreamer::important);

		FunctionsBuilder functions_builder(user_options, model, functions_structure);
		functions_builder.buildFunctions();
	} 
	catch (std::exception & e) {
		output_streamer.output(fail, std::string("Error occured while building Regulatory functions: ").append(e.what()));
		return 3;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STEP FOUR:
// Create the Kripke Structure (product of all activation value of all the species)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ParametrizedStructure parametrized_structure; // Kripke structure that has transitions labelled with functions
	try {
		output_streamer.output(verbose, "Parametrized Kripke structure building started.", OutputStreamer::important);

		// Create temporary Kripke structure without parametrization
		BasicStructure basic_structure; // Kripke structure built from the network
		BasicStructureBuilder basic_structure_builder(user_options, model, basic_structure);
		basic_structure_builder.buildStructure();

		// Build PKS
		ParametrizedStructureBuilder parametrized_structure_builder(user_options, basic_structure, functions_structure, parametrized_structure);
		parametrized_structure_builder.buildStructure();
	} 
	catch (std::exception & e) {
		output_streamer.output(fail, std::string("Error occured while building Parametrized Kripke structure: ").append(e.what()));
		return 4;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STEP FIVE:
// Create the automaton.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	AutomatonStructure automaton; // Set of transitions - controlling automaton
	try {
		output_streamer.output(verbose, "Buchi automaton building started.", OutputStreamer::important);

		AutomatonBuilder automaton_builder(user_options, model, automaton);
		automaton_builder.buildAutomaton();
	} 
	catch (std::exception & e) {
		output_streamer.output(fail, std::string("Error occured while building the automaton: ").append(e.what()));
		return 5;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STEP SIX:
// Create the product
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ProductStructure product_structure(user_options, functions_structure, parametrized_structure, automaton);
	try {
		output_streamer.output(verbose, "Product building started.", OutputStreamer::important);

		ProductBuilder product_builder(user_options, parametrized_structure, automaton, product_structure);
		product_builder.buildProduct();
	} catch (std::exception & e) {
		output_streamer.output(fail, std::string("Error occured while building the product: ").append(e.what()));
		return 6;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STEP SEVEN:
// Synthetize the colors and output them
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	try {
		SynthesisManager synthesis_manager(user_options, functions_structure, product_structure);
		output_streamer.output(verbose, "Coloring started.", OutputStreamer::important);
		synthesis_manager.doSynthesis();
	} 
	catch (std::exception & e) {
		output_streamer.output(fail, std::string("Error occured while syntetizing the parameters: ").append(e.what()));
		return 7;
	}

	return 0;
}
