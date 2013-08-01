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
#include "data_parser.hpp"
#include "bitmask_manager.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief STEP 1 - Class that manages all of the parsing done by the application. Includes parsing of arguments and parsing of models.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace ParsingManager {
   /**
    * Main parsing function.
    */
   void parse(int argc, char* argv[], Model & model, vector<PropertyAutomaton> & properties) {
      ifstream model_stream; // Object that will reference input file.
      vector<string> arguments;
      for (int argn = 0; argn < argc; argn++) {
         arguments.push_back(argv[argn]);
      }

      // Parse arguments
      ArgumentParser parser;
      parser.parseArguments(arguments, model_stream);
      user_options.addDefaultFiles();

      // Open datafiles that were requested by the user.
      if (user_options.inputMask()) {
         bitmask_manager.openFile(user_options.inMaskFile());
         bitmask_manager.parseMask();
      }
      if (user_options.outputMask()) {
         bitmask_manager.createOutput(user_options.outMaskFile());
      }
      if (user_options.toFile()) {
         output_streamer.createStreamFile(results_str, user_options.textFile());
      }
      if(user_options.toDatabase()) {
         sql_adapter.setDatabase(user_options.dataFile());
      }

      // Parse model itself
      DataParser data_parser;
      model = data_parser.parseNetwork(&model_stream);
      // Currently reads property from the model file.
      properties = data_parser.parseProperties(&model_stream);
   }
}

#endif // PARSYBONE_PARSING_MANAGER_INCLUDED
