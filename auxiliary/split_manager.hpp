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
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../auxiliary/data_types.hpp"

class SplitManager {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::size_t processes_count;
	std::size_t process_number;
	std::size_t all_parameters_count;
	std::size_t parameters_count;
	std::size_t parameters_begin;
	std::size_t parameters_end;
	std::size_t bits_per_round;
	std::size_t last_round_bits;
	std::size_t rounds_count;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void computeSubspace() {
		// Split parameter space rounded down, this number may not be precise
		std::size_t parameters_per_process = all_parameters_count / processes_count;
		// Compute start and end positions, last round can be little bit bigger due to rounding
		parameters_begin = parameters_per_process * (process_number - 1);
		if (process_number == processes_count) {
			parameters_end = all_parameters_count;
		}
		else {
			parameters_end = parameters_per_process * process_number;
		}
		// Real number of parameters for this process
		parameters_count = (parameters_end - parameters_begin);
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
	SplitManager(const SplitManager & other);            // Forbidden copy constructor.
	SplitManager& operator=(const SplitManager & other); // Forbidden assignment operator.

public:
	SplitManager() {}

	void setupSplitting(const std::size_t _processes_count, const std::size_t _process_number, const std::size_t _parameters_count) {
		// Pass the numbers
		processes_count = _processes_count;
		process_number = _process_number;
		all_parameters_count = _parameters_count;
		// Set the main size of the loop
		bits_per_round = sizeof(Parameters) * 8;
		// Compute the rest
		computeSubspace();
	}
		
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline const std::size_t getProcessesCount() {
		return processes_count;
	}
	inline const std::size_t getProcessNumber() {
		return process_number;
	}
	inline const std::size_t getAllParametersCount() {
		return all_parameters_count;
	}
	inline const std::size_t getParametersCount() {
		return parameters_count;
	}
	inline const std::size_t getParametersBegin() {
		return parameters_begin;
	}
	inline const std::size_t getParametersEnd() {
		return parameters_end;
	}
	inline const std::size_t getBitsPerRound() {
		return bits_per_round;
	}
	inline const std::size_t getLastRoundBits() {
		return last_round_bits;
	}
	inline const std::size_t getRoundsCount() {
		return rounds_count;
	}
};

#endif