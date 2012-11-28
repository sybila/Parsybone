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
/// \brief Storage for data parsed from the model.
///
/// Model stores model data in the raw form, almost the same as in the model file itself.
/// Model data can be set only form the ModelParser object.
/// Rest of the code can access the data only via constant getters - once the data are parse, model remains constant.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Model {
	friend class AutomatonParser;
	friend class ModelParser;
	friend class NetworkParser;
   friend class TimeSeriesParser;

public:
	/// Structure that stores regulation of a specie by another one
	struct Regulation {
		StateID source; ///< Regulator specie ID.
		size_t threshold; ///< Level of the regulator required for the regulation to be active.
		string label; ///< A behavioural constrain on this edge.
		size_t number; ///< Ordinal number given to each unique regulator of a specie.

		Regulation(const StateID _source, const size_t _threshold, const string _label, const size_t _number)
			: source(_source), threshold(_threshold), label(_label), number(_number) { }
	};

	typedef pair<vector<bool>, int> Parameter; ///< Kinetic parameter of the specie (bitmask of active incoming regulations, target value)
	typedef pair<StateID, string> Egde; ///< Edge in Buchi Automaton (Target ID, edge label)

private:
	/// Structure that holds data about a single specie. Most of the data is equal to that in the model file
	struct ModelSpecie {
	private:
		// Data are accesible from within the model class
		friend class Model;

		vector<Regulation> regulations; ///< Regulations of the specie (activations or inhibitions by other species)
		vector<Parameter> parameters; ///< Kinetic parameters of the regulations

		string name; ///< Actuall name of the specie
		SpecieID ID; ///< Numerical constant used to distinguish the specie. Starts from 0!
		size_t max_value; ///< Maximal activation level of the specie
		size_t basal_value; ///< Value the specie tends to if unregulated, currently unused

		ModelSpecie(string _name, SpecieID _ID, size_t _max_value, size_t _basal_value)
			: name(_name), ID(_ID), max_value(_max_value), basal_value(_basal_value) { }
	};

	/// Structure that holds data about a single state.
	struct BuchiAutomatonState {
	private:
      friend class Model; //< Data are accesible from within the model class

		string name; ///< Label of the state.
		SpecieID ID; ///< Numerical constant used to distinguish the state. Starts from 0!
      bool final; ///< True if the state is final.

      vector<Egde> edges; ///< Edges in Buchi Automaton (Target ID, edge label).

		BuchiAutomatonState(string _name, SpecieID _ID, bool _final)
			: name(_name), ID(_ID), final(_final) {	}
	};

   /// Structure that stores additional information about the model.
   struct AdditionalInformation {
      float ver_number;
   } additional_information; ///< Single object that stores the additional information.

	// Actuall data holders.
	vector<ModelSpecie> species; ///< vector of all species of the model
	vector<BuchiAutomatonState> states; ///< vector of all states of the controlling Buchi automaton

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FILLING METHODS (can be used only from a ModelParser)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
    * Create a new specie with the provided name, basal and maximal value.
	 *
	 * @return	index of specie in the vector
	 */
	inline size_t addSpecie(string name, size_t max_value, size_t basal_value) {
		species.push_back(ModelSpecie(name, species.size(), max_value, basal_value));
		return species.size() - 1;
	}

	/**
	 * Add a new regulation to the specie. Regulation is stored with the target, not the source.
	 */
	inline void addRegulation(SpecieID source_ID, SpecieID target_ID, size_t threshold, string label, size_t number) {
		species[target_ID].regulations.push_back(Regulation(source_ID, threshold, label, number));
	}

	/**
	 * Add a new regulation to the specie.
	 */
	inline void addParameter(SpecieID target_ID, const vector<bool> subset_mask, int target_value) {
		species[target_ID].parameters.push_back(Parameter(subset_mask, target_value));
	}

	/**
	 * Add a new state to the automaton.
	 *
	 * @return	ID of state in the vector
	 */
	inline size_t addState(string name, bool final) {
		states.push_back(BuchiAutomatonState(name, states.size(), final));
		return states.size() - 1;
	}

	/**
    * Add a new transition - transition is specified by the target state and label.
	 */
	inline void addConditions(StateID source_ID, StateID target_ID, string && edge_label) {
		states[source_ID].edges.push_back(Egde(target_ID, move(edge_label)));
	}

   /**
    * Fill in additional information.
    *
    * @param ver_number	float number with version of the model
    */
   void addAdditionalInformation(float ver_number) {
      additional_information.ver_number = ver_number;
   }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTING METHODS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   /**
    * Sorts parametrs by parametrizations values increasingly from the left. Context with more regulators goes after the one with less regulators.
    * Ordering for three species would be none, 1, 2, 3, 12, 13, 23, 123.
    */
   void sortParameters() {
      // Check all species
      forEach(species,[&](ModelSpecie & specie){
         // Sort parameters of the specie
         sort(specie.parameters.begin(), specie.parameters.end(), [](const Parameter & a, const Parameter & b) -> bool {
            // Control number of active regulators - specie with less regulators is considered bigger
            auto a_it = a.first.rbegin(); auto b_it = b.first.rbegin();
            size_t on_a = 0; size_t on_b = 0;
            while (a_it != a.first.rend()){
               on_a += *a_it++;
               on_b += *b_it++;
            }
            if (on_a != on_b)
               return on_a < on_b; // A goes first if it has less regulators

            // If numbers of activer regulators are equal, bigger is the one who has regulators sorted on the left
            auto a_it2 = a.first.begin(); auto b_it2 = b.first.begin();
            while (a_it2 != a.first.end()){
               if (*a_it2 > *b_it2)
                  return true; // A goes first if it has mostleft context
               else if(*a_it2 < *b_it2)
                  return false;
               a_it2++; b_it2++;
            }
            throw runtime_error("Multiple context definitions detected.");
         });
      });
   }

   Model(const Model & other); ///< Forbidden copy constructor.
	Model& operator=(const Model & other); ///< Forbidden assignment operator.

public:
   Model() {} ///< Default empty constructor.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @return	number of the species
	 */
	inline size_t getSpeciesCount() const {
		return species.size();
	}

	/**
	 * @return	number of the states
	 */
	inline size_t getStatesCount() const {
		return states.size();
	}

	/**
    * Finds numerical ID of the specie based on its name or ID string.
	 *
	 * @return	ID of the specie with the specified name if there is such, otherwise INF
	 */
	SpecieID findID(const string & name) const {
		SpecieID ID = INF;
		for_each(species.begin(), species.end(), [&ID, &name](const ModelSpecie & spec) {
					if (spec.name.compare(name) == 0)
					ID = spec.ID;
		});
		return ID;
	}


	/**
    * Finds ordinal number of the BA state based on its name or number string.
	 *
	 * @return	number of the state with the specified name if there is such, otherwise INF
	 */
	SpecieID findNumber(const string & name) const {
		StateID ID = INF;
		for_each(states.begin(), states.end(), [&ID, &name](const BuchiAutomatonState & state) {
					if (state.name.compare(name) == 0)
					ID = state.ID;
		});
		return ID;
	}

	/**
	 * @return	name of the specie
	 */
	inline const string & getName(const SpecieID ID) const {
		return species[ID].name;
	}

	/**
	 * @return	minimal value of the specie (always 0)
	 */
	inline size_t getMin(const SpecieID ID) const {
		return ID ? 0 : 0; // Just to disable a warning
	}

	/**
	 * @return	maximal value of the specie
	 */
	inline size_t getMax(const SpecieID ID) const {
		return species[ID].max_value;
	}

	/**
	 * @return	basal value of the specie
	 */
	inline size_t getBasal(const SpecieID ID) const {
		return species[ID].basal_value;
	}

	/**
	 * @return	regulations of the specie
	 */
	inline const vector<Regulation> & getRegulations(const SpecieID ID) const {
		return species[ID].regulations;
	}

	/**
	 * @return	kinetic parameters of the regulations of the specie
	 */
	inline const vector<Parameter> & getParameters(const SpecieID ID)  const {
		return species[ID].parameters;
	}

	/**
	 * @return	unique IDs of regulators of the specie
	 */
	set<SpecieID> getRegulators(const SpecieID ID) const {
		set<SpecieID> IDs;
		for (auto regul:species[ID].regulations) {
			IDs.insert(regul.source);
		}
		return IDs;
	}

	/**
	 * @return	true if the state is final
	 */
	inline bool isFinal(const size_t ID) const {
		return states[ID].final;
	}

	/**
	 * @return	edges of the state
	 */
	inline const vector<Egde> & getEdges(const SpecieID ID) const {
		return states[ID].edges;
	}
};

#endif // PARSYBONE_MODEL_INCLUDED
