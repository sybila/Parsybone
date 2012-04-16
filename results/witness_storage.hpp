/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_WITNESS_STORAGE_INCLUDED
#define PARSYBONE_WITNESS_STORAGE_INCLUDED
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// Description
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//#include "../auxiliary/data_types.hpp"
//#include "../reforging/product_structure.hpp"
//#include "tree_node.hpp"
//
//class WitnessSearcher;
//
//class WitnessStorage {
//	friend class WitnessSearcher;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// NEW TYPES AND DATA:
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	struct StateWitnesses {
//		std::size_t KS_state;
//		std::size_t BA_state;
//		std::map<std::size_t,TreeNode>  witnesses; // Witness tree for each color in the form (color_num, witness_tree)
//
//		StateWitnesses(const std::size_t _KS_state, const std::size_t _BA_state, std::map<std::size_t,TreeNode> && _witnesses) 
//			         : KS_state(_KS_state), BA_state(_BA_state), witnesses(std::move(_witnesses)) { }
//	};
//
//	std::vector<StateWitnesses> path_witnesses;
//	std::vector<StateWitnesses> cycle_witnesses;
//
//	const ProductStructure & product;
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONSTRUCTING FUNCTIONS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	/**
//	 * Stores a witness trees for all colors of a single state
//	 */
//	void addWitness(const bool path, const std::size_t _KS_state, const std::size_t _BA_state, std::map<std::size_t,TreeNode> && _witnesses) {
//		if (path) // for a path from initial to final vertex
//			path_witnesses.push_back(StateWitnesses(_KS_state, _BA_state, std::move(_witnesses)));
//		else // for a cycle from a finial vertex to itself
//			cycle_witnesses.push_back(StateWitnesses(_KS_state, _BA_state, std::move(_witnesses)));
//	}
//
//	WitnessStorage(const WitnessStorage & other);            // Forbidden copy constructor.
//	WitnessStorage& operator=(const WitnessStorage & other); // Forbidden assignment operator.
//
//public:
//	/**
//	 * Get reference data and create final states that will hold all the computed data
//	 */
//	WitnessStorage(const ProductStructure & _product) : product(_product) { } 
//
//	/**
//	 * Prepare for the next round (Free memory and store overall number)
//	 */
//	void finishRound() {
//		path_witnesses.clear();
//		cycle_witnesses.clear();
//	}
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PATHS TO STRINGS FUNCTIONS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//private:
//	/**
//	 * Recursive function that builds the witnesses by DFS in the witness tree
//	 *
//	 * @param node	current node in DFS
//	 * @param substring	string with already included states
//	 * @param full_strings	storage for fully build paths
//	 */
//	void createPaths(const TreeNode & node, std::string substring, std::vector<std::string> & full_strings) const {
//		substring = product.getString(node.getState()) + substring;
//		// Do not add if does not end in the initial state
//		if (node.getSuccs().empty() /* && (node.getState() % product.getBA().getStatesCount() == 0) */) 
//			full_strings.push_back(substring);
//		else
//			for (auto succ_it = node.getSuccs().begin(); succ_it != node.getSuccs().end(); succ_it++) {
//				// Skip if cycles on the final state
//				/*if (node.getState() % product.getBA().getStatesCount() == product.getBA().getStatesCount() - 1)
//					if (succ_it->getState() % product.getBA().getStatesCount() == product.getBA().getStatesCount() - 1)
//						continue;*/
//				createPaths(*succ_it, substring, full_strings);
//			}
//	}
//
//public:
//	/** 
//	 * Get vector of witnesses for a single color and state
//	 */
//	std::vector<std::string> getWitnesses(const bool path, const std::size_t state_index, const std::size_t color_num) const {
//		// Create supporting reference
//		const std::vector<StateWitnesses> & wits = path ? path_witnesses : cycle_witnesses;
//		// Data to fill
//		std::vector<std::string> strings;
//		
//		createPaths(wits[state_index].witnesses.find(color_num)->second, "", strings);
//
//		return strings;
//	}
//
//	/** 
//	 * Return vector of all witnesses for each color
//	 */
//	std::vector<std::pair<std::size_t, std::vector<std::pair<std::size_t, std::string>>>> getAllWitnesses (const bool path) const {
//		// Create supporting reference
//		const std::vector<StateWitnesses> & wits = path ? path_witnesses : cycle_witnesses;
//		// Data to fill
//		std::vector<std::pair<std::size_t, std::vector<std::pair<std::size_t, std::string>>>> strings;
//
//		// Cycle through colors
//		for (std::size_t color_num = 0; color_num < getParamsetSize(); color_num++) {
//
//			// Cycle through all colored states
//			std::vector<std::pair<std::size_t, std::string>> color_wits;
//			for (std::size_t state_num = 0; state_num < wits.size(); state_num++) {
//
//				// If the state has the color, add witnesses
//				if (wits[state_num].witnesses.find(color_num) != wits[state_num].witnesses.end()) {
//					std::vector<std::string> state_wits = std::move(getWitnesses(path, state_num, color_num));
//					// Add witnesses together with their states
//					for (auto wit_it = state_wits.begin(); wit_it != state_wits.end(); wit_it++) {
//						color_wits.push_back(std::make_pair(state_num, *wit_it));
//					}
//				}
//			}
//			// If there are some witnesses for this color, add it
//			if (!color_wits.empty()) 
//				strings.push_back(std::make_pair(color_num, color_wits));
//		}
//		return strings;
//	}
//};

#endif