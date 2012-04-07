/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_TEMPLATE_INCLUDED
#define PARSYBONE_TEMPLATE_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class that stores options provided by the user on the input.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../auxiliary/data_types.hpp"

class ArgumentParser;

class UserOptions {
	friend class ArgumentParser;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OPTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool show_coloring;
	bool show_witnesses;
	bool be_verbose;
	bool negation_check;
	std::size_t process_number;
	std::size_t processes_count;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION METHODS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	UserOptions(const UserOptions & other);            // Forbidden copy constructor.
	UserOptions& operator=(const UserOptions & other); // Forbidden assignment operator.

public:
	/**
	 * Constructor, sets up default values
	 */
	UserOptions() {
		show_witnesses = show_coloring = be_verbose = negation_check = false;
		process_number = processes_count = 1;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GETTERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @return	true if show_coloring is set (displaying each accepting color)
	 */
	inline const bool coloring() const {
		return show_coloring;
	}
	
	/**
	 * @return	true if show_witnesses is set (displaying a path from initial to final states)
	 */
	inline const bool witnesses() const {
		return show_witnesses;
	}

	/**
	 * @return	true if verbose is set (displaying additional information during computation
	 */
	inline const bool verbose() const {
		return be_verbose;
	}

	/**
	 * @return	true if negative_check (switching feasible for non-feasible)
	 */
	inline const bool negation() const {
		return negation_check;
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
	
} user_options;

#endif