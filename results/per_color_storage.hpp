/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_PER_COLOR_STORAGE_INCLUDED
#define PARSYBONE_PER_COLOR_STORAGE_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PerColorStorage stores unique predecessors and succesors for each color
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../auxiliary/data_types.hpp"
#include "../auxiliary/common_functions.hpp"
#include "../coloring/color_storage.hpp"
#include "../construction/product_structure.hpp"

class ProductBuilder;

class PerColorStorage {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	struct ColorData {
		std::size_t color_num; // Relative number of the color in this round
		Parameters color_bit; // Bit assigned to the color
		std::vector<StateID> predecessors; // Predecessors only for this color
		std::vector<StateID> successors; // Succesors only for this color
	};
	
	struct State{
		StateID ID;
		std::vector<ColorData> color_data;
	};

	const ColorStorage & storage;
	const ProductStructure & product;
	const ColoringAnalyzer & analyzer;
	
	// DATA STORAGE
	std::vector<State> states;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	PerColorStorage(const PerColorStorage & other);            // Forbidden copy constructor.
	PerColorStorage& operator=(const PerColorStorage & other); // Forbidden assignment operator.

public:
	PerColorStorage(const ColoringAnalyzer & _analyzer, const ColorStorage & _storage, const ProductStructure & _product) : analyzer(_analyzer), storage(_storage), product(_product) {
		auto colors = analyzer.getColors();

		states.resize(product.getStateCount());
		for (StateID ID = 0; ID < product.getStateCount(); ID++) {
			states[ID].ID = ID;
		}

		// Go through colors
		for (auto color_it = colors.begin(); color_it != colors.end(); color_it++) {

			// Get round values
			std::size_t color_num = getBitNum(color_it->first);
			Parameters color_bit = color_it->first;

			for (auto state_it = states.begin(); state_it != states.end(); state_it++) {
				ColorData color_data;
				color_data.color_bit = color_bit;
				color_data.color_num = color_num;
				color_data.successors = std::move(storage.getNeighbours(state_it->ID, true, color_bit));
				color_data.predecessors = std::move(storage.getNeighbours(state_it->ID, false, color_bit));
				state_it->color_data.push_back(std::move(color_data));
			}
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/** 
	 * Get all the neigbours for this color from this state.
	 *
	 * @param ID	index of the state to ask for predecessors
	 * @param successors	true if successors are required, false if predecessors
	 * @param number	ordinal number of stored coloring
	 *
	 * @return neigbours for given state
	 */
	inline const Neighbours & getNeighbours(const StateID ID, const bool successors, std::size_t number) const {
		return successors ? states[ID].color_data[number].successors : states[ID].color_data[number].predecessors;
	}
};

#endif
