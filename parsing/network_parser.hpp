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
#include "model.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Class for parsing of the regulatory network.
///
/// This object is responsible for parsing and translation of data related to the GRN.
/// Most of the possible semantics mistakes are under control and cause exceptions.
/// \attention Only models with up to 9 levels are supported.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class NetworkParser {
   Model & model; ///< Reference to the model object that will be filled.

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // TRANSLATORS:
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   /**
    * In a current regulation get source of that regulation, if possible.
    */
   SpecieID getSourceID(const rapidxml::xml_node<> * const regulation, const SpecieID t_ID ) const {
      string source; SpecieID source_ID;

      // Find the source and check correctness
      XMLHelper::getAttribute(source, regulation, "source");
      source_ID = model.findID(source);
      if (source_ID >= model.getSpeciesCount())
         throw invalid_argument("ID of a regulation of the specie " + toString(model.getName(t_ID)) + " is incorrect");

      return source_ID;
   }

   /**
    * Obtain a treshold of a current regulation and check if it is correct and unique.
    */
   size_t getThreshold(const rapidxml::xml_node<> * const regulation, const SpecieID t_ID, const SpecieID source_ID ) const {
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

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // PARSERS:
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   /**
    * Starting from the SPECIE node, the function parses all the REGUL tags and reads the data from them.
    * If not provided, attributes are defaulted - threshold to 1, label to Label::free
    */
   void parseRegulations(const rapidxml::xml_node<> * const specie_node, size_t specie_ID) const {
      // Regulation data
      string label;

      // Cycle through REGUL TAGS
      for (rapidxml::xml_node<>* regulation = XMLHelper::getChildNode(specie_node, "REGUL"); regulation; regulation = regulation->next_sibling("REGUL") ) {
         auto source_ID = getSourceID(regulation, specie_ID);
         auto threshold = getThreshold(regulation, specie_ID, source_ID);
         if (!XMLHelper::getAttribute(label, regulation, "label", false))
            label = Label::Free;

         // Add a new regulation to the specified target
         model.addRegulation(source_ID, specie_ID, threshold, label);
      }
   }

   /**
    * Starting from the STRUCTURE node, the function parses all the SPECIE tags and reads the data from them.
    * If not provided, attributes are defaulted - name is equal to ordinal number starting from 0, max to 1, targets to the whole range.
    */
   void firstParse(const rapidxml::xml_node<> * const structure_node) const {
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
         else if (name.length() < 2 || !isalpha(static_cast<int>(name[0])))
            throw invalid_argument("Name of the specie \"" + name + "\" is incorrect. Specie name can start only with a letter and must be at least 2 symbols in lenght.");
         // Get a max value and conver to integer.
         if (!XMLHelper::getAttribute(max, specie, "max", false))
            max = 1;

         if(XMLHelper::getAttribute(basal, specie, "basal", false)) {
            targets.push_back(basal);
            if (basal > max)
               throw invalid_argument("basal value is greater than maximal value for specie " + toString(ID));
         } else {
            targets = range(max + 1);
         }

         // Create a new specie
         model.addSpecie(name, max, targets);
      }
   }

   /**
    * Starting from the STRUCTURE node, the function parses all the SPECIE tags and reads the data from them.
    */
   void secondParse(const rapidxml::xml_node<> * const structure_node) const {
      // Step into first SPECIE tag, end when the current node does not have next sibling (all SPECIES tags were parsed)
      rapidxml::xml_node<> *specie = XMLHelper::getChildNode(structure_node, "SPECIE");
      for (SpecieID ID = 0; specie; ID++, specie = specie->next_sibling("SPECIE") ) {
         // Get all the regulations of the specie and store them to the model.
         parseRegulations(specie, ID);
      }
   }

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // CONSTRUCTION METHODS:
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   NetworkParser(const NetworkParser & other) = delete; ///< Forbidden copy constructor.
   NetworkParser& operator=(const NetworkParser & other) = delete; ///< Forbidden assignment operator.

public:
   NetworkParser(Model & _model) : model(_model) { } ///< Simple constructor, passes a references.

   /**
    * Main parsing function. It expects a pointer to inside of a MODEL node.
    */
   void parse(const rapidxml::xml_node<> * const model_node) {
      // Create the species.
      firstParse(XMLHelper::getChildNode(model_node, "STRUCTURE"));
      // Add regulatory logic.
      secondParse(XMLHelper::getChildNode(model_node, "STRUCTURE"));
   }
};

#endif // PARSYBONE_NETWORK_PARSER_INCLUDED
