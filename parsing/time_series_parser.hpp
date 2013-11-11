#ifndef PARSYBONE_TIME_SERIES_PARSER_INCLUDED
#define PARSYBONE_TIME_SERIES_PARSER_INCLUDED

#include "PunyHeaders/common_functions.hpp"

#include "../auxiliary/data_types.hpp"
#include "xml_helper.hpp"
#include "../model/property_automaton.hpp"
#include "../model/model.hpp"
#include "property_parsing.hpp"

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
      size_t val;
      size_t min_acc = XMLHelper::getAttribute(val, series_node, "min_acc", false) ? val : 1;
      size_t max_acc = XMLHelper::getAttribute(val, series_node, "max_acc", false) ? val : INF;
      property.setBounds(min_acc, max_acc);

      // Read all the measurements. For each add tt self-loop and conditional step to the next state
      StateID ID = 0;
      for (auto expression : XMLHelper::NodesRange(series_node, "EXPR", true)) {
         property.addState(toString(ID), false);

         PropertyAutomaton::Constraints cons = PropertyParsing::readConstraints(expression);

         // Initial state lack the self-loop (optimization)

         property.addEdge(ID, ID + 1, cons);
         if (ID != 0) {
            cons.values = "!" + cons.values;
            property.addEdge(ID, ID, cons);
         }
         ID++;
      }

      // Add a final state that marks succesful time series walk
      property.addState(toString(ID), true);

      return property;
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
