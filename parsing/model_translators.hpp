#ifndef MODEL_TRANSLATORS_HPP
#define MODEL_TRANSLATORS_HPP

#include "model.hpp"

namespace ModelTranslators {
   /**
    * @brief findID  obtain ID of the specie.
    */
   SpecieID findID(const Model & model, const string & name) {
      for(auto & spec:model.species)
         if (spec.name.compare(name) == 0)
            return spec.ID;
      return INF;
   }

   /**
    * @return	unique IDs of regulators of the specie
    */
   vector<SpecieID> getRegulatorsIDs(const Model & model, const SpecieID ID) {
      set<SpecieID> IDs;
      for (auto regul:model.species[ID].regulations) {
         IDs.insert(regul.source);
      }
      return vector<SpecieID>(IDs.begin(), IDs.end());
   }

   /**
    * @return	names of the regulators of the specie
    */
   vector<string> getRegulatorsNames(const Model & model, const SpecieID ID) {
      auto regulators = getRegulatorsIDs(model, ID);
      vector<string> names;
      for (auto reg:regulators) {
         names.push_back(model.getName(reg));
      }
      return names;
   }

   /**
    * @brief getThresholds Finds a list of thresholds for each regulator of a given component.
    * @param ID
    * @return
    */
   map<SpecieID, vector<ActLevel> > getThresholds(const Model & model, const SpecieID ID) {
      map<SpecieID, Levels > thresholds;
      for (auto reg:model.getRegulations(ID)) {
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
    * @brief getSpaceSize
    */
   ParamNum getSpaceSize(const Model & model) {
      ParamNum space_size = 1;
      for (const Model::ModelSpecie & spec:model.species) {
         space_size *= spec.subcolors.size();
      }
      return space_size;
   }

   /**
    * @brief getSpecieVals
    */
   const vector<ParamNum> getSpecieVals(const Model & model, ParamNum number) {
      // Prepare storage vector
      vector<ParamNum> specie_vals(model.species.size());
      auto reverse_val_it = specie_vals.rbegin();

      // Go through colors backwards
      ParamNum divisor = getSpaceSize(model);
      for (auto specie_it = model.species.rbegin(); specie_it != model.species.rend(); specie_it++, reverse_val_it++) {
         // lower divisor value
         divisor /= specie_it->subcolors.size();
         // pick a number for current specie
         *reverse_val_it = (number / divisor);
         // take the rest for next round
         number = number % divisor;
      }

      return specie_vals;
   }

   /**
    * This function returns a vector containing target value for a given regulatory contexts for ALL the contexts allowed (in lexicographical order).
    * @param ID	ID of the specie that is regulated
    * @param param_num	ordinal number of the kinetic parameter (in a lexicographical order)
    * @return	vector with a target value for a given specie and regulatory context for each subcolor (parametrization of the single specie)
    */
   const vector<size_t> getTargetVals(const Model & model, const SpecieID ID, const size_t param_num) {
      //Data to fill
      vector<size_t> all_target_vals;
      all_target_vals.reserve(model.species[ID].subcolors.size());

      // Store values for given regulation
      for (const auto & subcolor : model.species[ID].subcolors) {
         all_target_vals.push_back(subcolor[param_num]);
      }

      return all_target_vals;
   }

   /**
    * @brief createColorString
    */
   const string createColorString(const Model & model, ParamNum number) {
      // compute numbers of partial parametrizations for each component
      const vector<ParamNum> color_parts = getSpecieVals(model, number);

      string color_str = "(";
      // cycle through the species
      for (const SpecieID ID : scope(model.species)) {
         auto color = model.species[ID].subcolors[color_parts[ID]];
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
}

#endif // MODEL_TRANSLATORS_HPP
