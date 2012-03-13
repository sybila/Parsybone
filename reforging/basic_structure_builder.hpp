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

#ifndef PARSYBONE_BASIC_STRUCTURE_BUILDER_INCLUDED
#define PARSYBONE_BASIC_STRUCTURE_BUILDER_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BasicStructureBuilder creates the BasicStructure (Simple Kripke Structure) from the model data.
// In each iteration of the creation, a new state is generated as a cartesian product of values of the species. All the combinations are used.
// Each state is provided with indexes of their neighbours. For each dimension (specie) there are three neighbours base on the change of the specie's value - up, stay or donw.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <memory>
#include <limits>

#include "../parsing/model.hpp"
#include "basic_structure.hpp"

class BasicStructureBuilder {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Provided with constructor
	const UserOptions & user_options;
	const Model & model; // Model that holds the data
	BasicStructure & structure; // KipkeStructure to fill

	// Computed
	std::size_t states_count;
	std::size_t species_count;
	std::vector<std::size_t> index_jumps; // Holds index differences between two neighbour states in each direction for each specie

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTING FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Compute a vector of maximal levels 
	 *
	 * @param state_levels	vector of levels that is initialized to zero
	 *
	 * @return	vector of maximal levels of species
	 */
	const Levels getMaxes(Levels & state_levels) {
		Levels maxes; 
		for(std::size_t specie_num = 0; specie_num < species_count; specie_num++) {
			state_levels.push_back(0);
			maxes.push_back(model.getMax(specie_num));
			states_count *= (model.getMax(specie_num) + 1);
		}
		return maxes;
	}

	/**
	 * Creates a vector of index differences between neighbour states in each direction.
	 * Value is difference of the indexes between two states that were generated as a cartesian product.
	 * Differences are caused by the way the states are generated.
	 */
	void computeJumpDifferences() {
		std::size_t jump_lenght = 1;
		// Species with higher index cause bigger differences
		for (std::size_t specie_num = 0; specie_num < species_count; specie_num++) {
			index_jumps[specie_num] = jump_lenght;
			jump_lenght *= (model.getMax(specie_num) + 1);
		}
	}

	/**
	 * Compute indexes of the neighbour states of this state and pass them to the state. For each dimension store self and upper and lower neighbour. 
	 * If none such exists, store max value of std::size_t. 
	 *
	 * @param state_num	 ID of the current state
	 * @param state_levels	levels of species of this state
	 * @param maxes	globally maximal levels
	 */
	void storeNeigbours(const std::size_t state_num, const Levels & state_levels, const Levels & maxes) {
		for (std::size_t specie = 0; specie < species_count; specie++) {
			// If this value is not the lowest one, add neighbour with lower
			if (state_levels[specie] > 0) 
				structure.addNeighbour(state_num, state_num - index_jumps[specie], specie, down);

			// Add yourself
			structure.addNeighbour(state_num, state_num, specie, stay);

			// If this value is not the highest one, add neighbour with higher
			if (state_levels[specie] < maxes[specie]) 
				structure.addNeighbour(state_num, state_num + index_jumps[specie], specie, up);
		}	
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTING FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	BasicStructureBuilder(const BasicStructureBuilder & other);            // Forbidden copy constructor.
	BasicStructureBuilder& operator=(const BasicStructureBuilder & other); // Forbidden assignment operator.

public:
	/**
	 * Constructor initializes basic information from the model
	 */
	BasicStructureBuilder(const UserOptions &_user_options, const Model & _model, BasicStructure & _structure) 
		: user_options(_user_options), model(_model), structure(_structure), states_count(1)  {
		species_count = model.getSpeciesCount();
		index_jumps.resize(species_count);
		computeJumpDifferences();
	}

	/**
	 * Create the states from the model and fill the structure with them.
	 */
	void buildStructure() {
		Levels state_levels; // Initialized to zeroes
		Levels maxes = std::move(getMaxes(state_levels)); 

		// Create states 
		for(std::size_t state_num = 0; state_num < states_count; state_num++) {
			// Fill the structure with the state
			structure.addState(state_num, state_levels);
			storeNeigbours(state_num, state_levels, maxes);

			// Generate next combination of levels i.e. new state of the KS - using standard method for cartesian product generation
			int position = 0; // ID of specie to iterate
			std::for_each(state_levels.begin(), state_levels.end(),[&maxes, &position](std::size_t & level) {
				// Values are increased from left to right
				if (position == -1) ;
				// Is max? Set to 0 and continue rights.
				else if (level == maxes[position]) {
					level = 0;
					position++;
				}
				// Else increase and inform about ending.
				else {
					level++;
					position = -1;
				}
			});
		}
	}
};

#endif