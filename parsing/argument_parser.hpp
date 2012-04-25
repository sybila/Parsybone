/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_ARGUMENT_PARSER_INCLUDED
#define PARSYBONE_ARGUMENT_PARSER_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// A sets user options according to the switches provided as arguments at the start of the program.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../auxiliary/output_streamer.hpp"
#include "../auxiliary/user_options.hpp"
#include "coloring_parser.hpp"

/**
	* Parses arguments on the input and changes switches accordingly. If there is a file on the input, it is created.
	*
	* @param user_options	parsed data will be saved here
	* @param argc	same as at main
	* @param argv	same as at main
	* @param result_stream	pointer to the stream that will get the output
	*/
class ArgumentParser {

	/**
	 * Obtains numbers for distributed synthesis
	 */
	void getDistribution(char * argv [], int & arg_n) {
		// Get numbers
		try {
			user_options.process_number = boost::lexical_cast<std::size_t, std::string>(argv[arg_n+1]);
			user_options.processes_count = boost::lexical_cast<std::size_t, std::string>(argv[arg_n+2]);
		} catch (boost::bad_lexical_cast & e) {
			std::invalid_argument(std::string("Wrong parameters for the switch -d:").append(argv[arg_n+1]).append(" ").append(argv[arg_n+2]).append(". Should be -d this_ID number_of_parts."));
			throw(std::runtime_error(std::string("Parameter parsing failed.").append(e.what())));
		}
		// Assert that process ID is in the range
		if (user_options.process_number > user_options.processes_count)
			throw(std::runtime_error("Terminal failure - ID of the process is bigger than number of processes."));
		// Skip arguments that are already read
		arg_n += 2;
	}

public:
	/**
	 * Take all the arguments on the input and store information from them
	 */
	void parseArguments (UserOptions & user_options, int argc, char* argv[]) {

		std::string switches;

		for (int arg_n = 1; arg_n < argc; arg_n++) { 
			std::string arg = argv[arg_n];
			
			// There can be multiple switches after "-" so go through them in the loop
			if (arg[0] == '-') {
				for (std::size_t switch_num = 1; switch_num < arg.size(); switch_num++) {
					switch (arg[switch_num]) {

					case 'b':
						user_options.add_BA_to_witness = true;
						break;

					case 'c':
						user_options.show_coloring = true;
						break;

					case 'n':
						user_options.negation_check = true;
						break;

					case 's':
						user_options.display_stats = true;
						break;

					case 't':
						user_options.time_serie = true;
						break;

					case 'v':
						user_options.be_verbose = true;
						break;

					case 'w':
						user_options.witness_use = short_wit;
						user_options.time_serie = true;
						break;

					case 'W':
						user_options.witness_use = all_wit;
						user_options.time_serie = true;
						break;

					// Open file with color mask
					case 'm':
						if (switch_num + 1 < arg.size())
							throw(std::runtime_error(std::string("There are forbidden characters after m switch: ").append(arg.begin() + switch_num + 1, arg.end())));
						coloring_parser.openFile(argv[++arg_n]);
						break;

										// Open file with color mask
					case 'M':
						if (switch_num + 1 < arg.size())
							throw(std::runtime_error(std::string("There are forbidden characters after M switch: ").append(arg.begin() + switch_num + 1, arg.end())));
						coloring_parser.createOutput(argv[++arg_n]);
						break;

					// Get data for distributed computation
					case 'D':
						// After d there must be a white space (to distinct requsted numbers)
						if (switch_num + 1 < arg.size())
							throw(std::runtime_error(std::string("There are forbidden characters after d switch: ").append(arg.begin() + switch_num + 1, arg.end())));
						getDistribution(argv, arg_n);
						break;

					// Redirecting results output to a file by a parameter
					case 'F':
						// After f there must be a white space (to distinct file name)
						if (switch_num + 1 < arg.size())
							throw(std::runtime_error(std::string("There are forbidden characters after f switch: ").append(arg.begin() + switch_num + 1, arg.end())));
						// Create file for the result output and iterate argument pointer
						output_streamer.createStreamFile(results_str, argv[++arg_n]);
						break;

					// If the swich is not known.
					default:
						throw (std::invalid_argument(std::string("Wrong argument: -").append(arg).c_str()));
						break;
					}
				}
			}
			// If there is an argument that does not start with "-"
			else 
				throw (std::invalid_argument(std::string("Wrong argument: ").append(arg).c_str()));
		}	
	}
};


#endif