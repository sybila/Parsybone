/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

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
   ParamNo getSpaceSize(const Model & model) {
      ParamNo space_size = 1;
      for (const Model::ModelSpecie & spec:model.species) {
         space_size *= spec.subcolors.size();
      }
      return space_size;
   }

   /**
    * @brief getSpecieVals
    */
   const vector<ParamNo> getSpecieVals(const Model & model, ParamNo number) {
      // Prepare storage vector
      vector<ParamNo> specie_vals(model.species.size());
      auto reverse_val_it = specie_vals.rbegin();

      // Go through colors backwards
      ParamNo divisor = getSpaceSize(model);
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
    * @brief createColorString
    */
   const string createParamString(const Model & model, ParamNo number) {
      // compute numbers of partial parametrizations for each component
      const vector<ParamNo> color_parts = getSpecieVals(model, number);

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

   /**
    * @return representation of the parametrization used by the database
    */
   const string makeConcise(const Model::Parameter & param, const string target_name) {
      string context = "K_" + target_name + "_";
      for (auto values:param.requirements)
         context += toString(values.second.front());
      return context;
   }

   /**
    * @return numbers of all parametrizations that match the one given on input
    */
   set<ParamNo> findMatching(const Model & model, const Levels & param_vals) {
      set<ParamNo> matching;
      size_t begin = 0; //< Used to denote current range of the parametrization

      // Test subparametrizations for all species.
      for (const SpecieID ID : scope(model.species)) {
         vector<ParamNo> submatch;

         // Try to match all the subcolors fo the current specie
         for (const size_t subolor_no : scope(model.species[ID].subcolors)) {
            bool valid = true;
            // For the match to occur, all values must either be equal or defined irellevant.
            for (const size_t value_no: scope(model.species[ID].subcolors[subolor_no])) {
               if (param_vals[value_no + begin] != static_cast<ActLevel>(INF) && param_vals[value_no + begin] != model.species[ID].subcolors[subolor_no][value_no]) {
                  valid = false;
                  break;
               }
            }
            if (valid)
               submatch.push_back(subolor_no * model.getStepSize(ID));
         }

         // At least one subparametrization must be found for each specie, if not end.
         if (submatch.empty())
            return set<ParamNo>();

         // Create the results as all possible combinations.
         set<ParamNo> old = matching.empty() ? set<ParamNo>({0}) : move(matching);
         for (const ParamNo o_match : old) {
            for (const ParamNo m_match : submatch) {
               matching.insert(o_match + m_match);
            }
         }

         // Move the beginning for the next specie
         begin += model.species[ID].parameters.size();
      }
      return matching;
   }
}

#endif // MODEL_TRANSLATORS_HPP
