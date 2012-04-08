/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_RESULTS_INCLUDED
#define PARSYBONE_RESULTS_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Results are used by ModelChecker to store the data computed during synthesis - used mostly only for a single round.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../reforging/product_structure.hpp"

class SynthesisManager;
class ProductAnalyzer;

class ResultStorage {
	friend class SynthesisManager;
	friend class ProductAnalyzer;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	struct StateColoring {
		std::size_t KS_state;
		std::size_t BA_state;
		Parameters parameters; // Mask of all the round parameters
		std::vector<std::pair<std::size_t, std::string>> colors; // Colors from present parameters

		StateColoring(const std::size_t _KS_state, const std::size_t _BA_state, const Parameters _parameters, std::vector<std::pair<std::size_t, std::string>> && _colors) 
			         : KS_state(_KS_state), BA_state(_BA_state), parameters(_parameters), colors(std::move(_colors)) { }
	};

	// Values that increase through the rounds
	std::size_t total_colors;

	// Data stored for this round only
	std::vector<StateColoring> colorings;

	const ProductStructure & product; // Referecnce to product

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTING FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	/**
	 * Add a new results - contains product state, mask of parameters for this state and a color that corespond to them
	 */ 
	void addColoring (const std::size_t product_state, const Parameters _parameters, std::vector<std::pair<std::size_t, std::string>> && _colors) {
		// Recompute the states
		std::size_t KS_state = product.getStateIndexes(product_state).first;
		std::size_t BA_state = product.getStateIndexes(product_state).second;
		colorings.push_back(StateColoring(KS_state, BA_state, _parameters, std::move(_colors)));
	}

	/**
	 * Prepare for the next round
	 */
	void finishRound() {
		total_colors += count(getAllParameters());
		colorings.clear();
	}

	ResultStorage(const ResultStorage & other);            // Forbidden copy constructor.
	ResultStorage& operator=(const ResultStorage & other); // Forbidden assignment operator.

public:
	/**
	 * Get reference data and create final states that will hold all the computed data
	 */
	ResultStorage(const ProductStructure & _product) : product(_product) { 
		total_colors = 0;
	} 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
	 * @return	number of all the parameters synthetized
	 */
	inline const std::size_t getTotalColors() const {
		return total_colors;
	}

	/*
	 * @return	merge of parameters of all the states
	 */
	const Parameters getAllParameters() const {
		Parameters merged = 0;
		// Merge colors from all final states
		for (auto color_it = colorings.begin(); color_it != colorings.end(); color_it++) {
			merged |= color_it->parameters;
		}
		return merged;
	}

	/**
	 * @return	all the distinctive colors from this round
	 */
	const std::vector<std::pair<std::size_t, std::string>> getAllColors() const {
		std::set<std::pair<std::size_t, std::string>> colors;
		// Take all the unique color strings
		for (auto color_it = colorings.begin(); color_it != colorings.end(); color_it++) {
			colors.insert(color_it->colors.begin(), color_it->colors.end());
		}
		// Return color strings in a vector

		return std::vector<std::pair<std::size_t, std::string>>(colors.begin(), colors.end());
	}
};

#endif
