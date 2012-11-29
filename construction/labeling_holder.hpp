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
/// \brief Storage for the regulatory graph with kinetic parameters encoded in form of regulatory functions.
///
/// LabelingHolder contains basic representation of the Gene Regulatory network in the form of the labeled graph.
/// Each specie is stored together with its regulations. Each regulation has its step_size value (shared by multiple regulations).
/// This value represents division of parametrization space and is used for encoding and decoding it into paramset.
/// LabelingHolder data can be set only form the LabelingBuilder object.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class LabelingHolder {
	friend class LabelingBuilder;

   /// Storing a regulatory function in explicit form.
	struct RegulatoryFunction {
      size_t step_size; ///< How many neighbour parameters have the same value for this function.
      Levels possible_values; ///< Levels towards which this function can regulate.
      Configurations source_values;  ///< Values at which the regulations are active.

      RegulatoryFunction(const size_t _step_size, const vector<size_t> & _possible_values, const  vector<vector<size_t> > & _source_values)
         : step_size(_step_size), possible_values(_possible_values), source_values(_source_values){} ///< Simple filler, assigns values to all the variables.
	};
	
   /// Storing a sigle specie with its regulations.
	struct Specie {
      string name; ///< Real name of the specie.
      size_t ID; ///< Reference number.
      vector<size_t> source_species; ///< IDs of regulators.

      vector<RegulatoryFunction> functions; ///< Regulatory functions - set of all possible regulatory kinetics.
	
		Specie(const string _name, const size_t _ID, const vector<size_t> & _source_species)
			  : name(_name), ID(_ID), source_species(_source_species) {} ///< Simple filler, assigns values to all the variables.
	};

   /// Vector of all the species together with their Regulatory functions.
	vector<Specie> species;

   /// Total number of parameters (colors).
	size_t parameter_count;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FILLING METHODS (can be used only from LabelingBuilder)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
    * Add a new function - consist of target, step_size and values in which this function is active.
	 *
	 * @param source_ID	ID of the spicie whose function this is
	 */
	inline void addRegulatoryFunction(const size_t target_ID, const size_t step_sizes, 
												 const vector<size_t> & possible_values, const vector<vector<size_t> > & source_values) {
		species[target_ID].functions.push_back(RegulatoryFunction(step_sizes, possible_values, source_values));
	}

	/**
    * Add a new specie - consists of its name within a string, ID, possible values vector and source species IDs vector.
	 */
	inline void addSpecie(const string name, const size_t ID, const vector<size_t> & source_species) {
		species.push_back(Specie(name, ID, source_species));
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
	inline size_t getParametersCount() const {
		return parameter_count;
	}

	/**
	 * @return	number of the species
	 */
	inline size_t getSpeciesCount() const {
		return species.size();
	}

	/**
	 * @return	name of the specie with given ID
	 */
	inline const string & getSpecieName(const size_t ID) const {
		return species[ID].name;
	}

	/**
	 * @return	IDs of all the species that regulate this specie
	 */
	inline const vector<size_t> & getSourceSpecies(const size_t ID) const {
		return species[ID].source_species;
	}

	/**
	 * @return	number of regulations for this specie (two to power of number of source species)
	 */
	inline size_t getRegulationsCount(const size_t ID) const {
		return species[ID].functions.size();
	}

	/**
	 * @return	step_size (how many neigbour parameters share the same value for this regulation)
	 */
	size_t getStepSize(const size_t ID, const size_t regulation) const {
		return species[ID].functions[regulation].step_size;
	}

	/**
	 * @return	values this function can possibly regulate to
	 */
	const vector<size_t> & getPossibleValues(const size_t ID, const size_t regulation) const {
		return species[ID].functions[regulation].possible_values;
	}

	/**
	 * @return	for each source specie all the values that if it is within them, it allows this function
	 */
	const vector<vector<size_t> > & getSourceValues(const size_t ID, const size_t regulation) const {
		return species[ID].functions[regulation].source_values;
	}
};

#endif // PARSYBONE_LABELING_HOLDER_INCLUDED
