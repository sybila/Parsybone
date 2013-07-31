/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see http://www.mi.fu-berlin.de/en/math/groups/dibimath and http://sybila.fi.muni.cz/ .
 */

#ifndef MODEL_HPP
#define MODEL_HPP

#include "PunyHeaders/common_functions.hpp"
#include "../auxiliary/output_streamer.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Storage for data parsed from the model.
///
/// Model stores model data in the raw form, almost the same as in the model file itself.
/// Model data can be set only form the ModelParser object.
/// Rest of the code can access the data only via constant getters - once the data are parse, model remains constant.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Model {
public:
   /// Structure that holds additional static constraints inherent to the Thomas framework.
   struct Restrictions {
      bool bounded_loops; ///< True if autoregilations are not created for values below the bound. (conserves all TSs).
      bool force_extremes; ///< True if for extemal contexts the parameter values are forced to be extremal.

      Restrictions() {
         bounded_loops = force_extremes = false;
      }
   } restrictions;

   /// Structure that stores regulation of a specie by another one
   struct Satisfaction {
      bool none;
      bool activ;
      bool inhib;
      bool both;
   };
   struct Regulation {
      StateID source; ///< Regulator specie ID.
      ActLevel threshold; ///< Level of the regulator required for the regulation to be active.
      string name; ///< Name of the regulator.
      Levels activity; ///< Source levels of the regulator.
      string label; ///< A behavioural constrain on this edge.
      Satisfaction satisf; ///< What constraints must be satisfied by parameterization enforced by this edge.
   };
   typedef vector<Regulation> Regulations;

   /// A single kinetic parameter in explicit form - combinations of sources and possible targets are listed.
   struct Parameter {
      string context; ///< String name of the context of regulators.
      map<StateID, Levels> requirements; ///< Levels of the source species.
      Levels targets; ///< Towards which level this context may regulate.
   };
   typedef vector<Parameter> Parameters;

   struct SpecTraits {
      bool input;
      bool output;
   };
   /// Structure that holds data about a single specie. Most of the data is equal to that in the model file.
   struct ModelSpecie {
      string name; ///< Actuall name of the specie.
      SpecieID ID; ///< Numerical constant used to distinguish the specie. Starts from 0!
      ActLevel max_value; ///< Maximal activation level of the specie.
      Levels basals; ///< Basal targets (is no basal value is given, then all).
      SpecTraits traits; ///< Description of the specie in the network.


      Regulations regulations; ///< Regulations of the specie (activations or inhibitions by other species).
      Parameters parameters; /// Kintetic parameters for the specie (or at least their partiall specifiaction).
      Configurations subcolors; ///< Feasible subcolors of the specie.
   };

   vector<ModelSpecie> species; ///< vector of all species of the model

   /**
    * Create a new specie with the provided name, basal and maximal value.
    *
    * @return	index of specie in the vector
    */
   inline size_t addSpecie(string name, size_t max_value, Levels targets, bool input = false, bool output = false) {
      species.push_back({name, species.size(), max_value, targets, SpecTraits({input, output}), Regulations(), Parameters(), Configurations()});
      return species.size() - 1;
   }

   /**
    * Add a new regulation to the specie. Regulation is stored with the target, not the source.
    */
   inline void addRegulation(SpecieID source_ID, SpecieID target_ID, size_t threshold, string label) {
      string name = species[source_ID].name + ":" + toString(threshold);
      species[target_ID].regulations.push_back({source_ID, threshold, move(name), Levels(), label, Satisfaction()});
   }

   inline void setParameters(const SpecieID target_ID, const vector<Parameter> & parameters) {
      species[target_ID].parameters = parameters;
   }

   /**
    * For a regulation add levels where it is
    */
   inline void addActivityLevels(const SpecieID source, const SpecieID target, const Levels & levels) {
      if (levels.empty())
         throw runtime_error("Trying to assign empty levels to the regulation of " + toString(target) + " from the specie " + toString(source));

      for (auto & reg:species[target].regulations)
         if (reg.source == source && reg.threshold == levels[0])
            reg.activity = levels;
   }

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
    * @brief getBasalTargets Values towards which the specie is being regulated by default. Used in case of specification of basal values.
    * @param ID
    * @return
    */
   inline Levels getBasalTargets(const SpecieID ID) const {
      return species[ID].basals;
   }


   inline const vector<Regulation> & getRegulations(const SpecieID ID) const {
      return species[ID].regulations;
   }

   inline const vector<Parameter> & getParameters(const SpecieID ID)  const {
      return species[ID].parameters;
   }

   inline const Configurations & getSubcolors(const SpecieID ID)  const {
      return species[ID].subcolors;
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

   /**
    * @brief getThresholds Finds a list of thresholds for each regulator of a given component.
    * @param ID
    * @return
    */
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
    * This function returns a vector containing target value for a given regulatory contexts for ALL the contexts allowed (in lexicographical order).
    * @param ID	ID of the specie that is regulated
    * @param param_num	ordinal number of the kinetic parameter (in a lexicographical order)
    * @return	vector with a target value for a given specie and regulatory context for each subcolor (parametrization of the single specie)
    */
   const vector<size_t> getTargetVals(const SpecieID ID, const size_t param_num) const {
      //Data to fill
      vector<size_t> all_target_vals;
      all_target_vals.reserve(species[ID].subcolors.size());

      // Store values for given regulation
      for (const auto & subcolor : species[ID].subcolors) {
         all_target_vals.push_back(subcolor[param_num]);
      }

      return all_target_vals;
   }

   inline const vector<size_t> & getColor(const SpecieID ID, const ParamNum color_num) const {
      return species[ID].subcolors[static_cast<size_t>(color_num)];
   }

   const string createColorString(ParamNum number) const {
      // compute numbers of partial parametrizations for each component
      const vector<ParamNum> color_parts = getSpecieVals(number);

      string color_str = "(";
      // cycle through the species
      for (SpecieID ID = 0; ID < species.size(); ID++) {
         auto color = getColor(ID, color_parts[ID]);
         // fill partial parametrization of the specie
         for (auto it = color.begin(); it != color.end(); it++) {
            color_str += lexical_cast<string, size_t>(*it);
            color_str += ",";
         }
      }
      // Change the last value
      *(color_str.end() - 1) = ')';

      return color_str;
   }

   const vector<ParamNum> getSpecieVals(ParamNum number) const {
      // Prepare storage vector
      vector<ParamNum> specie_vals(species.size());
      auto reverse_val_it = specie_vals.rbegin();

      // Go through colors backwards
      ParamNum divisor = getSpaceSize();
      for (auto specie_it = species.rbegin(); specie_it != species.rend(); specie_it++, reverse_val_it++) {
         // lower divisor value
         divisor /= specie_it->subcolors.size();
         // pick a number for current specie
         *reverse_val_it = (number / divisor);
         // take the rest for next round
         number = number % divisor;
      }

      return specie_vals;
   }

   ParamNum getSpaceSize() const {
      ParamNum space_size = 1;
      for (const ModelSpecie & spec:species) {
         space_size *= spec.subcolors.size();
      }
      return space_size;
   }
};

#endif // MODEL_HPP
