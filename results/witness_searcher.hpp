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
// Class for display of witnesses for all colors in current round.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../reforging/product_structure.hpp"
#include "../reforging/color_storage.hpp"

class WitnessSearcher {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const ColoringAnalyzer & analyzer;
	const ColorStorage & storage;
	const ProductStructure & product;

	// Witness counting related auxiliary variables:
	std::vector<std::size_t> path; // IDs of states alongside the path
	std::size_t lenght; // Lenght of the path
	Parameters color_num;
	
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
		lenght = color_num = 0;
	} 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OUTPUT FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:

	void displayWit() const {
		for (std::size_t state_index = 0; state_index < lenght; state_index++) {
			output_streamer.output(product.getString(path[state_index]), OutputStreamer::no_newl);
		}
		output_streamer.output("");
	}

	void DFS(const StateID ID) {
		path[lenght++] = ID;

		if (product.isInitial(ID)) {
			displayWit();
		}
		else { 
			const Predecessors preds = storage.getPredecessors(ID, color_num);
		
			for (auto pred_it = preds.begin(); pred_it != preds.end(); pred_it++) {

				bool used = false;
				for (std::size_t state_index = 0; state_index < lenght; state_index++) {
					if (path[state_index] == *pred_it) {
						used = true;
						break;
					}
				}

				if (used)
					continue;

				DFS(*pred_it);
			}
		}

		--lenght;
	}

	/**
	 * Display witnesses for given color form give state
	 *
	 * @color_num index in current round for this color
	 */
	void displayWitnesses(const Parameters _color_num, const StateID start) {
		// Set starting values
		lenght = 0;
		color_num = _color_num;
		
		DFS(start);
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
			for (auto final_it = product.getFinalStates().begin(); final_it != product.getFinalStates().end(); final_it++) {
				displayWitnesses(color_it->first, *final_it);
			}
		}
	}
};

#endif