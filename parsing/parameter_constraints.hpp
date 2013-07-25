#ifndef PARAMETER_CONSTRAINTS_HPP
#define PARAMETER_CONSTRAINTS_HPP

#include "../auxiliary/data_types.hpp"

class ParameterConstraints {
   friend class ParameterReader;

   /// A single kinetic parameter in explicit form - combinations of sources and possible targets are listed.
   struct Parameter {
      string context; ///< String name of the context of regulators.
      map<StateID, Levels> requirements; ///< Levels of the source species.
      Levels targets; ///< Basal targets (is no basal value is given, then all).
   };

   typedef vector<Parameter> SpecieParameters;

   vector<SpecieParameters> parameters;

public:
   inline const SpecieParameters & getParameters(const SpecieID ID)  const {
      return parameters[ID];
   }

   inline const Parameter & getParameter(const SpecieID ID, const size_t param_num) {
      return parameters[ID][param_num];
   }
};

#endif // PARAMETER_CONSTRAINTS_HPP
