#ifndef PROPERTY_PARSING_HPP
#define PROPERTY_PARSING_HPP

#include "xml_helper.hpp"
#include "../model/property_automaton.hpp"

namespace PropertyParsing {
   PropertyAutomaton::Constraints readConstraints(const rapidxml::xml_node<> * const node) {
      PropertyAutomaton::Constraints constraints;

      // Labelled transition to the next measurement
      XMLHelper::getAttribute(constraints.values, node, "values");
      constraints.values.erase(
               remove_if(constraints.values.begin(), constraints.values.end(), static_cast<int(*)(int)>(isspace)),
               constraints.values.end());

      // Resolve constraints
      XMLHelper::getAttribute(constraints.transient, node, "transient", false);
      XMLHelper::getAttribute(constraints.stable, node, "stable", false);
      if (constraints.transient && constraints.stable)
         throw runtime_error("An edge cannot be both transient and stable.");

      return constraints;
   }
}

#endif // PROPERTY_PARSING_HPP
