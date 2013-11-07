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
   /**
    * Function reads an EXPR tag and creates according state and transitions.
    * First state has only transition under given condition. Others have loop if (!expression) and transition if (expression).
    */
   static PropertyAutomaton parseExpressions(const rapidxml::xml_node<> * const series_node) {
      PropertyAutomaton property(TimeSeries);

      // Read all the measurements. For each add tt self-loop and conditional step to the next state
      StateID ID = 0;
      for (auto expression : XMLHelper::NodesRange(series_node, "EXPR")) {
         property.addState(toString(ID), false);

         // Labelled transition to the next measurement
         string values;
         XMLHelper::getAttribute(values, expression, "values");
         values.erase(remove_if(values.begin(), values.end(), static_cast<int(*)(int)>(isspace)), values.end());
         if (property.getStatesCount() > 1)
            property.addEdge(ID, ID, "!" + values);
         property.addEdge(ID, ID + 1, values);
         ID++;
      }

      // Add a final state that marks succesful time series walk
      property.addState(toString(ID), true);
      property.addEdge(ID, ID, "ff");

      return property;
   }

   static PropertyAutomaton parseStable(const rapidxml::xml_node<> * const series_node) {

   }

public:
   /**
     * Main parsing function. It expects a pointer to inside of a MODEL node.
     */
   static PropertyAutomaton parse(const rapidxml::xml_node<> * const series_node) {
      // Parse Buchi Automaton
      return parseExpressions(series_node);
   }
};

#endif // PARSYBONE_TIME_SERIES_PARSER_INCLUDED
