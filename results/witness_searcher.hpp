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

#include "../coloring/parameters_functions.hpp"
#include "../coloring/color_storage.hpp"
#include "../construction/construction_holder.hpp"
#include "../construction/product_structure.hpp"
#include "../results/per_color_storage.hpp"
#include "coloring_analyzer.hpp"

class WitnessSearcher {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const ColoringAnalyzer & analyzer;
	const ColorStorage & storage;
	const ProductStructure & product;
	std::unique_ptr<PerColorStorage>(per_color_storage);

	// Witness counting related auxiliary variables:
	std::vector<StateID> path; // IDs of states alongside the path
	std::map<StateID, long double> state_robustness;
	std::set<StateID> used_paths;
	std::vector<double> probability; // Probability of stepping into each consecutive state
	std::size_t lenght; // Lenght of the path, non-zero value
	std::size_t color_num; // Ordinal of the color
	std::size_t max_path_lenght; // Reference value for output of shortest paths only
	
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
    WitnessSearcher(const ConstructionHolder & holder, const ColoringAnalyzer & _analyzer, const ColorStorage & _storage)
                   : product(holder.getProduct()), analyzer(_analyzer), storage(_storage) {
		// Resize path for maximal possible lenght
        path.resize(product.getStateCount() - product.getBA().getStateCount() * 2 + 2);
		lenght = color_num = max_path_lenght = 0;
	} 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OUTPUT FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:

	/**
	 * Add probability of getting to the requested final state from specific initial state alongside this path
	 */
	void countProb() {
		// Probability of being in the first state
		long double probability = 1.0;
		// For each state divide probability by all its successors
		for (std::size_t state_index = (lenght - 1); state_index > 1; state_index--) {
			probability /= per_color_storage->getNeighbours(path[state_index], true, color_num).size();
		}

		// Check if this initial state is already included, if so, add probability, otherwise create new state with probability of this path
		auto state_prob = state_robustness.find(path[(lenght - 1)]);
		if (state_prob == state_robustness.end())
			state_robustness.insert(std::make_pair(path[(lenght - 1)], probability));
		else
			state_prob->second += probability;
	}

	/**
	 * Outputs a witness path as a sequence of states.
	 */
	void displayWit() const {
		// Cycle through states
		for (std::size_t state_index = lenght; state_index > 1; state_index--) {
			output_streamer.output(results_str, product.getString(path[state_index- 1]), OutputStreamer::no_newl);
		}
		// Endline
		output_streamer.output("");
	}

	/**
	 * Recursive DFS search function for the witness path
	 */
	void DFS(const StateID ID) {
		// Add yourself to the path
		path[lenght++] = ID;

		// If you are initial, print this path
		if (product.isInitial(ID)) {
			used_paths.insert(path[1]);
			if (user_options.displayWintess())
				displayWit();
			if (user_options.robustness())
				countProb();
		}
		else if (lenght < max_path_lenght) { // Continue DFS only if witness has still allowed lenght
			// Get predecessors
			const Neighbours & preds = per_color_storage->getNeighbours(ID, false, color_num);
		
			// List through predecessors
			if (preds.size()) 
				for (Neighbours::const_iterator pred_it = preds.begin(); pred_it != preds.end(); pred_it++) {
					// Do not reuse paths
					if (used_paths.find(*pred_it) != used_paths.end())
						continue;
					// If it was not used yet, continue in DFS
					DFS(*pred_it);
				}
		}

		// Return
		--lenght;
	}

public:
	/**
	 * Output all witnesses for all colors, might be together with the colors as well.
	 */
	void display(const std::vector<std::size_t> & BFS_reach) {
		// Get synthetized colors
		auto colors = analyzer.getColors();
		per_color_storage.reset(new PerColorStorage(analyzer, storage, product));

		// Go through colors
		for (color_num = 0; color_num < colors.size(); color_num++) {
			// Display color if requested
			if (user_options.coloring())
				output_streamer.output(results_str, colors[color_num].second);

			// Get round values
			std::size_t bit_num = getBitNum(colors[color_num].first);
			max_path_lenght = (user_options.witnesses() == short_wit) ? BFS_reach[bit_num] : ~0;

			// Compute witnesses for given color from each final state, they are also displayed, if requested
			for (auto final_it = product.getFinalStates().begin(); final_it != product.getFinalStates().end(); final_it++) {
				// Restart values
				lenght = 0;
				
				// Start search 
				DFS(*final_it);
			}

			// Display robustness if requested
			if (user_options.robustness()) {
				double robustness = 0.0;
				// For each possible initial state add robustness
				forEach(state_robustness, [&robustness](const std::pair<StateID, double> & start) {
					robustness += start.second;
				});
				// Divide by number of inital states and ouput
				robustness /= state_robustness.size();
				output_streamer.output(results_str, robustness);

				state_robustness.clear();
			}

			used_paths.clear();
		}
	}
};

#endif
