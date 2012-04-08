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

#include "../auxiliary/data_types.hpp"
#include "../reforging/product_structure.hpp"
#include "witness_storage.hpp"

class WitnessSearcher {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const ProductStructure & product;
	WitnessStorage & witnesses;

	// Stores used arcs in the form (state, predecessor)
	typedef std::multimap<std::size_t, std::size_t> UsedArcs;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// METHODS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const TreeNode getTree(const std::size_t state_num, const std::size_t color_num, const UsedArcs & used_arcs) {
		const Predecessors & predecessors = product.getPredecessors(state_num, color_num);
		std::vector<TreeNode> nodes;
		for (auto pred_it = predecessors.begin(); pred_it != predecessors.end(); pred_it++) {
			// Skip if already used
			auto used_state = used_arcs.find(state_num);
			if (used_state != used_arcs.end())
				if (used_state->second == *pred_it)
					continue;
			// Otherwise include and continue in recursion
			UsedArcs new_used = used_arcs;
			new_used.insert(std::make_pair(state_num, *pred_it));
			nodes.push_back(getTree(*pred_it, color_num, new_used));
			int slack = 0;
		}
		return TreeNode(state_num, std::move(nodes));
	}

	WitnessSearcher(const WitnessSearcher & other);            // Forbidden copy constructor.
	WitnessSearcher& operator=(const WitnessSearcher & other); // Forbidden assignment operator.

public:
	/**
	 * Get reference data and create final states that will hold all the computed data
	 */
	WitnessSearcher(const ProductStructure & _product, WitnessStorage & _witnesses) : product(_product), witnesses(_witnesses) {

	} 

	void storeWitnesses(const std::size_t state_num) {
		// Cycle through all the colors
		std::vector<TreeNode> all_trees;
		for (std::size_t color_num = 0; color_num < getParamsetSize(); color_num++) {
			TreeNode witness_tree = std::move(getTree(state_num, color_num, UsedArcs()));
			if (!witness_tree.getSuccs().empty())
				all_trees.push_back(witness_tree);
		}
		std::size_t KS_state = product.getStateIndexes(state_num).first;
		std::size_t BA_state = product.getStateIndexes(state_num).second;
		witnesses.addStateWitness(KS_state, BA_state, std::move(all_trees));
	}

	void storeWitnesses(const std::vector<std::size_t> states) {
	}


};

#endif