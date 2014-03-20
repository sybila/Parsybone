/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warranty. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see http://www.mi.fu-berlin.de/en/math/groups/dibimath and http://sybila.fi.muni.cz/ .
 */

#ifndef MODEL_HPP
#define MODEL_HPP

#include "../auxiliary/common_functions.hpp"
#include "../auxiliary/output_streamer.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Storage for data parsed from the model.
///
/// Model stores model data in the raw form, almost the same as in the model file itself.
/// Model data can be set only form the ModelParser object.
/// Rest of the code can access the data only via constant getters - once the data are parse, model remains constant.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Model {
public:
   NO_COPY(Model)

   /// Structure that holds additional static constraints inherent to the Thomas framework.
   struct Restrictions {
      bool bound_loop; ///< True if autoregilations are not created for values below the bound. (conserves all TSs).
      bool force_extremes; ///< True if for extemal contexts the parameter values are forced to be extremal.

      Restrictions() {
         bound_loop = force_extremes = false;
      }
   };

   struct Regulation {
      StateID source; ///< Regulator specie ID.
      ActLevel threshold; ///< Level of the regulator required for the regulation to be active.
      string name; ///< Name of the regulator.
      // Levels activity; ///< Source levels of the regulator.
      string label; ///< A behavioural constrain on this edge.
   };
   typedef vector<Regulation> Regulations;

   /// A single kinetic parameter in explicit form - combinations of sources and possible targets are listed.
   struct Parameter {
      string context; ///< String name of the context of regulators.
      map<StateID, Levels> requirements; ///< Levels of the source species.
      Levels targets; ///< Towards which level this context may regulate.
	  bool functional; ///< True iff the property currently employed allows for such a context to occur.

      Levels parameter_vals; ///< Levels towards which this function regulates based on current subcolor.
   };
   typedef vector<Parameter> Parameters;

   enum SpecType {
	   Input, Control, Component, Output
   };

   /// Structure that holds data about a single specie. Most of the data is equal to that in the model file.
   struct ModelSpecie {
      string name; ///< Actuall name of the specie.
      SpecieID ID; ///< Numerical constant used to distinguish the specie. Starts from 0!
      ActLevel max_value; ///< Maximal activation level of the specie.
	  SpecType spec_type; ///< What is the type of this specie.
	  ParamNo step_size; ///< How many neighbour parameters have the same value for this function.
	  vector<string> par_cons;  ///< Constraints on the parameters.

      Regulations regulations; ///< Regulations of the specie (activations or inhibitions by other species).
      Parameters parameters; /// Kintetic parameters for the specie (or at least their partiall specifiaction).
      Configurations subcolors; ///< Feasible subcolors of the specie. THIS INFORMATION GETS REPLICATED TO PARAMETER_VALS - POSSIBLE PLACE OF SIMPLIFICATION
   };

   Restrictions restrictions;
   vector<ModelSpecie> species; ///< vector of all species of the model

   inline void addSpecie(const string & name, const size_t max, const Model::SpecType type) {
	   species.push_back({ name, species.size(), max, type, 0, vector<string>(), Model::Regulations(), Model::Parameters(), Configurations() });
   }

   inline void addRegulation(SpecieID source_ID, SpecieID target_ID, ActLevel threshold, string label) {
      string name = species[source_ID].name + ":" + to_string(threshold);
      species[target_ID].regulations.push_back({source_ID, threshold, move(name), label});
   }

   inline void addParameter(const SpecieID ID, string context, map<StateID, Levels> requirements, Levels targets) {
      species[ID].parameters.push_back({context, requirements, targets, true, Levels()});
   }

   inline ParamNo getStepSize(const SpecieID ID) const {
      if (species[ID].parameters.empty())
         throw runtime_error("Trying to obtain step size before parametrizations were built");
      return species[ID].step_size;
   }

   inline const vector<Parameter> & getParameters(const SpecieID ID)  const {
      return species[ID].parameters;
   }

};

#endif // MODEL_HPP
