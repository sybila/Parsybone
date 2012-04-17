/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_WITNESS_SEARCHER_INCLUDED
#define PARSYBONE_WITNESS_SEARCHER_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Description
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../reforging/product_structure.hpp"
#include "../reforging/color_storage.hpp"

class WitnessSearcher {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA AND NEW TYPES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const ColoringAnalyzer & analyzer;
	const ColorStorage & storage;
	const ProductStructure & product;

	// Witness counting related auxiliary variables:
	std::vector<std::size_t> path; // IDs of states alongside the path
	std::size_t lengh; // Lenght of the path
	std::size_t last_transit; // Number steps since last change of BA state
	
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SEARCH FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	WitnessSearcher(const WitnessSearcher & other);            // Forbidden copy constructor.
	WitnessSearcher& operator=(const WitnessSearcher & other); // Forbidden assignment operator.

public:
	/**
	 * Get reference data and create final states that will hold all the computed data
	 */
	WitnessSearcher(const ColoringAnalyzer & _analyzer, const ColorStorage & _storage, const ProductStructure & _product) 
		           : analyzer(_analyzer), product(_product), storage(_storage) {
		// Resize path for maximal possible lenght
		path.resize(_product.getStateCount() - _product.getBA().getStateCount() * 2 + 2);
		lengh = last_transit = 0;
	} 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OUTPUT FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	/**
	 * Display witnesses for given color
	 *
	 * @color_num index in current round for this color
	 */
	void displayWitnesses(const std::size_t color_num, const std::size_t) {
		
	}

public:
	/**
	 * Output all witnesses for all colors, might be together with the colors as well.
	 */
	void display() {
		// Get synthetized colors
		auto colors = analyzer.getColors();

		// Go through colors
		for (auto color_it = colors.begin(); color_it != colors.end(); color_it++) {
			// Display color if requested
			if (user_options.coloring())
				output_streamer.output(results_str, color_it->second);

			// Display witnesses for given color from each final state
			for (auto final_it = product.getFinalStates().begin(); final_it != product.getFinalStates().begin(); final_it++) {
				displayWitnesses(color_it->first, *final_it);
			}
		}
	}
};

#endif