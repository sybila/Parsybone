/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_LABELING_HOLDER_INCLUDED
#define PARSYBONE_LABELING_HOLDER_INCLUDED

#include "../auxiliary/output_streamer.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// LabelingHolder contains basic representation of the Gene Regulatory network in the form of the labeled graph.
/// Each specie is stored together with its regulations. Each regulation has its step_size value (shared by multiple regulations).
/// This value represents division of parametrization space and is used for encoding and decoding it into paramset.
/// LabelingHolder data can be set only form the LabelingBuilder object.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class LabelingHolder {
	friend class LabelingBuilder;

   /// Storing a regulatory function in explicit form.
	struct RegulatoryFunction {
      std::size_t step_size; ///< How many neighbour parameters have the same value for this function.
      std::vector<std::size_t> possible_values; ///< Levels towards which this function can regulate.
      std::vector<std::vector<std::size_t> > source_values;  ///< Values at which the regulations are active.

		RegulatoryFunction(const std::size_t _step_size, std::vector<std::size_t> && _possible_values, std::vector<std::vector<std::size_t> > && _source_values)
         : step_size(_step_size), possible_values(std::move(_possible_values)), source_values(std::move(_source_values)){} ///< Simple filler, assigns values to all the variables.
	};
	
   /// Storing a sigle specie with its regulations.
	struct Specie {
      std::string name; ///< Real name of the specie.
      std::size_t ID; ///< Reference number.
      std::vector<std::size_t> specie_values; ///< Levels this specie can occur in.
      std::vector<std::size_t> source_species; ///< IDs of regulators.

      std::vector<RegulatoryFunction> functions; ///< Regulatory functions - set of all possible regulatory kinetics.
	
		Specie(const std::string _name, const std::size_t _ID, std::vector<std::size_t> && _specie_values, std::vector<std::size_t> && _source_species)
           : name(_name), ID(_ID), specie_values(std::move(_specie_values)), source_species(std::move(_source_species)) {} ///< Simple filler, assigns values to all the variables.
	};

   /// Vector of all the species together with their Regulatory functions.
	std::vector<Specie> species;

   /// Total number of parameters (colors).
	std::size_t parameter_count;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FILLING METHODS (can be used only from LabelingBuilder)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
    * Add a new function - consist of target, step_size and values in which this function is active.
	 *
	 * @param source_ID	ID of the spicie whose function this is
	 */
	inline void addRegulatoryFunction(const std::size_t target_ID, const std::size_t step_sizes, 
                                     std::vector<std::size_t> && possible_values, std::vector<std::vector<std::size_t> > && source_values) {
		species[target_ID].functions.push_back(RegulatoryFunction(step_sizes, std::move(possible_values), std::move(source_values)));
	}

	/**
    * Add a new specie - consists of its name within a string, ID, possible values vector and source species IDs vector.
	 */
	inline void addSpecie(const std::string name, const std::size_t ID, std::vector<std::size_t> && specie_values, std::vector<std::size_t> && source_species) {
		species.push_back(Specie(name, ID, std::move(specie_values), std::move(source_species)));
	}

	LabelingHolder(const LabelingHolder & other); ///< Forbidden copy constructor.
	LabelingHolder& operator=(const LabelingHolder & other); ///< Forbidden assignment operator.

public:
   LabelingHolder() {} ///< Default empty constructor.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @return	size of the parameter space
	 */
	inline std::size_t getParametersCount() const {
		return parameter_count;
	}

	/**
	 * @return	number of the species
	 */
	inline std::size_t getSpeciesCount() const {
		return species.size();
	}

	/**
	 * @return	name of the specie with given ID
	 */
	inline const std::string & getSpecieName(const std::size_t ID) const {
		return species[ID].name;
	}

	/**
	 * @return	all the values the specie can occur in
	 */
	inline const std::vector<std::size_t> & getSpecieValues(const std::size_t ID) const {
		return species[ID].specie_values;
	}

	/**
	 * @return	IDs of all the species that regulate this specie
	 */
	inline const std::vector<std::size_t> & getSourceSpecies(const std::size_t ID) const {
		return species[ID].source_species;
	}

	/**
	 * @return	number of regulations for this specie (two to power of number of source species)
	 */
	inline std::size_t getRegulationsCount(const std::size_t ID) const {
		return species[ID].functions.size();
	}

	/**
	 * @return	step_size (how many neigbour parameters share the same value for this regulation)
	 */
	std::size_t getStepSize(const std::size_t ID, const std::size_t regulation) const {
		return species[ID].functions[regulation].step_size;
	}

	/**
	 * @return	values this function can possibly regulate to
	 */
	const std::vector<std::size_t> & getPossibleValues(const std::size_t ID, const std::size_t regulation) const {
		return species[ID].functions[regulation].possible_values;
	}

	/**
	 * @return	for each source specie all the values that if it is within them, it allows this function
	 */
	const std::vector<std::vector<std::size_t> > & getSourceValues(const std::size_t ID, const std::size_t regulation) const {
		return species[ID].functions[regulation].source_values;
	}
};

#endif // PARSYBONE_LABELING_HOLDER_INCLUDED
