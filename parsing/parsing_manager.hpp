/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_PARSING_MANAGER_INCLUDED
#define PARSYBONE_PARSING_MANAGER_INCLUDED

#include "../auxiliary/data_types.hpp"
#include "model_parser.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Class that manages all of the parsing done by the application. Icludes parsing of arguments and parsing of models.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ParsingManager {
   std::vector<std::string> arguments; ///< vector containing single arguments from the input
   Model & model; ///< Model object that will contain all the parsed information from the .dbm file

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ParsingManager(const ParsingManager & other); ///< Forbidden copy constructor.
	ParsingManager& operator=(const ParsingManager & other); ///< Forbidden assignment operator.

public:
   /**
    * Constructor copies arguments from the argv and passes the model object that will store parsed information.
    */
   ParsingManager(int argc, char* argv[], Model & _model) : model(_model) {
      for (int argn = 0; argn < argc; argn++) {
         arguments.push_back(argv[argn]);
      }
   }

   /**
    * Main parsing function.
    */
   void parse() {
      std::ifstream input_stream; // Object that will reference input file

      output_streamer.output(verbose_str, "Argument parsing started.", OutputStreamer::important);

		// Parse arguments
		ArgumentParser parser;
		parser.parseArguments(arguments, input_stream);

		// Parse mask if necessary
		if (coloring_parser.input())
			coloring_parser.parseMask();

		output_streamer.output(verbose_str, "Model parsing started.", OutputStreamer::important);

		// Parse model itself
		ModelParser model_parser(model, &input_stream);
		model_parser.parseInput();
	}
};

#endif // PARSYBONE_PARSING_MANAGER_INCLUDED
