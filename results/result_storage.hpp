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
// Results are used by ModelChecker to store the data computed during synthesis.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../auxiliary/output_streamer.hpp"
#include "../coloring/split_manager.hpp"
#include "../coloring/synthesis_manager.hpp"
#include "../reforging/automaton_structure.hpp"
#include "../reforging/parametrized_structure.hpp"
#include "../reforging/product_structure.hpp"

class ModelChecker;

class ResultStorage {
	friend class SynthesisManager;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const ProductStructure & product; // Referecnce to product

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTING FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	

	ResultStorage(const ResultStorage & other);            // Forbidden copy constructor.
	ResultStorage& operator=(const ResultStorage & other); // Forbidden assignment operator.

public:
	/**
	 * Get reference data and create final states that will hold all the computed data
	 */
	ResultStorage(const ProductStructure & _product) 
		   : product(_product) { } 
	
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
