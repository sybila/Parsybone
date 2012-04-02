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

#ifndef PARSYBONE_OUTPUT_STREAMER_INCLUDED
#define PARSYBONE_OUTPUT_STREAMER_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class that contains methods for standard and special stream output.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>

#include "../auxiliary/data_types.hpp"

class OutputStreamer {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CLASS DATA
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Stream to output minor fails as well as terminal failures
	std::ostream * fail_stream;
	// Stream to output work status in case it is requested by user
	std::ostream * verbose_stream;
	// Stream to output results of coloring
	std::ostream * result_stream;
	
	// True if the user chooses so
	bool output_verbose;
	// True if these streams are assigned a file
	bool fail_file, verbose_file, result_file;

public:

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OUTPUT TRAITS DEFINITIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	static const unsigned int no_newl    = 1;
	static const unsigned int important  = 2;
	static const unsigned int rewrite_ln = 4;

	/**
	 * test if given trait is present
	 *
	 * @param tested	number of the tested trait
	 * @param traits	traits given with the function
	 * 
	 * @return bool if the trait is present
	 */
	inline bool testTrait(const unsigned int tested, const unsigned int traits) const {
		return ((traits | tested)== traits);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Basic constructor - should be used only for the single object shared throught the program
	 */
	OutputStreamer() { 
		// Basic direction of the streams
		fail_stream = &std::cerr;
		verbose_stream = &std::cout;
		result_stream = &std::cout;

		// Set control variables
		output_verbose = false;
		fail_file = verbose_file = result_file = false;
	}

	/**
	 * If some of the streams has been assigned a file, delete that file object
	 */
	~OutputStreamer() {
		if (fail_file)
			delete fail_stream;
		if (verbose_file)
			delete verbose_stream;
		if (result_file)
			delete result_stream;
	}

	/**
	 * Set verbose to true
	 */
	inline void useVerbose() {
		output_verbose = true;
	}

	/**
	 * output on a specified stream
	 *
	 * @param stream_type	enumeration type specifying the type of stream to output to
	 * @param data	data to output - should be any possible ostream data
	 */
	void createStreamFile(StreamType stream_type, std::string filename) {
		// Try to open the file
		std::fstream * file_stream = new std::fstream(filename, std::ios::out);
		if (file_stream->fail()) 
			throw std::runtime_error(std::string("Program failed to open output stream file: ").append(filename));
		
		// Assiciate pointer to the file with one of the streams
		switch (stream_type) {
		case fail:
			fail_stream = file_stream;
			break;
		case verbose:
			verbose_stream = file_stream;
			break;
		case data: 
			result_stream = file_stream;
			break;
		}	
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OUTPUT FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	/**
	 * output on a specified stream
	 *
	 * @param stream_type	enumeration type specifying the type of stream to output to
	 * @param data	data to output - should be any possible ostream data
	 */
	template <class outputType> 
    const OutputStreamer & output(StreamType stream_type, const outputType & stream_data, const unsigned int trait_mask = 0) const {
		switch (stream_type) {
		case fail:
			actualOutput(*fail_stream, stream_data, trait_mask);		
			break;
		case verbose:
			if (output_verbose)
				actualOutput(*verbose_stream, stream_data, trait_mask);				
			break;
		case data: 
			actualOutput(*result_stream, stream_data, trait_mask);		
			break;
		}
		return *this;
	}

private:
	template <class outputType> 
	void actualOutput(std::ostream & stream, const outputType & stream_data, const unsigned int trait_mask) const {
		// Return to start of the line
		if (testTrait(rewrite_ln, trait_mask))
			stream << '\r';
		// Add stars
		if (testTrait(important, trait_mask))
			stream << "*** ";
		// Actuall data
		stream << stream_data;
		// Add stars
		if (testTrait(important, trait_mask))
			stream << " ***";
		// End of the line if not requested otherwise
		if (!testTrait(no_newl, trait_mask))
			stream << std::endl;
	}
} output_streamer; // Single program-shared output file

#endif