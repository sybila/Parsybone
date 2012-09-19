/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Parsybone tool version 1.0.
/// @file This is the entry point of the program and also the only .cpp file used.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "auxiliary/time_manager.hpp"
#include "auxiliary/output_streamer.hpp"
#include "auxiliary/user_options.hpp"
#include "parsing/argument_parser.hpp"
#include "parsing/model_parser.hpp"
#include "parsing/parsing_manager.hpp"
#include "construction/construction_manager.hpp"
#include "construction/product_builder.hpp"
#include "synthesis/synthesis_manager.hpp"

/**
 * Linear execution of succesive parts of the parameter synthesis.
 */
int main(int argc, char* argv[]) {
	time_manager.startClock("runtime");
    ConstructionHolder holder; ///< Object that will hold all the constructed data structures that are used as reference.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STEP ONE:
// Parse input information.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	try {
		Model * model = new Model;
		ParsingManager parsing_manager(argc, argv, *model);
		output_streamer.output(verbose_str, "Parsing started.");
		parsing_manager.parse();
		// Pass the model
		holder.fillModel(model);
	}
	catch (std::exception & e) {
      output_streamer.output(error_str, std::string("Error occured while parsing input: \"").append(e.what()).append("\"."));
		return 1;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STEP TWO:
// Construct data structures that correspond to formal verification structures.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	try {
		ConstructionManager construction_manager(holder);
		output_streamer.output(verbose_str, "Construction started.");
		construction_manager.construct();
	}
	catch (std::exception & e) {
      output_streamer.output(error_str, std::string("Error occured while constructing data structures: \"").append(e.what()).append("\"."));
		return 2;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STEP THREE:
// Synthetize the colors and output them
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	try {
		SynthesisManager synthesis_manager(holder);
		output_streamer.output(verbose_str, "Coloring started.");
		synthesis_manager.doSynthesis();
	} 
	catch (std::exception & e) {
      output_streamer.output(error_str, std::string("Error occured while syntetizing the parameters: \"").append(e.what()).append("\"."));
		return 3;
	}

	time_manager.ouputClock("runtime");
	return 0;
}
