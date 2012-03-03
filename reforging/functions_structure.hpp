/*
 * Copyright (C) 2012 - Adam Streck
 *
 * This file is part of PoSeIDoN (Parameter Synthetizer for Discrete Networks) verification tool
 *
 * Poseidon is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Poseidon is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 *
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno.
 * See http://sybila.fi.muni.cz/ .
 */

#ifndef POSEIDON_FUNCTIONS_STRUCTURE_INCLUDED
#define POSEIDON_FUNCTIONS_STRUCTURE_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FunctionsStructure stores functions in an explicit, but utilizable form.
// Functions are stored as values that are requested for the function to be active (exact values for given regulatory context).
// There are auxiliary precomputed data that fasten usage of this structure.
// FunctionsStructure data can be set only form the FunctionsBuilder object.
// Rest of the code can access the data only via constant getters.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class FunctionsBuilder;

class FunctionsStructure {
	friend class FunctionsBuilder;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Storing a regulatory function in explicit form
	struct RegulatoryFunction {

	private:
		friend class FunctionsStructure;

		std::size_t target_specie; // Specie that is regulated.
		std::vector<std::size_t> source_species; // Regulating species (those who have outcoming interaction to the target of the function, ergo both active and inactive)
		std::vector<std::size_t> possible_values; // The levels the target can occur in, same for all the functions of one specie
		std::vector<std::vector<std::size_t> > source_values;  // Values at which the regulations are active

		RegulatoryFunction(std::size_t _target_specie, std::vector<std::size_t> && _source_species, std::vector<std::vector<std::size_t> > && _source_values, 
			               std::vector<std::size_t> && _possible_values)
			: target_specie(_target_specie), source_species(std::move(_source_species)), 
			source_values(std::move(_source_values)), possible_values(std::move(_possible_values)) {}
	};
	
	std::vector<RegulatoryFunction> reg_functions;
	// auxiliary data
	std::vector<std::size_t> specie_begin; // lenght = |Species| Positions of reg_functions where functions that regulates specie i begin
	std::vector<std::size_t> step_sizes; // lenght = |Regulatory functions| How many neighbour parameters have the same value for this function 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FILLING FUNCTIONS (can be used only from FunctionsBuilder)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Add a new function - consist of target, IDs of regulators (both active and inactive) and levels they have to be at, and possible parameter values
	 */
	inline void addRegulatoryFunction(std::size_t _target_specie, std::vector<std::size_t> && _source_species, std::vector<std::vector<std::size_t> > && _source_values, 
		                       std::vector<std::size_t> && _possible_values_count) {
		reg_functions.push_back(RegulatoryFunction(_target_specie, std::move(_source_species), 
			std::move(_source_values), std::move(_possible_values_count)));
	}

	/**
	 * @param position	number of the first function with target specie equal to vector index
	 */
	inline void addSpecieBegin(const std::size_t position) {
		specie_begin.push_back(position);
	}

	/**
	 * @param number	how many parameter steps it takes to change the value of the current function
	 */
	inline void addStepSize(const std::size_t step_size) {
		step_sizes.push_back(step_size);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OTHER FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	FunctionsStructure(const FunctionsStructure & other);            // Forbidden copy constructor.
	FunctionsStructure& operator=(const FunctionsStructure & other); // Forbidden assignment operator.

public:
	FunctionsStructure() {} // Default empty constructor
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	 * @return	size of the parameter space
	 */
	const std::size_t getParametersCount() const {
		std::size_t parameters_count = 1;
		std::for_each(reg_functions.begin(), reg_functions.end(), [&parameters_count](const RegulatoryFunction & function) {
			parameters_count *= function.possible_values.size();
		});
		return parameters_count;
	}

	/**
	 * @return	number of single regulatory functions
	 */
	inline const std::size_t getFunctionsCount() const {
		return reg_functions.size();
	}

	/**
	 * @param specie_num	number of the specie that is regulated
	 *
	 * @return	position in the functions vector where functions regulating specie_num starts
	 */
	inline const std::size_t getSpecieBegin(std::size_t specie_num) const {
		return specie_begin[specie_num];
	}

	/**
	 * @param function_num	number of the funcion
	 *
	 * @return	number of neighbour bits in the bitset that share the same value for this function
	 */
	inline const std::size_t getStepSize(std::size_t function_num) const {
		return step_sizes[function_num];
	}

	/**
	 * @return	vector of the step sizes for the functions
	 */
	inline const std::vector<std::size_t> getStepSizes() const {
		return step_sizes;
	}

	/**
	 * @param ID	ID of the function to get data from
	 *
	 * @return	target of the function
	 */
	inline const std::vector<std::size_t>& getPossibleValues(std::size_t ID) const {
		return reg_functions[ID].possible_values;
	}

	/**
	 * @param ID	ID of the function to get data from
	 *
	 * @return	target of the function
	 */
	inline const std::vector<std::size_t>& getSourceSpecies(std::size_t ID) const {
		return reg_functions[ID].source_species;
	}

	/**
	 * @param ID	ID of the function to get data from
	 *
	 * @return	target of the function
	 */
	inline const std::vector<std::vector<std::size_t> >&  getSourceValues(std::size_t ID) const {
		return reg_functions[ID].source_values;
	}

	/**
	 * @param ID	ID of the function to get data from
	 *
	 * @return	target of the function
	 */
	inline const std::size_t getTarget(std::size_t ID) const {
		return reg_functions[ID].target_specie;
	}
};

#endif