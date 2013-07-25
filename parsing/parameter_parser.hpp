#ifndef PARAMETER_PARSER_HPP
#define PARAMETER_PARSER_HPP

#include "model.hpp"

class ParameterParser {
public:
   struct ParameterSpecifications {
      typedef pair<string,string> Parameter; ///< Context or logical condition and the specific target value attached to it.

      struct SpecieParameters {
         vector<Parameter> k_pars;
         vector<Parameter> l_pars;
      };

      vector<SpecieParameters> param_specs;
   };
   typedef vector<ParameterSpecifications::Parameter> ParsList;

private:

   vector<ParameterSpecifications::Parameter> parseParameters(const string tag, const string desc, const rapidxml::xml_node<> * const specie_node) const {
      vector<ParameterSpecifications::Parameter> parameters;

      // List through all the PARAM nodes.
      for (rapidxml::xml_node<> * parameter = XMLHelper::getChildNode(specie_node, tag.c_str(), false); parameter; parameter = parameter->next_sibling(tag.c_str()) ) {
         // Obtain context specified.
         string context;
         XMLHelper::getAttribute(context, parameter, desc.c_str());
         string value = "?";
         XMLHelper::getAttribute(value, parameter, "value", false);
         parameters.push_back(make_pair(context,value));
      }

      return parameters;
   }

public:
   ParameterSpecifications parse(const rapidxml::xml_node<> * const model_node) {
      ParameterSpecifications specifications;
      auto structure_node = XMLHelper::getChildNode(model_node, "STRUCTURE");

      // For each specie find explicit descriptions
      auto specie = XMLHelper::getChildNode(structure_node, "SPECIE");
      for (SpecieID ID = 0; specie; ID++, specie = specie->next_sibling("SPECIE") ) {
         // Create all contexts with all the possible values.
         auto k_pars = parseParameters("PARAM", "context", specie);
         auto l_pars = parseParameters("LOGIC", "expression", specie);

         specifications.param_specs.push_back({k_pars,l_pars});
      }

      return specifications;
   }
};

#endif // PARAMETER_PARSER_HPP
