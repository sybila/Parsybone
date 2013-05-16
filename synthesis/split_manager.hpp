/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_SPLIT_MANAGER_INCLUDED
#define PARSYBONE_SPLIT_MANAGER_INCLUDED

#include "../auxiliary/user_options.hpp"
#include "../auxiliary/output_streamer.hpp"
#include "paramset_helper.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Class responsible for division of a parametrization space between rounds within a process.
///
/// This class controls splitting of the parameter space both for independent rounds and for distributed synthesis.
/// All data in this class are basic type variables.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SplitManager {
   ParamNum all_colors_count; ///< All the parameters.
   ParamNum process_color_count; ///< All the parameters.
   size_t last_round_bits; ///< Number of bits for the absolutelly last round of this process.
   RoundNum rounds_count; ///< Number of rounds totally.
   RoundNum round_number; ///< Number of this round (starting from 0).
   ParamNum round_begin; ///< Position to start a synthesis for this round (absolute position w.r.t. all the parameters).
   ParamNum round_end; ///< Position one behind the last parameter for this round (absolute position w.r.t. all the parameters).

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTATION METHODS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * This function computes index of the first parameter, size of a single round, number of rounds and other auxiliary data members used for splitting.
	 */
	void computeSubspace() {
		// Number of full rounds for all processes
		rounds_count = all_colors_count / (user_options.procCount() * paramset_helper.getSetSize());
		ParamNum rest_bits = all_colors_count % (user_options.procCount() * paramset_helper.getSetSize());
		last_round_bits = paramset_helper.getSetSize();

		// If there is some leftover, add a round
		if (ceil(static_cast<double>(rest_bits) / static_cast<double>(paramset_helper.getSetSize())) >= user_options.procNum()) {
			rounds_count++;
			// Pad last round
			if ((rest_bits / paramset_helper.getSetSize()) == (user_options.procNum() - 1))
				last_round_bits = rest_bits % paramset_helper.getSetSize();
		}
		
		// Get colors num for this process
		process_color_count = (rounds_count - 1) * paramset_helper.getSetSize() + last_round_bits;

		// Set positions for the round
		setStartPositions();
		// Compute number of full rounds

		// Check if it fits together with number from mask
      if (user_options.inputMask())
         if (bitmask_manager.getColors().size() != rounds_count)
            throw runtime_error("The number of rounds computed from the bitmask: " + toString(bitmask_manager.getColors().size()) +
                                " it not equal to the round number computed from the model: " + toString(rounds_count));
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREATION METHODS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/**
	 * Computes splitting for both process (in case of a distributed computation) and its rounds that are of a size of the Parameters data type.
	 * 
	 * @param _processes_count	how many processes compute the coloring
	 * @param _process_number	index of this process
	 * @param _parameters_count	complete number of parameters that have to be tested by all the processes
	 */
	SplitManager(ParamNum _all_colors_count) {
		all_colors_count = _all_colors_count;
		// Compute the rest
		computeSubspace();
	}

	/**
	 * Set values for the first round of computation.
	 */
	void setStartPositions() {
		round_begin = (user_options.procNum() - 1) * paramset_helper.getSetSize();
		round_end = round_begin + paramset_helper.getSetSize();
      round_number = 1;
	}

	/**
	 * Increase parameter positions so a new round can be computed.
    *
    * @return  true if the increase is possible
	 */
   bool increaseRound() {
      if (++round_number > rounds_count)
         return false;

		round_begin += (paramset_helper.getSetSize() * user_options.procCount());
		round_end = round_begin + paramset_helper.getSetSize();
      return true;
	}
	
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @return	total number of parameters for all the processes
	 */ 
	inline ParamNum getAllColorsCount() const {
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
	inline ParamNum getRoundSize() const {
		return round_end - round_begin;
	}

	/**
	 * @return	range with first and one before last parameter to compute for this process
	 */ 
	inline ParamNum getProcColorsCount() const {
		return process_color_count;
	}

	/**
	 * @return	true if this round is not the last
	 */ 
	inline bool lastRound() const {
      return round_number == rounds_count;
	}

	/**
	 * @return	number of this round
	 */ 
	inline RoundNum getRoundNum() const {
		return round_number;
	}

	/**
	 * @return	total number of rounds
	 */ 
	inline RoundNum getRoundCount() const {
		return rounds_count;
	}

	/**
    * @return all the parameters of the current round - for the last round, finish has to be cropped
	 */
	inline Paramset createStartingParameters() const {
		if (!lastRound())
			return paramset_helper.getAll();
		else
			return (paramset_helper.getAll() >> (paramset_helper.getSetSize() - last_round_bits)) << (paramset_helper.getSetSize() - last_round_bits);
	}
};

#endif // PARSYBONE_SPLIT_MANAGER_INCLUDED
