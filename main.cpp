/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This is the entry point of the program and also only .cpp file used.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "auxiliary/time_manager.hpp"
#include "auxiliary/output_streamer.hpp"
#include "auxiliary/user_options.hpp"
#include "parsing/argument_parser.hpp"
#include "parsing/model_parser.hpp"
#include "parsing/parsing_manager.hpp"
#include "construction/basic_structure_builder.hpp"
#include "construction/constrains_parser.hpp"
#include "construction/construction_holder.hpp"
#include "construction/construction_manager.hpp"
#include "construction/parametrizations_builder.hpp"
#include "construction/parametrized_structure_builder.hpp"
#include "construction/automaton_builder.hpp"
#include "construction/product_structure.hpp"
#include "construction/product_builder.hpp"
#include "coloring/synthesis_manager.hpp"

// porgram-related data
const float program_version = 1.0;

/**
 * Linear execution of succesive parts of the parameter synthesis
 */
int main(int argc, char* argv[]) {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STEP ONE:
// Create long-live objects
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	time_manager.startClock("runtime");
	ConstructionHolder holder;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STEP ONE:
// Parse input information.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	try {
		Model model;
		ParsingManager parsing_manager(argc, argv, model);
		parsing_manager.parse();
		holder.setModel(std::move(model));
	}
	catch (std::exception & e) {
		output_streamer.output(error_str, std::string("Error occured while parsing input: ").append(e.what()));
		return 1;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STEP ONE:
// Parse input information.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	try {
		ConstructionManager construction_manager(holder.getModel());
		construction_manager.construct();
	}
	catch (std::exception & e) {
		output_streamer.output(error_str, std::string("Error occured while constructing data structures: ").append(e.what()));
		return 2;
	}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STEP THREE:
// Create Functions (implicit reprezentation of regulatory contexts of species)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ParametrizationsHolder parametrizations_holder;
	ConstrainsParser constrains_parser(holder.getModel());
	try {
		output_streamer.output(verbose_str, "Functions building started.", OutputStreamer::important);

		constrains_parser.parseConstrains();

		ParametrizationsBuilder parametrizations_builder(holder.getModel(), constrains_parser, parametrizations_holder);
		parametrizations_builder.buildFunctions();
	} 
	catch (std::exception & e) {
		output_streamer.output(error_str, std::string("Error occured while building Regulatory functions: ").append(e.what()));
		return 3;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STEP FOUR:
// Create the Kripke Structure (product of all activation value of all the species)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ParametrizedStructure parametrized_structure; // Kripke structure that has transitions labelled with functions
	try {
		output_streamer.output(verbose_str, "Parametrized Kripke structure building started.", OutputStreamer::important);

		// Create temporary Kripke structure without parametrization
		BasicStructure basic_structure; // Kripke structure built from the network
		BasicStructureBuilder basic_structure_builder(holder.getModel(), basic_structure);
		basic_structure_builder.buildStructure();

		// Build PKS
		ParametrizedStructureBuilder parametrized_structure_builder(basic_structure, parametrizations_holder, parametrized_structure);
		parametrized_structure_builder.buildStructure();
	} 
	catch (std::exception & e) {
		output_streamer.output(error_str, std::string("Error occured while building Parametrized Kripke structure: ").append(e.what()));
		return 4;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STEP FIVE:
// Create the automaton.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	AutomatonStructure automaton; // Set of transitions - controlling automaton
	try {
		output_streamer.output(verbose_str, "Buchi automaton building started.", OutputStreamer::important);

		AutomatonBuilder automaton_builder( holder.getModel(), automaton);
		automaton_builder.buildAutomaton();
	} 
	catch (std::exception & e) {
		output_streamer.output(error_str, std::string("Error occured while building the automaton: ").append(e.what()));
		return 5;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STEP SIX:
// Create the product - splitted into two parts
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ProductStructure product_structure(parametrizations_holder, constrains_parser, parametrized_structure, automaton);
	ColorStorage color_storage;
	try {
		output_streamer.output(verbose_str, "Product building started.", OutputStreamer::important);

		ProductBuilder product_builder(parametrized_structure, automaton, product_structure, color_storage);
		product_builder.buildProduct();
	} catch (std::exception & e) {
		output_streamer.output(error_str, std::string("Error occured while building the product: ").append(e.what()));
		return 6;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STEP SEVEN:
// COMPUTATION:
// Synthetize the colors and output them
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	try {
		SynthesisManager synthesis_manager(product_structure, color_storage);
		output_streamer.output(verbose_str, "Coloring started.", OutputStreamer::important);
		synthesis_manager.doSynthesis();
	} 
	catch (std::exception & e) {
		output_streamer.output(error_str, std::string("Error occured while syntetizing the parameters: ").append(e.what()));
		return 7;
	}

	time_manager.ouputClock("runtime");
	return 0;
}
