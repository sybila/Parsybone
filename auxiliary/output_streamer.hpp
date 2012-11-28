/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_OUTPUT_STREAMER_INCLUDED
#define PARSYBONE_OUTPUT_STREAMER_INCLUDED

#include "../auxiliary/data_types.hpp"
#include "../auxiliary/user_options.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Class that contains methods for standard and special stream output.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class OutputStreamer {
   ostream * error_stream; ///< Stream to output minor fails as well as terminal failures.
   ostream * verbose_stream; ///< Stream to output work status in case it is requested by user.
   ostream * stats_stream; ///< Stream to output overall statistics.
   ostream * result_stream; ///< Stream to output results of coloring.
   ostream * console_stream; ///< This one always goes to console.

   /// True if these streams are assigned a file.
   bool error_file, verbose_file, stats_file, result_file;

   StreamType last_stream_type;	///< Used to ease usage of output - last stream is stored and used if no new is specified.

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // OUTPUT TRAITS DEFINITIONS
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	typedef const unsigned int Trait;
	static Trait no_newl    = 1; ///< After last line no newline symbol will be output.
	static Trait important  = 2; ///< Add "-- " before and " --" after the ouptut.
	static Trait rewrite_ln = 4; ///< Return the cursor and start from the beginning of the line.
	static Trait tab        = 8; ///< Add "   " before the output.

	/**
	 * Test if given trait is present.
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
	// CREATION METHODS
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Basic constructor - should be used only for the single object shared throught the program.
	 */
	OutputStreamer() {
		// Basic direction of the streams
		error_stream = &cerr;
		result_stream = &cout;
		stats_stream = &cout;
		verbose_stream = &cout;
		console_stream = &cout;

		// Set control variables
		error_file = result_file = stats_file = verbose_file = false;

		// Set stream type in the beggining to error stream
		last_stream_type = error_str;
	}

	/**
	 * If some of the streams has been assigned a file, delete that file object.
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
	 * Create a file to which given stream will be redirected.
	 *
	 * @param stream_type	enumeration type specifying the type of stream to output to
	 * @param data	data to output - should be any possible ostream data
	 */
	void createStreamFile(StreamType stream_type, string filename) {
		// Try to open the file
		fstream * file_stream = new fstream(filename.c_str(), ios::out);
		if (file_stream->fail())
			throw runtime_error(string("Program failed to open an output stream file: ").append(filename));
		
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
	// OUTPUT METHODS
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Flush all the streams that are in use.
	 */
	void flush() {
		error_stream->flush();
		result_stream->flush();
		stats_stream->flush();
		verbose_stream->flush();
	}

	/**
	 * Output on a specified stream.
	 *
	 * @param stream_type	enumeration type specifying the type of stream to output to
	 * @param data	data to output - should be any possible ostream data
	 * @param trait_mask	bitmask of traits for output
	 */
	template <class outputType>
	const OutputStreamer & output(StreamType stream_type, const outputType & stream_data, const unsigned int trait_mask = 0) {
		// Update stream
		last_stream_type = stream_type;
		switch (stream_type) {
		case error_str:
			// Start with !
			*error_stream << "! ";
		break;
		case stats_str:
			// Start with #
			if (user_options.stats())
				*verbose_stream << "# ";
		break;
		case verbose_str:
			// Start with * - if requested, remove previous line
			if (user_options.verbose()) {
				if (testTrait(rewrite_ln, trait_mask))
					*verbose_stream << '\r';
				*verbose_stream << "* ";
			}
		break;
		case results_str:
			// This is the only stream that is not enhanced.
		break;
		}
		// Continue with output, then return this object
		return output(stream_data, trait_mask);
	}

	/**
	 * Overloaded method that uses the same stream as the last ouput.
	 *
	 * @param data	data to output - should be any possible ostream data
	 * @param trait_mask	bitmask of traits for output
	 */
	template <class outputType>
	const OutputStreamer & output(const outputType & stream_data, const unsigned int trait_mask = 0) const {
		// Pick the correct stream and pass the data - output only if requested
		switch (last_stream_type) {
		case error_str:
			actualOutput(*error_stream, stream_data, trait_mask);
		break;
		case results_str:
			if (user_options.toFile())
				actualOutput(*result_stream, stream_data, trait_mask);
			if (user_options.toConsole())
				actualOutput(*console_stream, stream_data, trait_mask);
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
    * Method that actually puts the data on the stream.
    *
    * @param stream	reference to the ostream that gets the data
    * @param stream_data	provided content
    * @param trait_mask	information about how the output should be formatted
    */
   template <class outputType>
   void actualOutput(ostream & stream, const outputType & stream_data, const unsigned int trait_mask) const {
      // Use 3 spaces tab
      if (testTrait(tab, trait_mask))
         stream << "   ";
      // Add prefix dashes
      if (testTrait(important, trait_mask))
         stream << "-- ";
      // Actuall data
      stream << stream_data;
      // Add postfix dashes
      if (testTrait(important, trait_mask))
         stream << " --";
      // End of the line if not requested otherwise
      if (!testTrait(no_newl, trait_mask))
         stream << endl;
   }
} output_streamer; ///< Single program-shared output object.

#endif // PARSYBONE_OUTPUT_STREAMER_INCLUDED
