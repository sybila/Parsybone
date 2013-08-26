/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PROPERTY_AUTOMATON_HPP
#define PROPERTY_AUTOMATON_HPP

#include "PunyHeaders/common_functions.hpp"
#include "../auxiliary/data_types.hpp"

class PropertyAutomaton {
public:
   typedef pair<StateID, string> Edge; ///< Edge in Buchi Automaton (Target ID, edge label).
   typedef vector<Edge> Edges; ///< Set of outgoing edges.

private:
   struct AutomatonState {
      string name; ///< Label of the state.
      SpecieID ID; ///< Numerical constant used to distinguish the state. Starts from 0!
      bool final; ///< True if the state is final.

      Edges edges; ///< Edges in Buchi Automaton (Target ID, edge label).
   };

   string automaton_name; ///< Name of the property.
   PropType prop_type; ///< What property does this automaton hold.
   vector<AutomatonState> states; ///< vector of all states of the controlling Buchi automaton

public:
   PropertyAutomaton(const string & _automaton_name = "default_aut", const PropType _prop_type = LTL) : automaton_name(_automaton_name), prop_type(_prop_type){ }

   /**
    * Add a new state to the automaton. If the name is empty, then "ID of the automaton" + "letter" is used.
    *
    * @return	ID of state in the vector
    */
   inline size_t addState(string name, bool final) {
      if (name.empty())
         name = automaton_name + toString(states.size());
      states.push_back({name, states.size(), final, Edges()});
      return states.size() - 1;
   }

   /**
    * @return	number of the states
    */
   inline size_t getStatesCount() const {
      return states.size();
   }

   /**
    * Add a new edge - edge is specified by the target state and label.
    */
   inline void addEdge(StateID source_ID, StateID target_ID, const string & edge_label) {
      states[source_ID].edges.push_back(Edge(target_ID, edge_label));
   }

   /**
    * Finds ordinal number of the BA state based on its name or number string.
    * @return	number of the state with the specified name if there is such, otherwise INF
    */
   SpecieID findID(const string & name) const {
      for (const AutomatonState & state : states)
         if (state.name.compare(name) == 0)
            return state.ID;

      return INF;
   }

   const string & getName(StateID ID) {
      return states[ID].name;
   }

   const string & getAutomatonName() {
      return automaton_name;
   }

   inline bool isFinal(const size_t ID) const {
      return states[ID].final;
   }

   inline const Edges & getEdges(const SpecieID ID) const {
      return states[ID].edges;
   }

   inline PropType getPropType() const {
      return prop_type;
   }
};

#endif // PROPERTY_AUTOMATON_HPP
