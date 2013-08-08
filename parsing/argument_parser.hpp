/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_ARGUMENT_PARSER_INCLUDED
#define PARSYBONE_ARGUMENT_PARSER_INCLUDED

#include "../auxiliary/output_streamer.hpp"
#include "../auxiliary/user_options.hpp"
#include "bitmask_manager.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief A class responsible for reading the arguments on the input.
///
/// A sets user options according to the string provided as arguments at the start of the program.
/// All values that are not used for direct setup are stored within a UserOptions class.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ArgumentParser {
   enum Filetype {database, datatext, input_mask, output_mask};

   /**
    * Obtain parameters for synthesis distribution.
    */
   int getDistribution(vector<string>::const_iterator position, const vector<string>::const_iterator & end) {
      // Get numbers
      try {
         if (++position == end)
            throw invalid_argument("The number of processes and the total count of processes are missing");
         user_options.process_number = lexical_cast<size_t>(*position);
         if (++position == end)
            throw invalid_argument("The number of processes or the total count of processes is missing");
         user_options.processes_count = lexical_cast<size_t>(*position);
      } catch (bad_lexical_cast & e) {
         throw invalid_argument("Error while parsing the modifier --dist" + toString(e.what()));
      }

      // Assert that process ID is in the range
      if (user_options.process_number > user_options.processes_count)
         throw runtime_error("Error while parsing the modifier --dist - ID of the process is bigger than number of processes");

      return 2;
   }

   /**
    * Obtain parameters for synthesis distribution.
    */
   int getDistribution(vector<string>::const_iterator position, const vector<string>::const_iterator & end) {
      try {
         if (++position == end)
            throw invalid_argument("Bound values is missing");
         user_options.step_bound = lexical_cast<size_t>(*position);
      } catch (bad_lexical_cast & e) {
         throw invalid_argument("Error while parsing the modifier --bound" + toString(e.what()));
      }

      return 1;
   }

   /**
    * @brief getFileName   stores path to a file based on its type in user options
    * @param filetype
    * @param position
    * @param end
    * @return how many arguments you have used
    */
   int getFileName(const Filetype & filetype, vector<string>::const_iterator position, const vector<string>::const_iterator & end) {
      if (++position == end) {
         throw invalid_argument("Filename missing after the modifier " + *(--position));
         return 0;
      }

      switch (filetype) {
      case database:
         user_options.database_file = *position;
         user_options.use_database = true;
         break;
      case datatext:
         user_options.datatext_file = *position;
         user_options.use_textfile = true;
         break;
      case input_mask:
         user_options.in_mask_file = *position;
         user_options.use_in_mask = true;
         break;
      case output_mask:
         user_options.out_mask_file = *position;
         user_options.use_out_mask = true;
         break;
      };

      return 1;
   }

   /**
    * Get the position of the specified argument.
    */
   const vector<string>::const_iterator getArgumentPosition(const string & argument, const vector<string> & arguments) {
      // Get the position of the current modifier.
      auto position = arguments.begin();
      for(auto arg:arguments) {
         if (argument.compare(arg) != 0)
            position++;
         else
            return position;
      }
      throw runtime_error("Argument not found in arguments (internal error).");
   }

   /**
    * Get the full-text modifiers.
    */
   int parseModifier(const string & modifier, const vector<string> & arguments) {
      auto position = getArgumentPosition(modifier, arguments);

      // Apply the modifier.
      if (position->compare("--dist") == 0) {
         return getDistribution(position, arguments.end());
      } else if (position->compare("--text") == 0) {
         user_options.use_textfile = true;
         return getFileName(datatext, position, arguments.end());
      } else if (position->compare("--data") == 0) {
         user_options.use_database = true;
         return getFileName(database, position, arguments.end());
      } else if (position->compare("--min") == 0) {
         user_options.use_in_mask = true;
         return getFileName(input_mask, position, arguments.end());
      } else if (position->compare("--mout") == 0) {
         user_options.use_out_mask = true;
         return getFileName(output_mask, position, arguments.end());
      } else if (position->compare("--bound") == 0) {
         user_options.bounded_check = true;
         return getBound(output_mask, position, arguments.end());
      } else {
         throw invalid_argument("Unknown modifier " + *position);
      }
   }

   /**
    * Function that parses switches in the string that starts with a "-" symbol.
    *
    * @param argument	iterator pointer to the string to read
    */
   void parseSwitch(const char s){
      switch (s) {
      case 'W':
         user_options.use_long_witnesses = true;

      case 'w':
         user_options.compute_wintess = true;
         break;

      case 'r':
         user_options.compute_robustness = true;
         break;

      case 'v':
         user_options.be_verbose = true;
         break;

      case 'm':
         user_options.use_in_mask = true;
         break;

      case 'M':
         user_options.use_out_mask = true;
         break;

      case 'f':
         user_options.use_textfile = true;
         break;

      case 'd':
         user_options.use_database = true;
         break;

      case 'c':
         user_options.output_console = true;
         break;

      case 'b':
         user_options.bounded_check = true;
         break;

      default:
         throw invalid_argument("Unknown switch -" + toString(s));
         break;
      }
   }

   /**
     * @brief referenceModel save the model name and connect the given file to its stream.
     * @param path  path to model
     * @param model_stream
     */
   void referenceModel(const string & path, ifstream & model_stream) {
      // If the model is alredy parsed.
      if (!user_options.model_name.empty())
         throw invalid_argument("Model file (file with a .dbm suffix) occurs multiple times on the input, only a single occurence is allowed");

      // Attach the stream to the file.
      model_stream.open(path, ios::in);
      if (model_stream.fail())
         throw runtime_error("Program failed to open an intput stream file: " + path);

      // Store the models name.
      auto pos1 = path.find_last_of("/\\") + 1; // Remove the prefix (path), if there is any.
      auto pos2 = path.find(MODEL_SUFFIX) - pos1; // Remove the suffix.
      user_options.model_path = path.substr(0, pos1);
      user_options.model_name = path.substr(pos1, pos2);
   }

public:
   /**
    * Take all the arguments on the input and store information from them.
    *
    * @param argc value passed from main function
    * @param argv	value passed from main function
    * @param intput_stream pointer to a file that will be used as an input stream
    */
   void parseArguments (const vector<string> & arguments, ifstream & model_stream) {
      int skip = 1;

      // Cycle through arguments (skip the first - name of the program).
      for (auto argument:arguments) {
         if (skip-- > 0)
            continue;
         // There can be multiple switches after "-" so go through them in the loop.
         else if (argument[0] == '-' && argument[1] != '-') {
            for (char s:argument.substr(1)) {
               parseSwitch(s);
            }
         }
         else if (argument[0] == '-' && argument[1] == '-') {
            skip = parseModifier(argument, arguments);
         }
         // If it is a model file.
         else if (argument.find(MODEL_SUFFIX) != string::npos) {
            referenceModel(argument, model_stream);
         }
         else {
            throw runtime_error("Wrong argument on the input stream: " +  argument);
         }
      }

      // Throw an exception if no model file was found.
      if (user_options.model_name.empty())
         throw (invalid_argument("Model file (file with a .dbm suffix) is missing"));
   }
};

#endif // PARSYBONE_ARGUMENT_PARSER_INCLUDED
