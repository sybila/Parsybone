/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_PARSING_MANAGER_INCLUDED
#define PARSYBONE_PARSING_MANAGER_INCLUDED

#include "../auxiliary/data_types.hpp"
#include "../synthesis/SQLAdapter.hpp"
#include "model_parser.hpp"
#include "bitmask_manager.hpp"

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
   vector<PropertyAutomaton> parse() {
      ifstream model_stream; // Object that will reference input file.

      // Parse arguments
      ArgumentParser parser;
      parser.parseArguments(arguments, model_stream);
      user_options.addDefaultFiles();

      // Open datafiles that were requested by the user.
      if (user_options.use_in_mask) {
         bitmask_manager.openFile(user_options.in_mask_file);
         bitmask_manager.parseMask();
      }
      if (user_options.use_out_mask) {
         bitmask_manager.createOutput(user_options.out_mask_file);
      }
      if (user_options.use_textfile) {
         output_streamer.createStreamFile(results_str, user_options.datatext_file);
      }
      if(user_options.use_database) {
         sql_adapter.setDatabase(user_options.database_file);
      }

      // Parse model itself
      ModelParser model_parser(model, &model_stream);
      return  model_parser.parseInput();
   }
};

#endif // PARSYBONE_PARSING_MANAGER_INCLUDED
