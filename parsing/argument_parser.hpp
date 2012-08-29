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
   void getDistribution(std::string procn, std::string procc) {
		// Get numbers
		try {
			user_options.process_number = boost::lexical_cast<std::size_t, std::string>(procn);
         user_options.processes_count = boost::lexical_cast<std::size_t, std::string>(procc);
		} catch (boost::bad_lexical_cast & e) {
         std::invalid_argument(std::string("Wrong parameters for the switch -d:").append(procn).append(" ").append(procc).append(". Should be -d proc_number proc_count."));
         throw(std::runtime_error(std::string("Parameter parsing failed").append(e.what())));
		}
		// Assert that process ID is in the range
		if (user_options.process_number > user_options.processes_count)
         throw(std::runtime_error("Terminal failure - ID of the process is bigger than number of processes"));
	}

	/**
	 * Some of the switches must be followed by additional argument (i.e. filename).
	 * Such a switch must be last in the string of switches, or is not valid. This function controls it.
	 *
    * @return  true if the position is valid, false otherwise
	 */
   bool testLast(const std::size_t position, const std::size_t size) {
		if (position + 1 != size) {
			throw(std::runtime_error(std::string("There are forbidden characters after some switch")));
		}
		return true;
	}

   /**
    * Some of the switches must be followed by additional argument (i.e. filename).
    * Such a switch must be followed by parametrizing arguments. This function controls that.
    *
    * @return  true if the arguments are present, false otherwise
    */
   bool testFollowers(std::vector<std::string>::const_iterator argument, const std::vector<std::string>::const_iterator & last_pos, const std::size_t followers_count = 1) {
      for(std::size_t foll_num = 0; foll_num < followers_count; foll_num++, argument++) {
         if (argument == last_pos) {
            throw(std::runtime_error(std::string("There are missing parametrizing arguments after some switch")));
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
   void parseSwitches(std::vector<std::string>::const_iterator & argument, const std::vector<std::string>::const_iterator & last_pos){
		for (std::size_t switch_num = 1; switch_num < argument->size(); switch_num++) {
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
				throw (std::invalid_argument(std::string("Wrong argument: -").append(*argument).c_str()));
				return;
			}
		}
	}

public:
	/**
    * Take all the arguments on the input and store information from them.
	 *
    * @param argc passed from main function
	 * @param argv	passed from main function
    * @param intput_stream pointer to a file that will be used as an input stream
	 */
	void parseArguments (const std::vector<std::string> & arguments, std::ifstream & input_stream) {
      // True after name of the model file is read
		bool file_parsed = false;

      // Cycle through arguments (skip the first - name of the program)
      for (auto argument = arguments.begin() + 1; argument != arguments.end(); argument++) {
			// There can be multiple switches after "-" so go through them in the loop
			if ((*argument)[0] == '-') {
            parseSwitches(argument, arguments.end());
			}
			// If it is a model file
			else if (argument->find(".dbm") != std::string::npos){
				// If the model is alredy parsed
				if (file_parsed)
               throw (std::invalid_argument("Model file (file with a .dbm suffix) occurs multiple times on the input, only a single occurence is allowed"));
				// Attach the stream to the file
				input_stream.open(*argument, std::ios::in);
				if (input_stream.fail())
					throw std::runtime_error(std::string("Program failed to open an intput stream file: ").append(*argument));
				file_parsed = true;
         } else {
            throw std::runtime_error(std::string("Wrong argument on the input stream: ").append(*argument));
         }
		}	

      // Throw an exception if no model file was found
		if (!file_parsed)
         throw (std::invalid_argument("Model file (file with a .dbm suffix) is missing"));
	}
};

#endif
