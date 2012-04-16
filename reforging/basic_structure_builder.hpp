/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_BASIC_STRUCTURE_BUILDER_INCLUDED
#define PARSYBONE_BASIC_STRUCTURE_BUILDER_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BasicStructureBuilder creates the BasicStructure (Simple Kripke Structure) from the model data.
// In each iteration of the creation, a new state is generated as a cartesian product of values of the species. All the combinations are used.
// Each state is provided with indexes of their neighbours. For each dimension (specie) there are three neighbours base on the change of the specie's value - up, stay or donw.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../parsing/model.hpp"
#include "basic_structure.hpp"

class BasicStructureBuilder {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Provided with constructor
	const Model & model; // Model that holds the data
	BasicStructure & structure; // KipkeStructure to fill
	std::size_t species_count; // Number of species of the model

	// Computed
	std::size_t states_count; // Number of states in this KS (exponential in number of species)
	std::vector<std::size_t> index_jumps; // Holds index differences between two neighbour states in each direction for each specie
	Levels maxes; // Maximal activity levels of the species

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FILLING FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Compute indexes of the neighbour states of this state and pass them to the state. For each dimension store self and upper and lower neighbour. 
	 * If none such exists, store max value of std::size_t. 
	 *
	 * @param state_num	 ID of the current state
	 * @param state_levels	levels of species of this state
	 * @param maxes	globally maximal levels
	 */
	void storeNeigbours(const StateID ID, const Levels & state_levels, const Levels & maxes) {
		for (std::size_t specie = 0; specie < species_count; specie++) {
			// If this value is not the lowest one, add neighbour with lower
			if (state_levels[specie] > 0) 
				structure.addNeighbour(ID, ID - index_jumps[specie], specie, down_dir);

			// Add yourself
			structure.addNeighbour(ID, ID, specie, stay_dir);

			// If this value is not the highest one, add neighbour with higher
			if (state_levels[specie] < maxes[specie]) 
				structure.addNeighbour(ID, ID + index_jumps[specie], specie, up_dir);
		}	
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTATION FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Generate next combination of levels i.e. new state of the KS - using standard method for cartesian product generation
	 */
	Levels getNextState(Levels state_levels) const {
		// ID of specie to iterate
		int position = 0; 
		// Iterate through species
		for (auto specie_level = state_levels.begin(); specie_level != state_levels.end(); specie_level++) {
			if (*specie_level == maxes[position]) {	// Is max? Set to 0 and continue rights. 
				(*specie_level) = 0;
				position++;
			}
			else { // Else increase and return
				(*specie_level)++;
				return state_levels;
			}
		}
		// All-zero is returned after last round
		return state_levels;
	}
	
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTING FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Compute a vector of maximal levels and store information about states
	 */
	void computeBoundaries() {
		states_count = 1;
		for(std::size_t specie_num = 0; specie_num < species_count; specie_num++) {
			// Maximal values of species
			maxes.push_back(model.getMax(specie_num));
			// How many states
			states_count *= (model.getMax(specie_num) + 1);
		}
	}

	/**
	 * Creates a vector of index differences between neighbour states in each direction.
	 * Value is difference of the indexes between two states that were generated as a cartesian product.
	 * Differences are caused by the way the states are generated.
	 */
	void computeJumps() {
		index_jumps.resize(species_count);
		// How many far away are two neighbour in the vector
		std::size_t jump_lenght = 1;
		// Species with higher index cause bigger differences
		for (std::size_t specie_num = 0; specie_num < species_count; specie_num++) {
			index_jumps[specie_num] = jump_lenght;
			jump_lenght *= (model.getMax(specie_num) + 1);
		}
	}
	
	BasicStructureBuilder(const BasicStructureBuilder & other);            // Forbidden copy constructor.
	BasicStructureBuilder& operator=(const BasicStructureBuilder & other); // Forbidden assignment operator.

public:
	/**
	 * Constructor initializes basic information from the model
	 */
	BasicStructureBuilder(const Model & _model, BasicStructure & _structure) 
		                 : model(_model), structure(_structure), species_count(_model.getSpeciesCount())   {
		// Compute species-related values
		computeBoundaries();
		// Compute transitions-related values
		computeJumps();
	}

	/**
	 * Create the states from the model and fill the structure with them.
	 */
	void buildStructure() {
		output_streamer.output(stats_str, "Computing Kripke structure states, total number of states: ", OutputStreamer::no_newl)
			           .output(states_count, OutputStreamer::no_newl).output(".");

		// Create initial state (by its values)
		Levels state_levels(species_count, 0);

		// Create states 
		for(std::size_t state_num = 0; state_num < states_count; state_num++) {
			// Fill the structure with the state
			structure.addState(state_num, state_levels);
			storeNeigbours(state_num, state_levels, maxes);
			// Generate new state for the next round
			state_levels = std::move(getNextState(state_levels));
		}
	}
};

#endif