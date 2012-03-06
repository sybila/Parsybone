/*
 * Copyright (C) 2012 - Adam Streck
 *
 * This file is part of PoSeIDoN (Parameter Synthetizer for Discrete Networks) verification tool
 *
 * Poseidon is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Poseidon is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 *
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno.
 * See http://sybila.fi.muni.cz/ .
 */

#ifndef POSEIDON_RESULTS_INCLUDED
#define POSEIDON_RESULTS_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Results are used by ModelChecker to store the important data from synthesis.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ModelChecker;

class Results {
	friend class ModelChecker;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	 std::vector<Coloring> coloring;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FILLING FUNCTIONS (can be used only from BasicStructureBuilder)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void addResult(const std::size_t state, const Parameters & parameters, const std::size_t state_index) {
		coloring[state_index].first = state;
		for (std::size_t param_pos = 0; param_pos < parameters.size(); param_pos++) {
			coloring[state_index].second.push_back(parameters[param_pos]);
		}
	}

	void setStatesCount(const std::size_t states_count) {
		coloring.resize(states_count);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTING FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	Results(const Results & other);            // Forbidden copy constructor.
	Results& operator=(const Results & other); // Forbidden assignment operator.

public:
	Results() {} // Default empty constructor, needed to create an empty object that will be filled

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @return	total number of parameters
	 */
	const std::size_t countParameters() const {
		Parameters all;
		all = coloring[0].second;
		std::for_each(coloring.begin(), coloring.end(), [&all](const Coloring & coloring){
			all |= coloring.second;
		});
		return all.count();
	}

	/**
	 * @return	All feasible parameters from the paramter space
	 */
	const Parameters getAllParameters() const {
		Parameters all;
		if (coloring.empty())
			return all;
		all = coloring[0].second;
		std::for_each(coloring.begin(), coloring.end(), [&all](const Coloring & coloring){
			all |= coloring.second;
		});
		return all;
	}

	/**
	 * @return	number of colorings in the result
	 */
	inline const std::size_t getColoringsCount() const {
		return coloring.size();
	}

	/**
	 * @param coloring_index	index in the vector of colorings
	 *
	 * @return	coloring with given index
	 */
	inline const Coloring & getColoring(const std::size_t coloring_index) const {
		return coloring[coloring_index];
	}
};

#endif
