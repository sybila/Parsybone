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
	 * Obtains numbers for distributed synthesis
	 *
	 * @param argv	reference to the original array of c-strings
	 * @param arg_n	ordinary number of the previous argument (D-switch)
	 */
	void getDistribution(std::string procn, std::string procv) {
		// Get numbers
		try {
			user_options.process_number = boost::lexical_cast<std::size_t, std::string>(procn);
			user_options.processes_count = boost::lexical_cast<std::size_t, std::string>(procv);
		} catch (boost::bad_lexical_cast & e) {
			std::invalid_argument(std::string("Wrong parameters for the switch -d:").append(procn).append(" ").append(procv).append(". Should be -d this_ID number_of_parts."));
			throw(std::runtime_error(std::string("Parameter parsing failed.").append(e.what())));
		}
		// Assert that process ID is in the range
		if (user_options.process_number > user_options.processes_count)
			throw(std::runtime_error("Terminal failure - ID of the process is bigger than number of processes."));
		// Skip arguments that are already read
	}

	/**
	 * Some of the switches must be followed by additional argument (i.e. filename).
	 * Such a switch must be last in the string of switches, or is not valid. This function controls it.
	 *
	 * @return true if the position is valid, false otherwise
	 */
	bool testLast(const std::size_t position, const std::size_t size) {
		if (position + 1 != size) {
			throw(std::runtime_error(std::string("There are forbidden characters after some switch")));
		}
		return true;
	}

	/**
	 * Function that parses switches in the string that starts with a "-" symbol
	 *
	 * @param argument	iterator pointer to the string to read
	 */
	void parseSwitches(std::vector<std::string>::const_iterator & argument){
		for (std::size_t switch_num = 1; switch_num < argument->size(); switch_num++) {
			switch ((*argument)[switch_num]) {

			case 'b':
				user_options.add_BA_to_witness = true;
				break;

			case 'c':
				user_options.show_coloring = true;
				break;

			case 'n':
				user_options.negation_check = true;
				break;

			case 'r':
				if (user_options.witness_use == none_wit)
					user_options.witness_use = short_wit;
				user_options.compute_robustness = true;
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
				user_options.display_wintess = true;
				user_options.time_serie = true;
				break;

			case 'W':
				user_options.witness_use = all_wit;
				user_options.display_wintess = true;
				user_options.time_serie = true;
				break;

			// Open file with color mask
			case 'm':
				testLast(switch_num, argument->size());
				coloring_parser.openFile(*(++argument));
				return;

			// Open file with color mask
			case 'M':
				testLast(switch_num, argument->size());
				coloring_parser.createOutput(*(++argument));
				return;

			// Get data for distributed computation
			case 'D':
				testLast(switch_num, argument->size());
				getDistribution(*(argument+1), *(argument+2)); argument += 2;
				return;

			// Redirecting results output to a file by a parameter
			case 'F':
				testLast(switch_num, argument->size());
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
	 * Take all the arguments on the input and store information from them
	 *
	 * @param argc	passed from main function
	 * @param argv	passed from main function
	 * @param intput_stream	pointer to a file that will be used as an input stream
	 */
	void parseArguments (const std::vector<std::string> & arguments, std::ifstream & input_stream) {
		// Control provision of an input file
		bool file_parsed = false;

		// Cycle through arguments
		for (auto argument = arguments.begin(); argument != arguments.end(); argument++) {
			// There can be multiple switches after "-" so go through them in the loop
			if ((*argument)[0] == '-') {
				parseSwitches(argument);
			}

			// If it is a model file
			else if (argument->find(".dbm") != std::string::npos){
				// If the model is alredy parsed
				if (file_parsed)
					throw (std::invalid_argument("Model file (file with a .dbm suffix) occurs multiple times on the input, only a single occurence is allowed."));
				// Attach the stream to the file
				input_stream.open(*argument, std::ios::in);
				if (input_stream.fail())
					throw std::runtime_error(std::string("Program failed to open an intput stream file: ").append(*argument));
				file_parsed = true;
			}
		}	

		// Throw an exception if no file was found
		if (!file_parsed)
			throw (std::invalid_argument("Model file (file with a .dbm suffix) is missing."));
	}
};


#endif
