#ifndef PARSYBONE_TIME_SERIES_PARSER_INCLUDED
#define PARSYBONE_TIME_SERIES_PARSER_INCLUDED

#include "PunyHeaders/common_functions.hpp"

#include "../auxiliary/data_types.hpp"
#include "xml_helper.hpp"
#include "../model/property_automaton.hpp"
#include "../model/model.hpp"

#include <algorithm>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief This class parses takes the time series and builds it into a Buchi automaton.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class TimeSeriesParser {
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // PARSING:
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   /**
    * Function reads an EXPR tag and creates according state and transitions.
    */
   static PropertyAutomaton parseExpressions(const rapidxml::xml_node<> * const series_node, const string & name) {
      PropertyAutomaton property(name);

      // Read all the measurements. For each add tt self-loop and conditional step to the next state
      StateID ID = 0;
      for (rapidxml::xml_node<> *expression = XMLHelper::getChildNode(series_node, "EXPR"); expression; ID++, expression = expression->next_sibling("EXPR") ) {
         property.addState(toString(ID), false);

         // Self-loop assuring possibility of a value change
         property.addEdge(ID, ID, "tt");

         // Labelled transition to the next measurement
         string values;
         XMLHelper::getAttribute(values, expression, "values");
         values.erase(remove_if(values.begin(), values.end(),
                                [](const char c){return (c == ' ') | (c == '\t') | (c == '\n')  | (c == '\f')  | (c == '\r') | (c == '\v');
         }), values.end());
         property.addEdge(ID, ID + 1, values);
      }

      // Add a final state that marks succesful time series walk
      property.addState(toString(ID), true);
      property.addEdge(ID, ID, "ff");

      return property;
   }
public:
   /**
     * Main parsing function. It expects a pointer to inside of a MODEL node.
     */
   static PropertyAutomaton parse(const rapidxml::xml_node<> * const series_node, const string & name) {
      // Parse Buchi Automaton
      return parseExpressions(series_node, name);
   }
};

#endif // PARSYBONE_TIME_SERIES_PARSER_INCLUDED
