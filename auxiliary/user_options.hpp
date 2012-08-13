/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_USER_OPTIONS_INCLUDED
#define PARSYBONE_USER_OPTIONS_INCLUDED

#include "../auxiliary/data_types.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Class that stores options provided by the user on the input. Values can be set up only using the ArgumentParser object.
/// Further access to global object user_options is possible only due to constant getters.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class UserOptions {
	friend class ArgumentParser;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OPTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool show_coloring; ///< Display parametrizations values
	bool be_verbose; ///< Display data from verbose stream (verbose_str), mainly progress information
	bool display_stats; ///< Display data from statistics stream (verbose_str), mainly states and transition counts
	bool time_serie; ///< Work with the property as with a time series (meaning check only reachability property)
	bool compute_robustness; ///< Should robustness value be computed and displyed?
	bool compute_wintess; ///< Should witnesses be computed and displayed?
	bool use_long_witnesses; ///< How witnesses should be displayed?
	std::size_t process_number; ///< What is the ID of this process?
	std::size_t processes_count; ///< How many processes are included in the computation?

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION METHODS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	UserOptions(const UserOptions & other); ///< Forbidden copy constructor.
	UserOptions& operator=(const UserOptions & other); ///< Forbidden assignment operator.

public:
	/**
	 * Constructor, sets up default values
	 */
	UserOptions() {
		compute_wintess = show_coloring = be_verbose = display_stats = time_serie = use_long_witnesses = compute_robustness = false;
		process_number = processes_count = 1;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @return	true if show_coloring is set (displaying each accepting color)
	 */
	inline const bool coloring() const {
		return show_coloring;
	}

	/**
	 * @return	true if compute_robustness (robustness output is requested)
	 */
	inline const bool robustness() const {
		return compute_robustness;
	}

	/**
	 * @return	true if witnesses are to be computed
	 */
	inline const bool witnesses() const {
		return compute_wintess;
	}

	/**
	 * @return	true if additional analysis will be computed (witnesses/robustness)
	 */
	inline const bool analysis() const {
		return (compute_robustness || compute_wintess);
	}

	/**
	 * @return	true if use_long_witnesses is set (display state levels instead of just a number)
	 */
	inline const bool longWit() const {
		return use_long_witnesses;
	}

	/**
	 * @return	true if verbose is set (displaying additional information during computation)
	 */
	inline const bool verbose() const {
		return be_verbose;
	}

	/**
	 * @return	true if display_stats is set (displaying statistics of the model)
	 */
	inline const bool stats() const {
		return display_stats;
	}

	/**
	 * @return	true if negative_check (switching feasible for non-feasible)
	 */
	inline const bool timeSerie() const {
		return time_serie;
	}

	/**
	 * @return	number of this process in distributed computation (indexed from 1)
	 */
	inline const std::size_t procNum() const {
		return process_number;
	}

	/**
	 * @return	total number of processes in distributed computation
	 */
	inline const std::size_t procCount() const {
		return processes_count;
	}	
} user_options; ///< Single object further used through the program.

#endif // PARSYBONE_USER_OPTIONS_INCLUDED
