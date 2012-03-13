/*
 * Copyright (C) 2012 - Adam Streck
 *
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 *
 * ParSyBoNe is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 *
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno.
 * See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_ARGUMENT_PARSER_INCLUDED
#define PARSYBONE_ARGUMENT_PARSER_INCLUDED

#include <fstream>

#include <boost/lexical_cast.hpp>

#include "../auxiliary/data_types.hpp"

class ArgumentParser {
public:
	/**
	 * @param _user_options	parsed data will be saved here
	 */
	void parseArguments (UserOptions & user_options, int argc, char* argv[], std::istream * input_stream, 
		                 std::ifstream & _input_file, std::ostream * output_stream, std::ofstream & output_file) {
					// Parse arguments
		std::string switches;
		std::ifstream * input_file = new std::ifstream;

		for (int arg_n = 1; arg_n < argc; arg_n++) { 
			std::string arg = argv[arg_n];
			
			// Control basic switches
			if (arg[0] == '-') {
				for (std::size_t switch_num = 1; switch_num < arg.size(); switch_num++) {
					switch (arg[switch_num]) {
						case 'b':
							user_options.show_base_coloring = true;
						break;

						case 'c':
							user_options.show_counterexamples = true;
						break;

						case 'v':
							user_options.verbose = true;
						break;

						case 'n':
							user_options.negative_check = true;
						break;

						// Get data for distributed computation
						case 'd':
							if (switch_num + 1 < arg.size())
								throw(std::runtime_error(std::string("There are forbidden characters after d switch: ").append(arg.begin() + switch_num + 1, arg.end())));
							try {
								user_options.this_ID = boost::lexical_cast<std::size_t, std::string>(argv[arg_n+1]);
								user_options.total_count = boost::lexical_cast<std::size_t, std::string>(argv[arg_n+2]);
							} catch (boost::bad_lexical_cast & e) {
								std::invalid_argument(std::string("Wrong parameters for the switch -d:").append(argv[arg_n+1]).append(" ").append(argv[arg_n+2]).append(". Should be -d this_ID number_of_parts."));
								throw(std::runtime_error(std::string("Parameter parsing failed.").append(e.what())));
							}
							if (user_options.this_ID > user_options.total_count)
								throw(std::runtime_error("Terminal failure - ID of the process is bigger than number of processes."));
							arg_n += 2;
						break;
						
						// Redirecting input to a file by a parameter
						case 'f':
							if (switch_num + 1 < arg.size())
								throw(std::runtime_error(std::string("There are forbidden characters after f switch: ").append(arg.begin() + switch_num + 1, arg.end())));
							input_file->open(argv[++arg_n], std::ios::in);
							if (input_file->fail() || !input_file->is_open())
								throw (std::invalid_argument(std::string("Wrong input filename: ").append(argv[arg_n]).c_str()));
							input_stream = input_file;
						break;

						// Redirecting output to a file by a parameter
						case 'F':
							if (switch_num + 1 < arg.size())
								throw(std::runtime_error(std::string("There are forbidden characters after F switch: ").append(arg.begin() + switch_num + 1, arg.end())));
							output_file.open(argv[++arg_n], std::ios::out);
							if (output_file.fail())
								throw (std::invalid_argument(std::string("Wrong output filename: ").append(argv[arg_n]).c_str()));
							output_stream = &output_file;
						break;

						// If the swich is not known.
						default:
							throw (std::invalid_argument(std::string("Wrong argument: -").append(arg).c_str()));
						break;
					}
				}
			}
			else 
				throw (std::invalid_argument(std::string("Wrong argument: ").append(arg).c_str()));
		}		
	}
};

#endif