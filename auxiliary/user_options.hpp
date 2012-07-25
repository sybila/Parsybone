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
	bool show_coloring;
	bool be_verbose;
	bool display_stats;
	bool negation_check;
	bool time_serie;
	bool add_BA_to_witness;
	bool compute_robustness;
	bool display_wintess;
	WitnessUse witness_use;
	std::size_t process_number;
	std::size_t processes_count;

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
		display_wintess = show_coloring = be_verbose = display_stats = negation_check = time_serie = add_BA_to_witness = compute_robustness = false;
		process_number = processes_count = 1;
		witness_use = none_wit;
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
	 * @return	how to manage witnesses
	 */
	inline const WitnessUse witnesses() const {
		return witness_use;
	}

	/**
	 * @return	true if add_BA_to_witness is set (displaying path with BA states as well)
	 */
	inline const bool BA() const {
		return add_BA_to_witness;
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
	 * @return	true if time_serie (checking only reachability)
	 */
	inline const bool negation() const {
		return negation_check;
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
	
	/**
	 * @return	true if compute_robustness (robustness output is requested)
	 */
	inline const bool robustness() const {
		return compute_robustness;
	}

	/**
	 * @return	true if display_wintess (witness output is requested)
	 */
	inline const bool displayWintess() const {
		return display_wintess;
	}
	
} user_options; ///< Single object further used through the program.

#endif // PARSYBONE_USER_OPTIONS_INCLUDED
