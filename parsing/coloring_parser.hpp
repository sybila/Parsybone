/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_COLORING_PARSER_INCLUDED
#define PARSYBONE_COLORING_PARSER_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Coloring parser reads a bitmask of colors from the file.
// REMEMBER: Mask file always needs to have number of bytes dividable by Parameters size. If the last set is smaller, it must be shifted to the right!
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../auxiliary/data_types.hpp"
#include "../auxiliary/user_options.hpp"

class ColoringParser {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CLASS DATA
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Data storage
	std::vector<Parameters> colors_sets;

	// File-based values
	std::ifstream::pos_type file_size;
	std::ifstream colors_file;

	// Info
	bool mask_used;

public:

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Basic constructor - should be used only for the single object shared throught the program
	 */
	ColoringParser() : mask_used(false) { }

	/**
	 * Only opens the file with the data stream.
	 *
	 * @param filename	path to the file to read from 
	 */
	void openFile(const std::string filename) {
		colors_file.open(filename, std::ios::in | std::ios::binary|std::ios::ate);
		if (colors_file.fail()) 
			throw std::runtime_error(std::string("Failed to open coloring mask file: ").append(filename).c_str());
		file_size = colors_file.tellg();
		if (file_size % sizeof(Parameters) != 0)
			throw std::runtime_error("Bitmask file has incorrect number of bits - it must be dividable by the size of Paramset.");
		colors_file.seekg(0, std::ios::beg);
		mask_used = true;
	}

	/**
	 * Main parsing function that creates parameters vector.
	 */
	void parseMask() {
		// Read bytemaks
		char * byteblock = new char [file_size];
		colors_file.read(byteblock, file_size);

		// Cycle through bytemasks and reforge them into Parameters
		for (std::size_t shade_num = 0; shade_num < file_size / sizeof(Parameters); shade_num++) {
			Parameters temp = 0;
			for (std::size_t byte_num = 0; byte_num < sizeof(Parameters); byte_num++) {
				temp <<= 8;
				temp |= byteblock[shade_num*sizeof(Parameters) + byte_num];
			}
			colors_sets.push_back(temp);
		}

		colors_file.close();
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @return	true if the mask was provided on the input
	 */
	inline const bool isUsed() const {
		return mask_used;
	}

	/**
	 * @return masks for all colors that can be used
	 */
	inline const std::vector<Parameters> & getColors() const {
		return colors_sets;
	}

	/**
	 * @return number of Parameters e.g. number of rounds of computation
	 */
	inline const std::size_t getParamNum() {
		return colors_sets.size();
	}

} coloring_parser; // Single program-shared output file

#endif