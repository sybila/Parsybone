/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_OUTPUT_STREAMER_INCLUDED
#define PARSYBONE_OUTPUT_STREAMER_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class that contains methods for standard and special stream output.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../auxiliary/data_types.hpp"

class OutputStreamer {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CLASS DATA
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Stream to output minor fails as well as terminal failures
	std::ostream * error_stream;
	// Stream to output work status in case it is requested by user
	std::ostream * verbose_stream;
	// Stream to output overall statistics
	std::ostream * stats_stream;
	// Stream to output results of coloring
	std::ostream * result_stream;
	
	// True if these streams are assigned a file
	bool error_file, verbose_file, stats_file, result_file;

	// Used to ease usage of output
	StreamType last_stream_type;

public:

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OUTPUT TRAITS DEFINITIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	typedef const unsigned int Trait;
	static Trait no_newl    = 1;
	static Trait important  = 2;
	static Trait rewrite_ln = 4;

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
// CONSTANT GETTERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/** 
	 * @return true if there is a file to output the results
	 */
	const bool isResultInFile() const {
		return result_file;
	}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Basic constructor - should be used only for the single object shared throught the program
	 */
	OutputStreamer() { 
		// Basic direction of the streams
		error_stream = &std::cerr;
		result_stream = &std::cout;
		stats_stream = &std::cout;
		verbose_stream = &std::cout;

		// Set control variables
		error_file = result_file = stats_file = verbose_file = false;

		// Set stream type in the beggining to error stream
		last_stream_type = error_str;
	}

	/**
	 * If some of the streams has been assigned a file, delete that file object
	 */
	~OutputStreamer() {
		if (error_file)
			delete error_stream;
		if (stats_file)
			delete stats_stream;
		if (result_file)
			delete result_stream;
		if (verbose_file)
			delete verbose_stream;
	}

	/**
	 * 
	 */

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
		case error_str:
			error_stream = file_stream;
			error_file = true;
			break;
		case stats_str:
			stats_stream = file_stream;
			stats_file = true;
			break;			
		case results_str: 
			result_stream = file_stream;
			result_file = true;
			break;
		case verbose_str:
			verbose_stream = file_stream;
			verbose_file = true;
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
	 * @param trait_mask	bitmask of traits for output 
	 */
	template <class outputType> 
    OutputStreamer & output(StreamType stream_type, const outputType & stream_data, const unsigned int trait_mask = 0) {
		last_stream_type = stream_type;
		switch (stream_type) {
		case error_str:
			*error_stream << "! ";
			break;
		case stats_str:
			if (user_options.stats())
				*verbose_stream << "# ";
			break;
		case verbose_str:
			if (user_options.verbose())
				*verbose_stream << "* ";
			break;
		}
		output(stream_data, trait_mask);
		return *this;
	}

	/**
	 * overloaded method that uses the same stream as the last ouput
	 *
	 * @param data	data to output - should be any possible ostream data
	 * @param trait_mask	bitmask of traits for output 
	 */
	template <class outputType> 
    const OutputStreamer & output(const outputType & stream_data, const unsigned int trait_mask = 0) const {
		switch (last_stream_type) {
		case error_str:
			actualOutput(*error_stream, stream_data, trait_mask);	
			break;
		case results_str: 
			actualOutput(*result_stream, stream_data, trait_mask);		
			break;
		case stats_str: 
			if (user_options.stats())
			actualOutput(*stats_stream, stream_data, trait_mask);		
			break;
		case verbose_str:
			if (user_options.verbose())
				actualOutput(*verbose_stream, stream_data, trait_mask);
			break;
		}
		return *this;
	}

private:
	/**
	 * method that makes formatted ouput
	 *
	 * @param stream	reference to the ostream that gets the data
	 * @param stream_data	provided content
	 * @param trait_mask	information about how the output should be formatted
	 */
	template <class outputType> 
	void actualOutput(std::ostream & stream, const outputType & stream_data, const unsigned int trait_mask) const {
		// Return to start of the line
		if (testTrait(rewrite_ln, trait_mask))
			stream << '\r';
		// Add prefix stars
		if (testTrait(important, trait_mask))
			stream << "-- ";
		// Actuall data
		stream << stream_data;
		// Add postfix stars
		if (testTrait(important, trait_mask))
			stream << " --";
		// End of the line if not requested otherwise
		if (!testTrait(no_newl, trait_mask))
			stream << std::endl;
	}
} output_streamer; // Single program-shared output file

#endif