/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_MODEL_INCLUDED
#define PARSYBONE_MODEL_INCLUDED

#include "../auxiliary/common_functions.hpp"
#include "../auxiliary/output_streamer.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Storage for data parsed from the model.
///
/// Model stores model data in the raw form, almost the same as in the model file itself.
/// Model data can be set only form the ModelParser object.
/// Rest of the code can access the data only via constant getters - once the data are parse, model remains constant.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Model {
   friend class Tester;
   friend class AutomatonParser;
   friend class ModelParser;
   friend class NetworkParser;
   friend class TimeSeriesParser;

public:
   /// Structure that stores regulation of a specie by another one
   struct Regulation {
      StateID source; ///< Regulator specie ID.
      ActLevel threshold; ///< Level of the regulator required for the regulation to be active.
      string name;
      Levels activity; ///<
      string label; ///< A behavioural constrain on this edge.
   };
   typedef vector<Regulation> Regulations;

   struct Parameter {
      string context;
      map<StateID, Levels> requirements;
      Levels targets;
   };
   typedef vector<Parameter> Parameters;

   typedef pair<StateID, string> Egde; ///< Edge in Buchi Automaton (Target ID, edge label).
   typedef vector<Egde> Edges;

private:
   /// Structure that holds data about a single specie. Most of the data is equal to that in the model file
   struct ModelSpecie {
      string name; ///< Actuall name of the specie
      SpecieID ID; ///< Numerical constant used to distinguish the specie. Starts from 0!
      ActLevel max_value; ///< Maximal activation level of the specie
      Levels targets;
      Levels range;

      Regulations regulations; ///< Regulations of the specie (activations or inhibitions by other species)
      Parameters parameters;
   };

   /// Structure that holds data about a single state.
   struct BuchiAutomatonState {
      string name; ///< Label of the state.
      SpecieID ID; ///< Numerical constant used to distinguish the state. Starts from 0!
      bool final; ///< True if the state is final.

      vector<Egde> edges; ///< Edges in Buchi Automaton (Target ID, edge label).
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
   inline size_t addSpecie(string name, size_t max_value, Levels targets) {
      species.push_back({name, species.size(), max_value, targets, range(max_value + 1), Regulations(), Parameters()});
      return species.size() - 1;
   }

   /**
    * Add a new regulation to the specie. Regulation is stored with the target, not the source.
    */
   inline void addRegulation(SpecieID source_ID, SpecieID target_ID, size_t threshold, string label) {
      string name = species[source_ID].name + ":" + toString(threshold);
      species[target_ID].regulations.push_back({source_ID, threshold, move(name), Levels(), label});
   }

   inline void addParameters(const SpecieID target_ID, const vector<Parameter> & parameters) {
      species[target_ID].parameters = parameters;
   }

   /**
    *
    */
   inline void addActivityLevels(const SpecieID source, const SpecieID target, const Levels & levels) {
      if (levels.empty())
         throw runtime_error("Trying to assign empty levels to the regulation of " + toString(target) + " from the specie " + toString(source));

      for (auto & reg:species[target].regulations)
         if (reg.source == source && reg.threshold == levels[0])
            reg.activity = levels;
   }

   /**
    * Add a new state to the automaton.
    *
    * @return	ID of state in the vector
    */
   inline size_t addState(string name, bool final) {
      states.push_back({name, states.size(), final, Edges()});
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

   Model(const Model & other) = delete; ///< Forbidden copy constructor.
   Model& operator=(const Model & other) = delete; ///< Forbidden assignment operator.

public:
   Model() = default; ///< Default empty constructor.

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

   inline Levels getTargets(const SpecieID ID) const {
      return species[ID].targets;
   }

   inline Levels getRange(const SpecieID ID) const {
      return species[ID].range;
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
   vector<SpecieID> getRegulatorsIDs(const SpecieID ID) const {
      set<SpecieID> IDs;
      for (auto regul:species[ID].regulations) {
         IDs.insert(regul.source);
      }
      return vector<SpecieID>(IDs.begin(), IDs.end());
   }

   /**
    * @return	names of the regulators of the specie
    */
   vector<string> getRegulatorsNames(const SpecieID ID) const {
      auto regulators = getRegulatorsIDs(ID);
      vector<string> names;
      for (auto reg:regulators) {
         names.push_back(getName(reg));
      }
      return names;
   }

   map<SpecieID, vector<ActLevel> > getThresholds(const SpecieID ID) const {
      map<SpecieID, Levels > thresholds;
      for (auto reg:getRegulations(ID)) {
         auto key = thresholds.find(reg.source);
         if (key == thresholds.end()) {
            thresholds.insert(make_pair(reg.source, Levels(1, reg.threshold)));
         } else {
            key->second.push_back(reg.threshold);
         }
      }

      for (auto ths:thresholds) {
         sort(ths.second.begin(), ths.second.end());
      }

      return thresholds;
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
