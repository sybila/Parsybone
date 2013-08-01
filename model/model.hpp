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
   };

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

      ParamNum step_size; ///< How many neighbour parameters have the same value for this function.
      Levels possible_values; ///< Levels towards which this function regulates based on current subcolor.
   };
   typedef vector<Parameter> Parameters;

   typedef pair<string,string> ParamSpec;
   struct ParamSpecs {
      vector<ParamSpec> k_pars;
      vector<ParamSpec> l_pars;
   };
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
      ParamSpecs params_specs; ///< Constraints on parameters.

      Regulations regulations; ///< Regulations of the specie (activations or inhibitions by other species).
      Parameters parameters; /// Kintetic parameters for the specie (or at least their partiall specifiaction).
      Configurations subcolors; ///< Feasible subcolors of the specie.

   };

   Restrictions restrictions;
   vector<ModelSpecie> species; ///< vector of all species of the model

   inline void addSpecie(string name, size_t max_value, Levels targets, bool input = false, bool output = false) {
      species.push_back({name, species.size(), max_value, targets, SpecTraits({input, output}), ParamSpecs(), Regulations(), Parameters(), Configurations()});
   }

   inline void addRegulation(SpecieID source_ID, SpecieID target_ID, size_t threshold, string label) {
      string name = species[source_ID].name + ":" + toString(threshold);
      species[target_ID].regulations.push_back({source_ID, threshold, move(name), Levels(), label, Satisfaction()});
   }

   inline void addParameter(const SpecieID ID, string context, map<StateID, Levels> requirements, Levels targets) {
      species[ID].parameters.push_back({context, requirements, targets, 0, Levels()});
   }

   inline void addKineticFunction(const size_t target_ID, const size_t param_ID, const size_t step_size, const Levels & possible_values) {
      species[target_ID].parameters[param_ID].step_size = step_size;
      species[target_ID].parameters[param_ID].possible_values = possible_values;
   }

   inline const string & getName(const SpecieID ID) const {
      return species[ID].name;
   }

   inline size_t getMin(const SpecieID ID) const {
      return ID ? 0 : 0; // Just to disable a warning
   }

   inline size_t getMax(const SpecieID ID) const {
      return species[ID].max_value;
   }

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
};

#endif // MODEL_HPP
