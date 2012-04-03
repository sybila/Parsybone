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

#ifndef PARSYBONE_SPLIT_MANAGER_INCLUDED
#define PARSYBONE_SPLIT_MANAGER_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This class controls splitting of the parameter space both for independent rounds and for distributed synthesis.
// All data in this class are basic type variables.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <climits>

#include "../auxiliary/data_types.hpp"
#include "../coloring/parameters_functions.hpp"
#include "../results/output_streamer.hpp"

class SplitManager {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::size_t processes_count; // Number of the processes
	std::size_t process_number; // Index of this process (starting from 1)
	std::size_t all_parameters_count; // All the parameters
	std::size_t parameters_begin; // Position to start a synthesis for this process (absolute position w.r.t. all the parameters)
	std::size_t parameters_end; // Position one behind the last parameter for this process (absolute position w.r.t. all the parameters)
	std::size_t bits_per_round; // Nuber of bits per round
	std::size_t last_round_bits; // Number of bits for the absolutelly last round of this process
	std::size_t rounds_count; // Number of rounds totally
	std::size_t round_number; // Number of this round (starting from 0)
	std::size_t round_begin; // Position to start a synthesis for this round (absolute position w.r.t. all the parameters)
	std::size_t round_end; // Position one behind the last parameter for this round (absolute position w.r.t. all the parameters)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	 * This function computes index of the first parameter, size of a single round, number of rounds and other auxiliary data members used for splitting.
	 */
	void computeSubspace() {
		// Split parameter space rounded down, this number may not be precise
		std::size_t parameters_per_process = all_parameters_count / processes_count;
		// Compute start and end positions
		parameters_begin = parameters_per_process * (process_number - 1);
		if (process_number == processes_count) {
			parameters_end = all_parameters_count;
		}
		else {
			parameters_end = parameters_per_process * process_number;
		}
		std::size_t parameters_count = parameters_end - parameters_begin;
		// Set positions for the round
		setStartPositions();
		// Compute number of full rounds
		rounds_count = parameters_per_process / bits_per_round;
		// Compute size of the last round, if its not full-sized, add another round
		if (parameters_count % bits_per_round == 0) {
			last_round_bits = bits_per_round;
		}
		else {
			last_round_bits = parameters_count % bits_per_round;
			rounds_count++;
		}
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
	void setupSplitting(const std::size_t _process_number, const std::size_t _processes_count, const std::size_t _parameters_count) {
		// Pass the numbers
		processes_count = _processes_count;
		process_number = _process_number;
		all_parameters_count = _parameters_count;
		// Set the main size of the loop
		bits_per_round = sizeof(Parameters) * 8;
		// Compute the rest
		computeSubspace();
	}

	/**
	 * Set values for the first round of computation.
	 */
	void setStartPositions() {
		round_begin = parameters_begin;
		round_end = std::min(round_begin + bits_per_round, parameters_end);
		round_number = 0;
	}

	/**
	 * Increase parameter positions so a new round can be computed.
	 */
	void increaseRound() {
		round_number++;
		round_begin = round_end;
		// For the last round we have to use a shorter range, if necessary, otherwise we use whole
		if (lastRound())
			round_end += last_round_bits;
		else
			round_end += bits_per_round;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OTHER
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void outputRound() {
		output_streamer.output(verbose, "Round: ", OutputStreamer::no_newl | OutputStreamer::rewrite_ln).output(round_number + 1, OutputStreamer::no_newl)
			           .output("/", OutputStreamer::no_newl).output(rounds_count, OutputStreamer::no_newl).output("         ", OutputStreamer::no_newl);
	}
		
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @return	total number of processes
	 */ 
	inline const std::size_t getProcessesCount() const {
		return processes_count;
	}

	/**
	 * @return	index of this process starting from 1
	 */ 
	inline const std::size_t getProcessNumber() const {
		return process_number;
	}

	/**
	 * @return	total number of parameters for all the processes
	 */ 
	inline const std::size_t getAllParametersCount() const {
		return all_parameters_count;
	}

	/**
	 * @return	range with first and one before last parameter to compute this round
	 */ 
	inline const Range getRoundRange() const {
		return Range(round_begin, round_end);
	}

	/**
	 * @return	range with first and one before last parameter to compute for this process
	 */ 
	inline const Range getProcessRange() const {
		return Range(parameters_begin, parameters_end);
	}

	/**
	 * @return	true if this round is not the last
	 */ 
	inline const bool lastRound() const {
		return (round_number + 1) == rounds_count;
	}

	/**
	 * @return	number of this round
	 */ 
	inline const std::size_t getRoundNum() const {
		return round_number;
	}

	/**
	 * @return	total number of rounds
	 */ 
	inline const std::size_t getRoundCount() const {
		return rounds_count;
	}

	/**
	 * @return All the parameters of the current round.
	 */
	inline Parameters createStartingParameters() {
		if (!lastRound())
			return getAll();
		else
			return getAll() >> (bits_per_round - last_round_bits);	
	}
};

#endif