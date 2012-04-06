/*
 * Copyright (C) 2012 - Adam Streck
 *
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 *
 * ParSyBoNe is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 *
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno.
 * See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_PRODUCT_ANALYZER_INCLUDED
#define PARSYBONE_PRODUCT_ANALYZER_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product analyzer is used to get polished and formatted data from the product.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../reforging/product_structure.hpp"

class ProductAnalyzer {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const SplitManager & split_manager; // Own copy of the split manager
	const ProductStructure & product;
	const ParametrizedStructure & structure;
    const AutomatonStructure & automaton;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTING FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	ProductAnalyzer(const ProductAnalyzer & other);            // Forbidden copy constructor.
	ProductAnalyzer& operator=(const ProductAnalyzer & other); // Forbidden assignment operator.

public:
	/**
	 * Get reference data and create final states that will hold all the computed data
	 */
	ProductAnalyzer(const ProductStructure & _product, const SplitManager & _split_manager) 
		   : product(_product), structure(_product.getKS()), automaton(_product.getBA()), split_manager(_split_manager) { } 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	/**
	 * Merget all the final colors together in a single color of all vertices
	 *
	 * @return a merge of all the accepting colors
	 */
	Parameters mergetAccepting() const {
		Parameters merged = 0;
		const std::vector<std::size_t> & finals = product.getFinals();
		for (auto color_it = finals.begin(); color_it != finals.end(); color_it++) {
			merged |= product.getParameters(*color_it); 
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	
	///**
	// * Routine that counts how many unique bits (parameters) are set in all the final states together.
	// *
	// * @return	total number of parameters
	// */
	//const std::size_t countParameters() const {
	//	// Resulting number
	//	std::size_t parameter_count = 0;
	//	// Go through each partition 
	//	for(std::size_t round_num = 0; round_num < split_manager.getRoundCount(); round_num++) {
	//		Parameters all = 0;
	//		// Add parameters from each other final state
	//		std::for_each(states.begin(), states.end(), [&all, round_num](const ColoredState & state){
	//			all |= state.parameters_parts[round_num];
	//		});
	//		// sum number of parameters from this partition with that from previous partitions
	//		parameter_count += count(all);
	//	}
	//	return parameter_count;
	//}

	///**
	// * @return total number of parameters for this process
	// */
	//inline const std::size_t getParametersCount() const {
	//	return (split_manager.getProcessRange().second - split_manager.getProcessRange().first);
	//}

	///**
	// * @return	number of colorings in the result
	// */
	//inline const std::size_t getStatesCount() const {
	//	return states.size();
	//}

	///**
	// * @param state_index	index in the vector of states
	// *
	// * @return	ID of state of the Kripke Structure this state is build from
	// */
	//inline const std::size_t & getKSNum(const std::size_t state_index) const {
	//	return states[state_index].KS_num;
	//}

	///**
	// * @param state_index	index in the vector of states
	// *
	// * @return	ID of state of the Buchi automaton this state is build from
	// */
	//inline const std::size_t & getBANum(const std::size_t state_index) const {
	//	return states[state_index].BA_num;
	//}

	///**
	// * Get part of parameters of the state
	// *
	// * @param state_index	index in the vector of states
	// * @param round_num	round to pick
	// *
	// * @return	coloring with given index
	// */
	//const Parameters getStateParameters(const std::size_t state_index, const std::size_t round_num) const {
	//	return states[state_index].parameters_parts[round_num];
	//}

	///**
	// * Get part of union of all the parameters
	// *
	// * @param round_num	round to pick
	// *
	// * @return	part of the coloring coloring with given index
	// */
	//const Parameters getMergedParameters(const std::size_t round_num) const {
	//	Parameters all = 0;
	//	// Add parameters from each other final state
	//	std::for_each(states.begin(), states.end(), [&all, round_num](const ColoredState & state){
	//		all |= state.parameters_parts[round_num];
	//	});
	//	return all;
	//}

};

#endif
