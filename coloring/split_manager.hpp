/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_SPLIT_MANAGER_INCLUDED
#define PARSYBONE_SPLIT_MANAGER_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This class controls splitting of the parameter space both for independent rounds and for distributed synthesis.
// All data in this class are basic type variables.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../auxiliary/user_options.hpp"
#include "../auxiliary/output_streamer.hpp"
#include "../coloring/parameters_functions.hpp"

class SplitManager {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ColorNum all_colors_count; // All the parameters
	ColorNum process_color_count; // All the parameters
	std::size_t last_round_bits; // Number of bits for the absolutelly last round of this process
	long long rounds_count; // Number of rounds totally
	long long round_number; // Number of this round (starting from 0)
	ColorNum round_begin; // Position to start a synthesis for this round (absolute position w.r.t. all the parameters)
	ColorNum round_end; // Position one behind the last parameter for this round (absolute position w.r.t. all the parameters)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	 * This function computes index of the first parameter, size of a single round, number of rounds and other auxiliary data members used for splitting.
	 */
	void computeSubspace() {
		// Number of full rounds for all processes
		rounds_count = all_colors_count / (user_options.procCount() * getParamsetSize());
		ColorNum rest_bits = all_colors_count % (user_options.procCount() * getParamsetSize());
		last_round_bits = getParamsetSize();

		// If there is some leftover, add a round
		if (std::ceil(static_cast<double>(rest_bits) / static_cast<double>(getParamsetSize())) >= user_options.procNum()) {
			rounds_count++;
			// Pad last round
			if ((rest_bits / getParamsetSize()) == (user_options.procNum() - 1))
				last_round_bits = rest_bits % getParamsetSize();
		}
		
		// Get colors num for this process
		process_color_count = (rounds_count - 1) * getParamsetSize() + last_round_bits;

		// Set positions for the round
		setStartPositions();
		// Compute number of full rounds

		// Check if it fits together with number from mask
		if (coloring_parser.input())
			if (coloring_parser.getColorsCount() != process_color_count)
				throw std::runtime_error(std::string("Rounds computed from bitmask: ").append(boost::lexical_cast<std::string>(coloring_parser.getColorsCount()))
				                         .append(" does not equal round count computed from model: ")
										 .append(boost::lexical_cast<std::string>(process_color_count).c_str()));
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/**
	 * Computes splitting for both process (in case of a distributed computation) and its rounds that are of a size of the Parameters data type.
	 * 
	 * @param _processes_count	how many processes compute the coloring
	 * @param _process_number	index of this process
	 * @param _parameters_count	complete number of parameters that have to be tested by all the processes
	 */
	SplitManager(ColorNum _all_colors_count) {
		all_colors_count = _all_colors_count;
		// Compute the rest
		computeSubspace();
	}

	/**
	 * Set values for the first round of computation.
	 */
	void setStartPositions() {
		round_begin = (user_options.procNum() - 1) * getParamsetSize();
		if (rounds_count > 0) {
			if (rounds_count > 1)
				round_end = round_begin + getParamsetSize();
			else
				round_end = round_begin + last_round_bits;
		}
		round_number = 0;
	}

	/**
	 * Increase parameter positions so a new round can be computed.
	 */
	void increaseRound() {
		round_number++;
		round_begin += (getParamsetSize() * user_options.procCount());
		// For the last round we have to use a shorter range, if necessary, otherwise we use whole
		if (lastRound())
			round_end = round_begin + last_round_bits;
		else
			round_end = round_begin + getParamsetSize();
	}
	
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @return	total number of parameters for all the processes
	 */ 
	inline const ColorNum getAllColorsCount() const {
		return all_colors_count;
	}

	/**
	 * @return	range with first and one before last parameter to compute this round
	 */ 
	inline const Range getRoundRange() const {
		return Range(round_begin, round_end);
	}

	/**
	 * @return	number of bits in current round
	 */
	inline const ColorNum getRoundSize() const {
		return round_end - round_begin;
	}

	/**
	 * @return	range with first and one before last parameter to compute for this process
	 */ 
	inline const ColorNum getProcColorsCount() const {
		return process_color_count;
	}

	/**
	 * @return	true if this round is not the last
	 */ 
	inline const bool lastRound() const {
		return (round_number + 1) == rounds_count;
	}

	/**
	 * @return	true if current round is valid (this round does not correspond to any paramteres)
	 */ 
	inline const bool valid() const {
		return round_number < rounds_count;
	}

	/**
	 * @return	number of this round
	 */ 
	inline const long long getRoundNum() const {
		return round_number;
	}

	/**
	 * @return	total number of rounds
	 */ 
	inline const long long getRoundCount() const {
		return rounds_count;
	}

	/**
	 * @return All the parameters of the current round.
	 */
	inline Parameters createStartingParameters() const {
		if (!lastRound())
			return getAll();
		else
			return getAll() >> (getParamsetSize() - last_round_bits);	
	}
};

#endif