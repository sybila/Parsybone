#ifndef PARAMETER_PARSER_HPP
#define PARAMETER_PARSER_HPP

#include "../model/model_translators.hpp"
#include "../parsing/xml_helper.hpp"

class ParameterParser {
private:
   /**
    * @brief parseParameters Reads the constraints.
    */
   static vector<Model::ParamSpec> parseParameters(const string tag, const string desc, const rapidxml::xml_node<> * const specie_node) {
      vector<Model::ParamSpec> parameters;

      // List through all the PARAM nodes.
      for (auto parameter : XMLHelper::NodesRange(specie_node, tag.c_str(), false)) {
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
   /**
    * @brief parse   Reads explicit parameter specification from the model file.
    */
   static void parse(const rapidxml::xml_node<> * const network_node, Model & model) {
      // For each specie find explicit descriptions
      auto specie = XMLHelper::getChildNode(network_node, "SPECIE");
      for (SpecieID ID = 0; specie; ID++, specie = specie->next_sibling("SPECIE") ) {
         // Create all contexts with all the possible values.
         auto k_pars = parseParameters("PARAM", "context", specie);
         auto l_pars = parseParameters("LOGIC", "expression", specie);

         model.species[ID].params_specs = {k_pars,l_pars};
      }
   }
};

#endif // PARAMETER_PARSER_HPP
