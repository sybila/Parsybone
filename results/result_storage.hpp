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

class ColoringAnalyzer;

class ResultStorage {
	friend class ColoringAnalyzer;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	struct StateColoring {
		StateID KS_ID;
		StateID BA_ID;
		Parameters parameters; // Mask of all the round parameters
		std::vector<std::pair<std::size_t, std::string>> colors; // Colors from present parameters

		StateColoring(const StateID _KS_ID, const StateID _BA_ID, const Parameters _parameters, std::vector<std::pair<std::size_t, std::string>> && _colors) 
			         : KS_ID(BA_ID), BA_ID(_BA_ID), parameters(_parameters), colors(std::move(_colors)) { }
		// If colors are not requested, they are not stored
		StateColoring(const StateID _KS_ID, const StateID _BA_ID, const Parameters _parameters) 
			         : KS_ID(_KS_ID), BA_ID(_BA_ID), parameters(_parameters) { }
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
	void addColoring (const StateID ID, const Parameters parameters, std::vector<std::pair<std::size_t, std::string>> && colors) {
		// Recompute the states
		StateID KS_ID = product.getKSID(ID);
		StateID BA_ID = product.getBAID(ID);
		colorings.push_back(StateColoring(KS_ID, BA_ID, parameters, std::move(colors)));
	}

	/**
	 * Add a new results in case colors are not requsted by user.
	 */ 
	void addColoring (const StateID ID, const Parameters parameters) {
		// Recompute the states
		StateID KS_ID = product.getKSID(ID);
		StateID BA_ID = product.getBAID(ID);
		colorings.push_back(StateColoring(KS_ID, BA_ID, parameters));
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

	/**
	 * Prepare for the next round (Free memory and store overall number)
	 */
	void finishRound() {
		total_colors += count(getAllParameters());
		colorings.clear();
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
	 * @return	all the distinctive colors from this round in the form (color_num, color_string)
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
