/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_ARGUMENT_PARSER_INCLUDED
#define PARSYBONE_ARGUMENT_PARSER_INCLUDED

#include "../auxiliary/output_streamer.hpp"
#include "../auxiliary/user_options.hpp"
#include "coloring_parser.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief A class responsible for reading the arguments on the input.
///
/// A sets user options according to the string provided as arguments at the start of the program.
/// All values that are not used for direct setup are stored within a UserOptions class.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ArgumentParser {
   /**
    * Obtain parameters for synthesis distribution.
    *
    * @param procn   ID of this process (counting from 1)
    * @param procc   total number of processes in computation
    */
   void getDistribution(string procn, string procc) {
      // Get numbers
      try {
         user_options.process_number = lexical_cast<size_t, string>(procn);
         user_options.processes_count = lexical_cast<size_t, string>(procc);
      } catch (bad_lexical_cast & e) {
         invalid_argument(string("Wrong parameters for the switch -d:").append(procn).append(" ").append(procc).append(". Should be -d proc_number proc_count."));
         throw(runtime_error(string("Parameter parsing failed").append(e.what())));
      }
      // Assert that process ID is in the range
      if (user_options.process_number > user_options.processes_count)
         throw(runtime_error("Terminal failure - ID of the process is bigger than number of processes"));
   }

	/**
	 * Some of the switches must be followed by additional argument (i.e. filename).
	 * Such a switch must be last in the string of switches, or is not valid. This function controls it.
	 *
	 * @return  true if the position is valid, false otherwise
	 */
	bool testLast(const size_t position, const size_t size) {
		if (position + 1 != size) {
			throw(runtime_error(string("There are forbidden characters after some switch")));
		}
		return true;
	}

   /**
    * Some of the switches must be followed by additional argument (i.e. filename).
    * Such a switch must be followed by parametrizing arguments. This function controls that.
    *
    * @return  true if the arguments are present, false otherwise
    */
   bool testFollowers(vector<string>::const_iterator argument, const vector<string>::const_iterator & last_pos, const size_t followers_count = 1) {
      for(size_t foll_num = 0; foll_num < followers_count; foll_num++, argument++) {
         if (argument == last_pos) {
            throw(runtime_error(string("There are missing parametrizing arguments after some switch")));
            return false;
         }
      }
      return true;
   }

   /**
    * Function that parses switches in the string that starts with a "-" symbol.
    *
    * @param argument	iterator pointer to the string to read
    */
   void parseSwitches(vector<string>::const_iterator & argument, const vector<string>::const_iterator & last_pos){
      for (size_t switch_num = 1; switch_num < argument->size(); switch_num++) {
         switch ((*argument)[switch_num]) {

			case 'W':
				user_options.use_long_witnesses = true;

			case 'w':
				user_options.compute_wintess = true;
				break;

			case 'r':
				user_options.compute_robustness = true;
				break;

			case 's':
				user_options.display_stats = true;
				break;

			case 'v':
				user_options.be_verbose = true;
				break;

				// Open file to read a color mask
			case 'm':
				testLast(switch_num, argument->size()); testFollowers(argument + 1, last_pos);
				coloring_parser.openFile(*(++argument));
				user_options.use_in_mask = true;
				return;

				// Open file to fill a color mask
			case 'M':
				testLast(switch_num, argument->size()); testFollowers(argument + 1, last_pos);
				coloring_parser.createOutput(*(++argument));
				user_options.use_out_mask = true;
				return;

			// Get data for distributed computation
			case 'd':
				testLast(switch_num, argument->size()); testFollowers(argument + 1, last_pos, 2);
				getDistribution(*(argument+1), *(argument+2)); argument += 2;
				return;

			// Redirecting results output to a file by a parameter
			case 'f':
				testLast(switch_num, argument->size()); testFollowers(argument + 1, last_pos);
				output_streamer.createStreamFile(results_str, *(++argument));
				return;

			// If the switch is not known.
			default:
				throw (invalid_argument(string("Wrong argument: -").append(*argument).c_str()));
				return;
			}
		}
	}

public:
   /**
    * Take all the arguments on the input and store information from them.
    *
    * @param argc value passed from main function
    * @param argv	value passed from main function
    * @param intput_stream pointer to a file that will be used as an input stream
    */
   void parseArguments (const vector<string> & arguments, ifstream & input_stream) {
      // Cycle through arguments (skip the first - name of the program).
      for (auto argument = arguments.begin() + 1; argument != arguments.end(); argument++) {
         // There can be multiple switches after "-" so go through them in the loop.
         if (regex_match(*argument, regex("-.*"))) {
            parseSwitches(argument, arguments.end());
         }
         // If it is a model file.
         else if (argument->find(MODEL_SUFFIX) != string::npos){
            // If the model is alredy parsed.
            if (!user_options.model_name.empty())
               throw (invalid_argument("Model file (file with a .dbm suffix) occurs multiple times on the input, only a single occurence is allowed"));

				// Attach the stream to the file.
				input_stream.open(*argument, ios::in);
				if (input_stream.fail())
					throw runtime_error(string("Program failed to open an intput stream file: ").append(*argument));

            // Store the models name.
            auto pos1 = argument->find_last_of("/\\") + 1;
            auto pos2 = argument->find(MODEL_SUFFIX) - pos1;
            user_options.model_name = argument->substr(pos1, pos2);
         } else {
            throw runtime_error(string("Wrong argument on the input stream: ").append(*argument));
         }
      }

		// Throw an exception if no model file was found.
		if (user_options.model_name.empty())
			throw (invalid_argument("Model file (file with a .dbm suffix) is missing"));
	}
};

#endif // PARSYBONE_ARGUMENT_PARSER_INCLUDED
