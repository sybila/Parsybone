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
#include "../synthesis/SQLAdapter.hpp"
#include "model_parser.hpp"
#include "coloring_parser.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief STEP 1 - Class that manages all of the parsing done by the application. Includes parsing of arguments and parsing of models.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ParsingManager {
   vector<string> arguments; ///< Vector containing individual arguments from the input.
   Model & model; ///< Model object that will contain all the parsed information from the *.dbm file.

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // CONSTRUCTION METHODS
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   ParsingManager(const ParsingManager & other); ///< Forbidden copy constructor.
   ParsingManager& operator=(const ParsingManager & other); ///< Forbidden assignment operator.

public:
   /**
    * Constructor copies arguments from the argv and passes the model object that will store parsed information.
    *
    * @param argc passed argc from main()
    * @param argv passed argv from main()
    * @param _model  model storing the reference data
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
      ifstream model_stream; // Object that will reference input file.

      output_streamer.output(verbose_str, "Arguments parsing started.", OutputStreamer::important);

      // Parse arguments
      ArgumentParser parser;
      parser.parseArguments(arguments, model_stream);
      addDefaultFiles();

      // Open datafiles that were requested by the user.
      if (user_options.use_out_mask) {
         coloring_parser.openFile(user_options.in_mask_file);
         coloring_parser.parseMask();
      }
      if (user_options.use_in_mask) {
         coloring_parser.createOutput(user_options.out_mask_file);
      }
      if (user_options.output_file) {
         output_streamer.createStreamFile(results_str, user_options.datatext_file);
      }
      if(user_options.output_database) {
         database_output.setDatabase(user_options.database_file);
      }

      // Parse model itself
      output_streamer.output(verbose_str, "Model parsing started.", OutputStreamer::important);
      ModelParser model_parser(model, &model_stream);
      model_parser.parseInput();
   }
};

#endif // PARSYBONE_PARSING_MANAGER_INCLUDED
