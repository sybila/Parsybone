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
#include "argument_parser.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief STEP 1 - Class that manages all of the parsing done by the application. Includes parsing of arguments and parsing of models.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace ParsingManager {
   /**
    * Main parsing function.
    */
   void parseOptions(int argc, char* argv[]) {
      vector<string> arguments;
      for (int argn = 0; argn < argc; argn++) {
         arguments.push_back(argv[argn]);
      }

      // Parse arguments
      ArgumentParser parser;
      parser.parseArguments(arguments);
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
   }

   Model parseModel(const string filename) {
      DataParser data_parser;
      ifstream file(filename, ios::in);
      return data_parser.parseNetwork(file);
   }

   PropertyAutomaton parseProperty(const string filename) {
      DataParser data_parser;
      ifstream file(filename, ios::in);
      return data_parser.parseProperty(file);
   }
}

#endif // PARSYBONE_PARSING_MANAGER_INCLUDED
