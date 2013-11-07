/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_NETWORK_PARSER_INCLUDED
#define PARSYBONE_NETWORK_PARSER_INCLUDED

#include "PunyHeaders/formulae_resolver.hpp"

#include "../auxiliary/data_types.hpp"
#include "xml_helper.hpp"
#include "../model/model_translators.hpp"
#include "parsing_commons.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Class for parsing of the regulatory network.
///
/// This object is responsible for parsing and translation of data related to the GRN.
/// Most of the possible semantics mistakes are under control and cause exceptions.
/// \attention Only models with up to 9 levels are supported.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class NetworkParser {
   /**
    * In a current regulation get source of that regulation, if possible.
    */
   static SpecieID getSourceID(const rapidxml::xml_node<> * const regulation, const SpecieID t_ID, Model & model ) {
      string source; SpecieID source_ID;

      // Find the source and check correctness
      XMLHelper::getAttribute(source, regulation, "source");
      source_ID = ModelTranslators::findID(model, source);
      if (source_ID >= model.species.size())
         throw invalid_argument("ID of a regulation of the specie " + toString(model.getName(t_ID)) + " is incorrect");

      return source_ID;
   }

   /**
    * Obtain a treshold of a current regulation and check if it is correct and unique.
    */
   static size_t getThreshold(const rapidxml::xml_node<> * const regulation, const SpecieID t_ID, const SpecieID source_ID, Model & model ) {
      size_t threshold;

      // Try to find a threshold, if not present, set to 1
      if(!XMLHelper::getAttribute(threshold, regulation, "threshold", false))
         threshold = 1;
      else if (threshold > model.getMax(source_ID) || threshold == 0) // Control the value
         throw invalid_argument("the threshold' value " + toString(threshold) + " is not within the range of the regulator " + toString(model.getName(source_ID)));

      // Test uniqueness of this combination (source, threshold)
      auto regulations = model.getRegulations(t_ID);
      for(const auto & regul:regulations) {
         if (threshold == regul.threshold && source_ID == regul.source)
            throw invalid_argument("multiple definition of a regulation of a specie " + toString(model.getName(source_ID)));
      }

      return threshold;
   }

   /**
    * Starting from the SPECIE node, the function parses all the REGUL tags and reads the data from them.
    * If not provided, attributes are defaulted - threshold to 1, label to Label::free
    */
   static void parseRegulations(const rapidxml::xml_node<> * const specie_node, SpecieID t_ID, Model & model) {
      // Regulation data
      string label;

      // Cycle through REGUL TAGS
      for (auto regulation : XMLHelper::NodesRange(specie_node, "REGUL", true)) {
         auto s_ID = getSourceID(regulation, t_ID, model);
         auto threshold = getThreshold(regulation, t_ID, s_ID, model);
         if (!XMLHelper::getAttribute(label, regulation, "label", false))
            label = Label::Free;

         // Add a new regulation to the specified target
         model.addRegulation(s_ID, t_ID, threshold, label);
      }
   }

   /**
    * Starting from the STRUCTURE node, the function parses all the SPECIE tags and reads the data from them.
    * If not provided, attributes are defaulted -
    * name is equal to ordinal number starting from 0,
    * max to 1,
    * targets to the range [0,max]
    * input and output to false
    */
   static void firstParse(const rapidxml::xml_node<> * const structure_node, Model & model) {
      // Start the naming from capital A.
      char specie_name = 'A';
      // Specie data
      string name; size_t max; size_t basal = 0; Levels targets;

      // Step into first SPECIE tag, end when the current node does not have next sibling (all SPECIES tags were parsed)
      rapidxml::xml_node<> *specie = XMLHelper::getChildNode(structure_node, "SPECIE");
      for (SpecieID ID = 0; specie; ID++, specie = specie->next_sibling("SPECIE"), specie_name++) {
         // Get a name of the specie.
         if (!XMLHelper::getAttribute(name, specie, "name", false))
            name = toString(specie_name);
         // Throw an error if the name is not correct.
         else if (!ParsingCommons::isValidSpecName(name))
            ParsingCommons::specNameExc(name);

         // Get a max value and conver to integer.
         if (!XMLHelper::getAttribute(max, specie, "max", false))
            max = 1;

         // Obtain basal values
         if(XMLHelper::getAttribute(basal, specie, "basal", false)) {
            targets.push_back(basal);
            if (basal > max)
               throw invalid_argument("basal value is greater than maximal value for specie " + toString(ID));
         } else {
            targets = range<ActLevel>(max + 1);
         }

         // Check if the node is either input or output node.
         bool input = false, output = false;
         string node_type;
         if(XMLHelper::getAttribute(node_type, specie, "type", false)) {
            if (node_type.compare("input") == 0)
               input = true;
            else if (node_type.compare("output") == 0) {
               output = true;
            }
         }

         // Create a new specie
         model.addSpecie(name, max, targets, input, output);
      }
   }

   /**
    * Starting from the STRUCTURE node, the function parses all the SPECIE tags and reads the data from them.
    */
   static void secondParse(const rapidxml::xml_node<> * const structure_node, Model & model) {
      // Step into first SPECIE tag, end when the current node does not have next sibling (all SPECIES tags were parsed)
      rapidxml::xml_node<> *specie = XMLHelper::getChildNode(structure_node, "SPECIE");
      for (SpecieID ID = 0; specie; ID++, specie = specie->next_sibling("SPECIE") ) {
         // Get all the regulations of the specie and store them to the model.
         parseRegulations(specie, ID, model);
      }
   }

public:
   /**
    * Main parsing function. It expects a pointer to inside of a MODEL node.
    */
   static void parseNetwork(const rapidxml::xml_node<> * const network_node, Model & model) {
      // Create the species.
      firstParse(network_node, model);
      // Add regulatory logic.
      secondParse(network_node, model);
   }

   /**
    * @brief parseConstraints   Parses the constraints given by the user.
    */
   static void parseConstraints(const rapidxml::xml_node<> * const network_node, Model & model) {
       for (auto constraint : XMLHelper::NodesRange(network_node, "CONSTRAINT", false)) {
           string const_type;
           XMLHelper::getAttribute(const_type, constraint, "type");
           if (const_type.compare("bound_loop")  == 0) {
               model.restrictions.bound_loop = true;
           } else if (const_type.compare("force_extremes") == 0) {
               model.restrictions.force_extremes = true;
           } else {
               throw runtime_error("Constraint \"" + const_type + "\" is not a valid constraint.");
           }
       }
   }
};

#endif // PARSYBONE_NETWORK_PARSER_INCLUDED
