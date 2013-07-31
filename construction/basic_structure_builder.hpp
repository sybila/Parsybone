/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_BASIC_STRUCTURE_BUILDER_INCLUDED
#define PARSYBONE_BASIC_STRUCTURE_BUILDER_INCLUDED

#include "PunyHeaders/common_functions.hpp"

#include "../parsing/model_translators.hpp"
#include "basic_structure.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Creates a full state space as a simple graph as a BasicStructure object.
///
/// BasicStructureBuilder creates the BasicStructure (Simple Kripke Structure) from the model data.
/// In each iteration of the creation, a new state is generated as a cartesian product of values of the species. All the combinations are used.
/// Each state is provided with indexes of their neighbours. For each dimension (specie) there are three neighbours, if possible, base on the change of the specie's value - up, stay or down.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class BasicStructureBuilder {
	// Provided with constructor
   const Model & model; ///< Model that holds the data.
   const size_t species_count; ///< Number of species of the model.
   BasicStructure & structure; ///< KipkeStructure to fill.

	// Computed
   size_t states_count; ///< Number of states in this KS (exponential in number of species).
   vector<size_t> index_jumps; ///< Holds index differences between two neighbour states in each direction for each specie.
   Levels maxes; ///< Maximal activity levels of the species.
   Levels mins; ///< Minimal activity levels of the species.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FILLING METHODS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Compute indexes of the neighbour states of this state and pass them to the state. For each dimension store self and upper and lower neighbour. 
	 * If none such exists, store max value of size_t. 
	 *
	 * @param state_num	 ID of the current state
	 * @param state_levels	levels of species of this state
	 * @param maxes	globally maximal levels
	 */
	void storeNeigbours(const StateID ID, const Levels & state_levels, const Levels & maxes) {
		for (size_t specie = 0; specie < species_count; specie++) {
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
// CONSTRUCTING METHODS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Compute a vector of maximal levels and store information about states
	 */
	void computeBoundaries() {
		states_count = 1;
		for(size_t specie_num = 0; specie_num < species_count; specie_num++) {
			// Maximal values of species
			maxes.push_back(model.getMax(specie_num));
			mins.push_back(model.getMin(specie_num));
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
		size_t jump_lenght = 1;
		// Species with higher index cause bigger differences
		for (size_t specie_num = 0; specie_num < species_count; specie_num++) {
			index_jumps[specie_num] = jump_lenght;
			jump_lenght *= (model.getMax(specie_num) + 1);
		}
	}

	/**
	 * Creates a label of a state from its activation levels
	 *
	 * @param levels	current activation levels of the specie
	 */
	const string createLabel(const Levels & levels) const {
		string state_string = "(";
		// Add species levels
		for (auto spec_it = levels.begin(); spec_it != levels.end(); spec_it++) {
			state_string += toString(*spec_it);
			state_string += ",";
		}
		// End the state
      state_string[state_string.length() - 1] = ')';

		return state_string;
	}
	
	BasicStructureBuilder(const BasicStructureBuilder & other); ///<  Forbidden copy constructor.
	BasicStructureBuilder& operator=(const BasicStructureBuilder & other); ///<  Forbidden assignment operator.

public:
	/**
	 * Constructor initializes basic information from the model
	 */
	BasicStructureBuilder(const Model & _model, BasicStructure & _structure) 
                         : model(_model), species_count(_model.species.size()), structure(_structure)  {
		// Compute species-related values
		computeBoundaries();
		// Compute transitions-related values
		computeJumps();
	}

	/**
	 * Create the states from the model and fill the structure with them.
	 */
	void buildStructure() {
      size_t transition_count = 0;

		// Create initial state (by its values)
		Levels levels(species_count, 0);

		// Create states 
      StateID ID = 0;
      do {
			// Fill the structure with the state
			structure.addState(ID, levels, move(createLabel(levels)));
			storeNeigbours(ID, levels, maxes);
         // Generate new state for the next round
         // Counting function - due to the fact, that self-loop is possible under all the species, the number has to be tweaked to account for just one self-loop.
         transition_count += structure.getTransitionCount(ID) - model.species.size() + 1;
         ID++;
      } while (iterate(maxes, mins, levels));

      output_streamer.output(verbose_str, "Transition system has " + toString(states_count) + " states with " + toString(transition_count) + " transitions.");
	}
};

#endif // PARSYBONE_BASIC_STRUCTURE_BUILDER_INCLUDED
