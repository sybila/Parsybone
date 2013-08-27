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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Class responsible for division of a parametrization space between rounds within a process.
///
/// This class controls splitting of the parameter space both for independent rounds and for distributed synthesis.
/// All data in this class are basic type variables.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SplitManager {
    ParamNo all_colors_count; ///< All the parametrizations.
    ParamNo process_color_count; ///< Cut of all the parametrizations for this process.
    RoundNo rounds_count; ///< Number of rounds totally.
    RoundNo round_number; ///< Number of this round (starting from 0).
    ParamNo param_no; ///< Which parametrization is currently in use.

    /**
     * This function computes index of the first parameter, size of a single round, number of rounds and other auxiliary data members used for splitting.
     */
    void computeSubspace() {
        // Number of full rounds for all processes
        rounds_count = all_colors_count / user_options.processes_count;
        ParamNo rest_bits = all_colors_count % user_options.processes_count;

        // If there is some leftover, add a round
        if (rest_bits >= user_options.process_number)
            rounds_count++;

        // Get colors num for this process
        process_color_count = rounds_count;

        // Set positions for the round
        setStartPositions();
    }

public:
    /**
     * Computes splitting for both process (in case of a distributed computation) and its rounds that are of a size of the Parameters data type.
     *
     * @param _processes_count	how many processes compute the coloring
     * @param _process_number	index of this process
     * @param _parameters_count	complete number of parameters that have to be tested by all the processes
     */
    SplitManager(ParamNo _all_colors_count) {
        all_colors_count = _all_colors_count;
        // Compute the rest
        computeSubspace();
    }

    /**
     * Set values for the first round of computation.
     */
    void setStartPositions() {
        param_no = user_options.process_number - 1;
        round_number = 1;
    }

    /**
     * Increase parameter positions so a new round can be computed.
    * @return  true if the increase is possible
     */
    bool increaseRound() {
        if (++round_number > rounds_count)
            return false;

        param_no += user_options.processes_count;
        return true;
    }

    /**
     * @return	total number of parameters for all the processes
     */
    inline ParamNo getAllColorsCount() const {
        return all_colors_count;
    }

    /**
     * @return	range with first and one before last parameter to compute this round
     */
    inline ParamNo getParamNo() const {
        return param_no;
    }

    /**
     * @return	range with first and one before last parameter to compute for this process
     */
    inline ParamNo getProcColorsCount() const {
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
    inline RoundNo getRoundNo() const {
        return round_number;
    }

    /**
     * @return	total number of rounds
     */
    inline RoundNo getRoundCount() const {
        return rounds_count;
    }
};

#endif // PARSYBONE_SPLIT_MANAGER_INCLUDED
