#ifndef PROPERTY_AUTOMATON_HPP
#define PROPERTY_AUTOMATON_HPP

#include "PunyHeaders/common_functions.hpp"
#include "../auxiliary/data_types.hpp"

class PropertyAutomaton {
public:
   typedef pair<StateID, string> Edge; ///< Edge in Buchi Automaton (Target ID, edge label).
   typedef vector<Edge> Edges;

private:
   struct AutomatonState {
      string name; ///< Label of the state.
      SpecieID ID; ///< Numerical constant used to distinguish the state. Starts from 0!
      bool final; ///< True if the state is final.

      Edges edges; ///< Edges in Buchi Automaton (Target ID, edge label).
   };

   string automaton_name;
   vector<AutomatonState> states; ///< vector of all states of the controlling Buchi automaton

public:
   PropertyAutomaton(const string & _automaton_name) : automaton_name(_automaton_name) { }

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
    * Add a new transition - transition is specified by the target state and label.
    */
   inline void addConditions(StateID source_ID, StateID target_ID, const string & edge_label) {
      states[source_ID].edges.push_back(Edge(target_ID, edge_label));
   }

   /**
    * Finds ordinal number of the BA state based on its name or number string.
    *
    * @return	number of the state with the specified name if there is such, otherwise INF
    */
   SpecieID findID(const string & name) const {
      StateID ID = INF;
      for (auto state : states)
         if (state.name.compare(name) == 0)
            return ID;

      return INF;
   }

   /**
    * @brief getName Gets the name of the given automaton state.
    * @param ID
    * @return
    */
   const string & getName(StateID ID) {
      return states[ID].name;
   }


   /**
    * @return	true if the state is final
    */
   inline bool isFinal(const size_t ID) const {
      return states[ID].final;
   }

   /**
    * @return	edges of the state
    */
   inline const Edges & getEdges(const SpecieID ID) const {
      return states[ID].edges;
   }
};

#endif // PROPERTY_AUTOMATON_HPP
