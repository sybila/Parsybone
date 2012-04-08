/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_WITNESS_STORAGE_INCLUDED
#define PARSYBONE_WITNESS_STORAGE_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Description
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../auxiliary/data_types.hpp"
#include "../reforging/product_structure.hpp"
#include "tree_node.hpp"

class WitnessSearcher;

class WitnessStorage {
	friend class WitnessSearcher;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	struct StateWitnesses {
		std::size_t KS_state;
		std::size_t BA_state;
		std::vector<std::pair<std::size_t,TreeNode>>  witnesses; // Witness tree for each color in the form (color_num, witness_tree)

		StateWitnesses(const std::size_t _KS_state, const std::size_t _BA_state, std::vector<std::pair<std::size_t,TreeNode>>  && _witnesses) 
			         : KS_state(_KS_state), BA_state(_BA_state), witnesses(std::move(_witnesses)) { }
	};

	std::vector<StateWitnesses> path_witnesses;
	std::vector<StateWitnesses> cycle_witnesses;

	const ProductStructure & product;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTING FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Stores a witness trees for all colors of a single state
	 */
	void addWitness(const bool path, const std::size_t _KS_state, const std::size_t _BA_state, std::vector<std::pair<std::size_t,TreeNode>> && _witnesses) {
		if (path) // for a path from initial to final vertex
			path_witnesses.push_back(StateWitnesses(_KS_state, _BA_state, std::move(_witnesses)));
		else // for a cycle from a finial vertex to itself
			cycle_witnesses.push_back(StateWitnesses(_KS_state, _BA_state, std::move(_witnesses)));
	}

	WitnessStorage(const WitnessStorage & other);            // Forbidden copy constructor.
	WitnessStorage& operator=(const WitnessStorage & other); // Forbidden assignment operator.

public:
	/**
	 * Get reference data and create final states that will hold all the computed data
	 */
	WitnessStorage(const ProductStructure & _product) : product(_product) {
	} 

	const std::vector<std::string> getAllWitnesses();
};

#endif