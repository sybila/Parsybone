/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_MODEL_INCLUDED
#define PARSYBONE_MODEL_INCLUDED

#include "../auxiliary/output_streamer.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Model stores model data in the raw form, almost the same as in the model file itself.
/// Model data can be set only form the ModelParser object.
/// Rest of the code can access the data only via constant getters - once the data are parse, model remains constant.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Model {
	friend class ModelParser;
	friend class NetworkParser;
	friend class PropertyParser;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/// Structure that stores regulation of a specie by another one
	struct Regulation {
		StateID source; ///< Regulator
		std::size_t threshold; ///< Level of the regulator requested for the function to work
		EdgeConstrain constrain; ///< What is requested behaviour
		bool observable; ///< Must the regulation be observable

		Regulation(const StateID _source, const std::size_t _threshold, const EdgeConstrain _constrain, const bool _observable)
			: source(_source), threshold(_threshold), constrain(_constrain), observable(_observable) { }
	};

	typedef std::pair<std::vector<bool>, int> Parameter; ///< Kinetic parameter of the specie (bitmask of active incoming regulations, target value)
	typedef std::pair<StateID, std::string> Egde; ///< Edge in Buchi Automaton (Target ID, edge label)

private:
	/// Structure that holds data about a single specie. Most of the data is equal to that in the model file
	struct ModelSpecie {
	private:
		// Data are accesible from within the model class
		friend class Model;

		std::vector<Regulation> regulations;

		std::vector<Parameter> parameters;

		std::string name; ///< Actuall name of the specie
		std::size_t ID; ///< Numerical constant used to distinguish the specie. Starts from 0!
		std::size_t max_value; ///< Maximal activation level of the specie
		std::size_t basal_value; ///< Value the specie tends to if unregulated, currently unused

		ModelSpecie(std::string _name, size_t _ID, size_t _max_value, size_t _basal_value)
			: name(_name), ID(_ID), max_value(_max_value), basal_value(_basal_value) { }
	};

	/// Structure that holds data about a single state.
	struct BuchiAutomatonState {
	private:
		// Data are accesible from within the model class
		friend class Model;

		std::size_t ID; ///< Numerical constant used to distinguish the state. Starts from 0!
		bool final; ///< stores the information whether the state is final

		std::vector<Egde> edges; ///< edges in Buchi Automaton (Target ID, edge label)

		BuchiAutomatonState(std::size_t _ID, bool _final) : ID(_ID), final(_final) {	}
	};

	/// Structure that stores additional settings
	struct AdditionalInformation {
		UnspecifiedParameters unspec;
		float ver_number;
	};

	// Actuall data holders.
	std::vector<ModelSpecie> species;
	std::vector<BuchiAutomatonState> states;
	AdditionalInformation additional_information;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FILLING FUNCTIONS (can be used only from ModelParser)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Create a new specie with the provided name, basal and maximal value
	 *
	 * @return	index of specie in the vector
	 */
	inline std::size_t addSpecie(std::string _name, size_t _max_value, size_t _basal_value) {
		species.push_back(ModelSpecie(_name, species.size(), _max_value, _basal_value));
		return species.size() - 1;
	}

	/**
	 * Add a new regulation to the specie. Regulation is stored with the target, not the source.
	 */
	inline void addRegulation(size_t source_ID, size_t target_ID, size_t threshold, EdgeConstrain constrain, const bool observable) {
		species[target_ID].regulations.push_back(std::move(Regulation(source_ID, threshold, constrain, observable)));
	}

	/**
	 * Add a new regulation to the specie.
	 */
	inline void addParameter(size_t target_ID, const std::vector<bool> subset_mask, int target_value) {
		species[target_ID].parameters.push_back(Parameter(subset_mask, target_value));
	}

	/**
	 * Add a new state to the automaton.
	 *
	 * @return	ID of state in the vector
	 */
	inline std::size_t addState(bool final) {
		states.push_back(BuchiAutomatonState(states.size(), final));
		return states.size() - 1;
	}

	/**
	 * Add a new transition - transition is specified by the target state and label
	 */
	inline void addConditions(StateID source_ID, StateID target_ID, std::string && edge_label) {
		states[source_ID].edges.push_back(Egde(target_ID, std::move(edge_label)));
	}

	/**
	 * Fill additional information
	 *
	 * @param unspec	what to do with unspecified regulations
	 * @param ver_number	float number with version of the model
	 */
	void addAdditionalInformation(UnspecifiedParameters unspec, float ver_number) {
		additional_information.unspec = unspec;
		additional_information.ver_number = ver_number;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTING FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	Model(const Model & other); ///< Forbidden copy constructor.
	Model& operator=(const Model & other); ///< Forbidden assignment operator.

public:
	Model() {} ///< Default empty constructor

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @return	number of the species
	 */
	inline const std::size_t getSpeciesCount() const {
		return species.size();
	}

	/**
	 * @return	number of the states
	 */
	inline const std::size_t getStateCount() const {
		return states.size();
	}

	/**
	 * Finds numerical ID of the specie based on its name or ID string
	 *
	 * @return	ID of the specie with the specified name if there is such, otherwise ~0
	 */
	const SpecieID findID(const std::string name) const {
		SpecieID ID = ~0;
		try { // Try direct translation
			ID = boost::lexical_cast<StateID, std::string>(name);
		}
		catch (boost::bad_lexical_cast) { // Try lookup by name
			std::for_each(species.begin(), species.end(), [&ID, &name](ModelSpecie spec) {
				if (spec.name.compare(name) == 0)
				ID = spec.ID;
			});
		}
		return ID;
	}

	/**
	 * @return	name of the specie
	 */
	inline const std::string & getName(const std::size_t ID) const {
		return species[ID].name;
	}

	/**
	 * @return	minimal value of the specie (always 0)
	 */
	inline const std::size_t getMin(const std::size_t ID) const {
		return ID ? 0 : 0; // Just to disable a warning
	}

	/**
	 * @return	maximal value of the specie
	 */
	inline const std::size_t getMax(const std::size_t ID) const {
		return species[ID].max_value;
	}

	/**
	 * @return	basal value of the specie
	 */
	inline const std::size_t getBasal(const std::size_t ID) const {
		return species[ID].basal_value;
	}

	/**
	 * @return	regulations of the specie
	 */
	inline const std::vector<Regulation> & getRegulations(const std::size_t ID) const {
		return species[ID].regulations;
	}

	/**
	 * @return	kinetic parameters of the regulations of the specie
	 */
	inline const std::vector<Parameter> & getParameters(const std::size_t ID)  const {
		return species[ID].parameters;
	}

	/**
	 * @return	true if the state is final
	 */
	inline const bool isFinal(const std::size_t ID) const {
		return states[ID].final;
	}

	/**
	 * @return	edges of the state
	 */
	inline const std::vector<Egde> & getEdges(const std::size_t ID) const {
		return states[ID].edges;
	}
};


#endif
