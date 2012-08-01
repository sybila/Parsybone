/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_COLORING_PARSER_INCLUDED
#define PARSYBONE_COLORING_PARSER_INCLUDED

#include "../auxiliary/data_types.hpp"
#include "../auxiliary/user_options.hpp"
#include "../synthesis/paramset_helper.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Coloring parser reads a bitmask of colors from the file.
/// @attention Mask file always needs to have number of bytes dividable by Parameters size. If the last set is smaller, it must be shifted to the right!
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ColoringParser {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CLASS DATA
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Data storage
	std::vector<Paramset> colors_sets;

	// File-based values
	std::ifstream::pos_type file_size;
	std::ifstream input_file;
	std::ofstream output_file;

	// Info
	bool input_mask;
	bool output_mask;

public:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Basic constructor - should be used only for the single object shared throught the program
	 */
	ColoringParser() : input_mask(false), output_mask(false) { }

	/**
	 * Only opens the file with the data stream.
	 *
	 * @param filename	path to the file to read from 
	 */
	void openFile(const std::string filename) {
        input_file.open(filename.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
		if (input_file.fail()) 
			throw std::runtime_error(std::string("Failed to open input coloring mask file: ").append(filename).c_str());
		file_size = input_file.tellg();
		if (file_size % sizeof(Paramset) != 0)
			throw std::runtime_error("Bitmask file has incorrect number of bits - it must be dividable by the size of Paramset.");
		if (file_size / sizeof(Paramset) > colors_sets.max_size())
			throw std::runtime_error("Bitmask is bigger than a possible size of the vector, can not be used due to memory boundaries.");
		input_file.seekg(0, std::ios::beg);
		input_mask = true;
	}

	/**
	 * Create a file to output bitmasks to.
	 *
	 * @param filename	path to the file to read from 
	 */
	void createOutput(const std::string filename) {
        output_file.open(filename.c_str(), std::ios::out | std::ios::binary);
		if (output_file.fail()) 
			throw std::runtime_error(std::string("Failed to open output coloring mask file: ").append(filename).c_str());
		output_mask = true;
	}

	/**
	 * Main parsing function that creates parameters vector.
	 */
	void parseMask() {
		// Read bytemaks
		char * byteblock = new char [static_cast<std::size_t>(file_size)];
		input_file.read(byteblock, file_size);

		// Cycle through bytemasks and reforge them into Parameters
		for (std::size_t shade_num = 0; shade_num < file_size / sizeof(Paramset); shade_num++) {
			Paramset temp = 0;
			// Read bytes and move them to the front of the Parameters, when possible
			for (std::size_t byte_num = 0; byte_num < sizeof(Paramset); byte_num++) {
				temp <<= 8;
				char val = byteblock[shade_num*sizeof(Paramset) + byte_num];
				temp += static_cast<unsigned char>(val);
			}
			colors_sets.push_back(temp);
		}

		input_file.close();
	}

	/**
	 * Send computed data for this round on the ouput
	 *
	 * @param parameters	bitmask of computed feasible colors
	 */
	void outputComputed(const Paramset parameters) {
		// Cycle through bytes
		for (std::size_t byte_num = sizeof(parameters); byte_num > 0; byte_num--) {
			// Take last 8 bits - to save the form, it is needed to shift up till last round 
			output_file << static_cast<unsigned char>(parameters >> (byte_num-1)*8);
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @return	true if the mask was provided on the input
	 */
	inline const bool input() const {
		return input_mask;
	}

    /**
	 * @return	true if the mask is requested on the output
	 */
	inline const bool output() const {
		return output_mask;
	}

	/**
	 * @return masks for all colors that can be used
	 */
	inline const std::vector<Paramset> & getColors() const {
		return colors_sets;
	}

	/**
	 * @return number of Parameters e.g. number of rounds of computation
	 */
	inline const std::size_t getColorsCount() {
		return colors_sets.size() * paramset_helper.getParamsetSize();
	}
} coloring_parser; // Single program-shared output file

#endif
