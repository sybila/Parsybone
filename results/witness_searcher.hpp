/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_WITNESS_SEARCHER_INCLUDED
#define PARSYBONE_WITNESS_SEARCHER_INCLUDED

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// Description
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//#include "../auxiliary/data_types.hpp"
//#include "../reforging/product_structure.hpp"
//#include "witness_storage.hpp"
//
//class WitnessSearcher {
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// DATA AND NEW TYPES
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	const ProductStructure & product;
//	WitnessStorage & witnesses;
//
//	// Stores used arcs in the form (state, predecessor)
//	typedef std::multimap<std::size_t, std::size_t> UsedArcs;
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// SEARCH FUNCTIONS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	/** 
//	 * Recursive function that creates a tree for witnesses of single color for a single state
//	 *
//	 * @param state_num	number of current state in the DFS
//	 * @param color_num	number of color witness is searched for
//	 * @param used_arcs	stores already used arcs in this branch of search
//	 * 
//	 * @return	pair with this state and its succesors (no references, raw data)
//	 */
//	const TreeNode getTree(const std::size_t state_num, const std::size_t color_num, const UsedArcs & used_arcs) {
//		// Get all arcs
//		const Predecessors & predecessors = product.getPredecessors(state_num, color_num);
//		// Store predecesors here
//		std::vector<TreeNode> nodes;
//		// Cycle through arcs
//		bool used_any = false;
//		for (auto pred_it = predecessors.begin(); pred_it != predecessors.end(); pred_it++) {
//			// Skip if already used
//			auto used_state = used_arcs.find(state_num);
//			if (used_state != used_arcs.end())
//				// This predicesor has been already used
//				if (used_state->second == *pred_it)
//					continue;
//			used_any = true;
//			// Otherwise include this arcs and continue in recursion
//			UsedArcs new_used = used_arcs;
//			new_used.insert(std::make_pair(state_num, *pred_it));
//			nodes.push_back(getTree(*pred_it, color_num, new_used));
//		}
//		//if (used_any == false && product.getStateIndexes(state_num).second != 0 && !used_arcs.empty())
//		//	int my_break = 0;
//		// Retrun yourself
//		return TreeNode(state_num, std::move(nodes));
//	}
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CREATION FUNCTIONS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	WitnessSearcher(const WitnessSearcher & other);            // Forbidden copy constructor.
//	WitnessSearcher& operator=(const WitnessSearcher & other); // Forbidden assignment operator.
//
//public:
//	/**
//	 * Get reference data and create final states that will hold all the computed data
//	 */
//	WitnessSearcher(const ProductStructure & _product, WitnessStorage & _witnesses) : product(_product), witnesses(_witnesses) { } 
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// STORING FUNCTIONS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	/**
//	 * Computes witness trees for all colors of a single state and stores them in WitnessStorage
//	 *
//	 * @param state_num	product state to find witnesses from
//	 * @param final	true if witness is for a path false if for a cycle
//	 */
//	void storeWitnesses(const std::size_t state_num, bool final) {
//		// Stores a tree for each color
//		std::map<std::size_t,TreeNode> all_trees;
//		// Cycle through all the colors
//		for (std::size_t color_num = 0; color_num < getParamsetSize(); color_num++) {
//			// create a witness tree
//			TreeNode witness_tree = std::move(getTree(state_num, color_num, UsedArcs()));
//			// If not empty, store
//			if (!witness_tree.getSuccs().empty())
//				all_trees.insert(std::make_pair(color_num, witness_tree));
//		}
//		// Pass to the storage
//		std::size_t KS_state = product.getStateIndexes(state_num).first;
//		std::size_t BA_state = product.getStateIndexes(state_num).second;
//		witnesses.addWitness(final, KS_state, BA_state, std::move(all_trees));
//	}
//
//	/**
//	 * Computes witness trees for all colors of all provided states and stores them in WitnessStorage
//	 *
//	 * @param states	states to store witnesses for
//	 * @param final	true if witness is for a path false if for a cycle
//	 */
//	void storeWitnesses(const std::vector<std::size_t> states, bool final) {
//		std::for_each(states.begin(), states.end(), [&](const std::size_t state) {
//			storeWitnesses(state, final);
//		});
//	}
//
//
//};

#endif