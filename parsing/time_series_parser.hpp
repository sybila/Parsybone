#ifndef PARSYBONE_TIME_SERIES_PARSER_INCLUDED
#define PARSYBONE_TIME_SERIES_PARSER_INCLUDED

#include "../auxiliary/data_types.hpp"
#include "../auxiliary/common_functions.hpp"
#include "xml_helper.hpp"
#include "model.hpp"

#include <algorithm>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// This class parses takes the time series and builds it into a B\"uchi automaton.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class TimeSeriesParser {
   Model & model; ///< Reference to the model object that will be filled.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PARSING:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   /**
    * Function reads an EXPR tag and creates according state and transitions.
    */
   void parseExpressions(const rapidxml::xml_node<> * const series_node) const {
      // Read all the measurements. For each add tt self-loop and conditional step to the next state
      StateID ID = 0;
      for (rapidxml::xml_node<> *expression = XMLHelper::getChildNode(series_node, "EXPR"); expression; ID++, expression = expression->next_sibling("EXPR") ) {
         model.addState(toString(ID), false);

         // Self-loop assuring possibility of a value change
         model.addConditions(ID, ID, std::move(std::string("tt")));

         // Labelled transition to the next measurement
         std::string values;
         XMLHelper::getAttribute(values, expression, "values");
         values.erase(std::remove_if(values.begin(), values.end(), isspace), values.end());
         model.addConditions(ID, ID + 1, std::move(values));
      }

      // Add a final state that marks succesful time series walk
      model.addState(toString(ID), true);
      model.addConditions(ID, ID, std::move(std::string("ff")));
   }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION METHODS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   TimeSeriesParser(const TimeSeriesParser & other); ///< Forbidden copy constructor.
   TimeSeriesParser& operator=(const TimeSeriesParser & other); ///< Forbidden assignment operator.

public:
   TimeSeriesParser(Model & _model) : model(_model) { } ///< Simple constructor, passes references

   /**
    * Main parsing function. It expects a pointer to inside of a MODEL node.
    */
   void parse(const rapidxml::xml_node<> * const model_node) {
      // Parse Buchi Automaton
      output_streamer.output(verbose_str, "Started reading of the Time series.");

      parseExpressions(XMLHelper::getChildNode(model_node, "SERIES"));
   }
};

#endif // PARSYBONE_TIME_SERIES_PARSER_INCLUDED
