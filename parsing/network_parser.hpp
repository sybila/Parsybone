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
    * @brief fillActivationLevels For each regulation fill the levels of its source in which it is active.
    */
   void fillActivationLevels() const {
      // Fill for all the species.
      for (auto ID:range(model.getSpeciesCount())) {
         auto space = model.getThresholds(ID);

         // List through the regulations of the specie.
         for (auto regul:model.getRegulations((ID))) {
            // Start at the lower threshold
            ActLevel begin = regul.threshold;
            // See if there is an upper threshold
            auto thresholds = space.find(regul.source)->second;
            sort(thresholds.begin(), thresholds.end(), [](unsigned int a, unsigned int b){return a <= b;});
            auto th_it = find(thresholds.begin(), thresholds.end(), begin) + 1;
            // Create the maximum based on whethter this is the last threshold or not.
            ActLevel end = (th_it == thresholds.end()) ? model.getMax(regul.source) + 1 : *th_it;

            model.addActivityLevels(regul.source, ID, range(begin, end));
         }
      }
   }

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
    * Searches for the LOGIC tag and if such is present, uses it for creation of parameters for the specie.
    *
    * @return true if the LOGIC was found and used
    */
   string parseLogic(const rapidxml::xml_node<> * const specie_node, size_t specie_ID) const {
      // Try to get the tag
      rapidxml::xml_node<>* logic = XMLHelper::getChildNode(specie_node, "LOGIC", false);

      // If the tag is present, use it
      if (logic != 0) {
         if (logic->next_sibling("LOGIC"))
            throw invalid_argument("LOGIC tag is present multiple times in the definition of the specie " + toString(model.getName(specie_ID)));

         // Get and apply the formula
         string formula;
         XMLHelper::getAttribute(formula, logic, "formula");

         return formula;
      }
      else
         return "";
   }

   /**
    * @brief getThreshold  For a given regulator, find out what it's threshold in the given context is.
    * @param context
    * @param target_ID
    * @param name
    * @param pos
    * @return
    */
   size_t getThreshold(const string & context, const SpecieID t_ID, const string & name, const size_t pos) const {
      // Regulator not present.
      if (pos == context.npos)
         return 0;
      const size_t COLON_POS = pos + name.length(); // Where colon is supposed to be.

      // Regulator level not specified.
      if (context[COLON_POS] != ':') {
         // Control if the context is unambiguous.
         auto thresholds = model.getThresholds(t_ID);
         if (thresholds.find(model.findID(name))->second.size() > 1)
            throw runtime_error ("Ambiguous context \"" + context + "\" - no threshold specified for a regulator " + name + " that has multiple regulations.");
         // If valid, add the threshold 1.
         return thresholds.find(model.findID(name))->second[0];
      }

      // There is not a threshold given after double colon.
      if (context[COLON_POS] == ':' && (COLON_POS == (context.npos - 1) || !isdigit(context[COLON_POS+1])))
         throw runtime_error ("No threshold given after colon in the context \"" + context + "\" of the regulator " + name);

      // Add a threshold if uniquely specified.
      string to_return;
      size_t number = 1;
      // Copy all the numbers
      while(isdigit(context[COLON_POS + number])) {
         to_return.push_back(context[COLON_POS + number]);
         number++;
      }
      return (boost::lexical_cast<size_t>(to_return));
   }

   /**
    * @brief formCanonicContext   Transforms the regulation specification into a canonic form (\forall r \in regulator [r:threshold,...]).
    * @param context
    * @param target_ID
    * @return canonic context form
    */
   string formCanonicContext(const string & context, const SpecieID t_ID) const {
      string new_context; // new canonic form
      const auto names = model.getRegulatorsNames(t_ID);

      // For each of the regulator of the specie.
      for (const auto & name:names) {
         auto pos = context.find(name);
         size_t threshold = getThreshold(context, t_ID, name, pos);
         new_context += name + ":" + toString(threshold) + ",";
      }

      // Remove the last comma and return
      return new_context.substr(0, new_context.length() - 1);
   }

   /**
    * @brief covertToLevels   Take a string of the form (\d,)*\d and transform it into a list of values.
    * @param val_str
    * @return
    */
   Levels covertToLevels(const string & val_str, const SpecieID t_ID) const {
      vector<string> numbers;
      Levels specified;
      split(numbers, val_str, is_any_of(","));

      // Convert the string into list of numbers.
      for (const auto & num:numbers) {
         size_t val;

         // Convert one number.
         try {
            val = lexical_cast<size_t>(num);
         } catch (bad_lexical_cast) {
            throw runtime_error("Specified value " + num + " in the list " + val_str + " is not a number");
         }

         if (val < model.getMin(t_ID) || val > model.getMax(t_ID))
            throw invalid_argument("target value " + val_str + " out of range for specie " + model.getName(t_ID));

         specified.push_back(val);
      }

      return specified;
   }

   /**
    * @brief interpretLevels  Reads the definition of target values and interprets them into Levels structure.
    * @return
    */
   Levels interpretLevels(rapidxml::xml_node<> * parameter, const SpecieID t_ID) const {
      string val_str = "";

      // Get the target value (the whole range if unspecified) and check it
      if (XMLHelper::getAttribute(val_str, parameter, "value", false))
         // ? goes for unspecified.
         if (val_str.compare("?") == 0)
            return model.getBasalTargets(t_ID);
         else
            return covertToLevels(val_str, t_ID);

      // If none is given, use the whole range.
      else
         return model.getBasalTargets(t_ID);
   }

   /**
    * @brief replaceInContext
    * @param parameters
    * @param in_context
    * @param can_context
    * @param targets
    */
   void replaceInContext(Model::Parameters & parameters, const string & in_context, const string & can_context, const Levels & targets) const {
      // List through all parameters of the specie.
      for(auto & param:parameters) {
         // If the context is matched.
         if (param.context.compare(can_context) == 0) {
            param.targets = targets;
            return;
         }
      }

      // If the context was not ever matched.
      throw runtime_error("Given context " + in_context + " not mached, probably incorrect.");
   }

   /**
    * @brief replaceExplicit  This function replaces already stored parameters by new ones, if asked in the model.
    * @param parameters
    * @param specie_node
    * @param target_ID
    */
   void replaceExplicit(Model::Parameters & parameters, const rapidxml::xml_node<> * const specie_node, const SpecieID t_ID) const {
      // List through all the PARAM nodes.
      for (rapidxml::xml_node<> * parameter = XMLHelper::getChildNode(specie_node, "PARAM", false); parameter; parameter = parameter->next_sibling("PARAM") ) {
         // Obtain context specified.
         string in_context = "";
         XMLHelper::getAttribute(in_context, parameter, "context");
         string can_context = formCanonicContext(in_context, t_ID);

         // Get the levels.
         Levels targets = interpretLevels(parameter, t_ID);

         // Find the context and replace it's target values.
         replaceInContext(parameters, in_context, can_context, targets);
      }
   }

   /**
    * @brief getSingleParam
    * @param all_thrs
    * @param thrs_comb
    * @param target_ID
    * @param formula
    * @return
    */
   Model::Parameter getSingleParam(const map<SpecieID, Levels> & all_thrs, const Levels thrs_comb, const SpecieID t_ID) const {
      // Empty data to fill.
      Model::Parameter parameter = {"", map<StateID, Levels>(), Levels()};

      // Loop over all the sources.
      for (auto source_num:range(thrs_comb.size())) {
         // Find the source details and its current threshold
         string source_name = model.getRegulatorsNames(t_ID)[source_num];
         StateID source_ID = model.getRegulatorsIDs(t_ID)[source_num];
         auto thresholds = all_thrs.find(source_ID)->second;

         // Find activity level of the current threshold.
         ActLevel threshold = (thrs_comb[source_num] == 0) ? 0 : thresholds[thrs_comb[source_num] - 1];

         // Add current regulation as present.
         string regulation_name = source_name + ":" + toString(threshold);

         // Add the regulation to the source
         parameter.context += regulation_name + ",";

         // Find in which levels the specie must be for the regulation to occur.
         ActLevel next_th = (thrs_comb[source_num] == thresholds.size()) ? model.getMax(source_ID) + 1 : thresholds[thrs_comb[source_num]];
         Levels activity_levels = range(threshold, next_th);
         parameter.requirements.insert(make_pair(source_ID, activity_levels));
      }

      parameter.targets = model.getBasalTargets(t_ID);

      // Remove the last comma and return.
      parameter.context = parameter.context.substr(0, parameter.context.length() - 1);
      return parameter;
   }

   /**
    * @brief createParameters Creates a description of kinetic parameters.
    * @param target_ID
    * @param formula
    * @return
    */
   Model::Parameters createParameters(const SpecieID t_ID) const {
      auto all_thrs = model.getThresholds(t_ID);
      Levels bottom, thrs_comb, top;
      Model::Parameters parameters;

      // These containers hold number of thresholds per regulator.
      for (auto & source_thresholds:all_thrs) {
         bottom.push_back(0);
         thrs_comb.push_back(0);
         top.push_back(source_thresholds.second.size());
      }

      // Loop over all the contexts.
      do {
         parameters.push_back(getSingleParam(all_thrs, thrs_comb, t_ID));
      } while(iterate(top, bottom, thrs_comb));

      return parameters;
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

      // Add levels to the regulations.
      fillActivationLevels();

      // For each specie create its parameters.
      specie = XMLHelper::getChildNode(structure_node, "SPECIE");
      for (SpecieID ID = 0; specie; ID++, specie = specie->next_sibling("SPECIE") ) {
         // Create all contexts with all the possible values.
         auto parameters = createParameters(ID);

         // If logic description is given evaluate results.
         auto formula = parseLogic(specie, ID);
         if (!formula.empty())
            throw runtime_error("Logical expression temporarily disabled.");

         // Otherwise replace values.
         replaceExplicit(parameters, specie, ID);
         model.addParameters(ID, parameters);
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
      // Create the species
      firstParse(XMLHelper::getChildNode(model_node, "STRUCTURE"));
      // Add regulatory logic
      secondParse(XMLHelper::getChildNode(model_node, "STRUCTURE"));
   }
};

#endif // PARSYBONE_NETWORK_PARSER_INCLUDED
