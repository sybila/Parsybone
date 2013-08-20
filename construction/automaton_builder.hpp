/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_AUTOMATON_BUILDER_INCLUDED
#define PARSYBONE_AUTOMATON_BUILDER_INCLUDED

#include "PunyHeaders/common_functions.hpp"

#include "../model/model_translators.hpp"
#include "../model/property_automaton.hpp"
#include "automaton_structure.hpp"
#include "automaton_helper.hpp"

// TODO: All the possible states are stored for outgoing transitions. Change.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  \brief Transform graph of the automaton into a set of labeled transitions in an AutomatonStructure object.
///
/// This builder creates a basic automaton controlling property - this automaton is based on the AutomatonInterface.
/// Automaton is provided with string labels on the edges that are parsed and resolved for the graph.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class AutomatonBuilder {
   const Model & model; ///< Model that holds the data.
   const PropertyAutomaton & source;

   Levels maxes; ///< Maximal activity levels of the species.
   Levels mins; ///< Minimal activity levels of the species.

   /**
    * Compute a vector of maximal levels and store information about states.
    */
   void computeBoundaries() {
      for(const SpecieID ID : scope(model.species)) {
         // Maximal values of species
         maxes.push_back(model.getMax(ID));
         mins.push_back(model.getMin(ID));
      }
   }

   /**
    * Computes a vector containing all Levels that are acceptable for a transition with a given label.
    */
   Configurations getAllowed(const string & label) const {
      // Get atoms of the lable
      auto atoms = AutomatonHelper::getAtoms(label);
      // Decide in which activation levels of species those atoms are true
      auto values = AutomatonHelper::getValues(atoms, model);

      // Try all combinations of values that are possible and for each resolve the label
      Configurations allowed;
      Levels iterated = mins;
      do {
         auto valuation = AutomatonHelper::getValuation(atoms, values, iterated);
         if (FormulaeResolver::resolve(valuation, label))
            allowed.push_back(iterated);
      } while (iterate(maxes, mins, iterated));

      return allowed;
   }

   /**
    * Creates transitions from labelled edges of BA and passes them to automaton structure.
    */
   void addTransitions(AutomatonStructure & automaton, const StateID ID) const {
      const PropertyAutomaton::Edges & edges = source.getEdges(ID);

      // Transform each edge into transition and pass it to the automaton
      for (size_t edge_num = 0; edge_num < source.getEdges(ID).size(); edge_num++) {
         // Compute allowed values from string of constrains
         Configurations allowed_values = move(getAllowed(edges[edge_num].second));
         // If the transition is possible for at least some values, add it
         if (!allowed_values.empty()) {
            automaton.addTransition(ID, edges[edge_num].first, allowed_values);
         }
      }
   }

public:
   AutomatonBuilder(const Model & _model, const PropertyAutomaton & _source) : model(_model), source(_source) {
      computeBoundaries();
   }

   /**
    * Create the transitions from the model and fill the automaton with them.
    */
   AutomatonStructure buildAutomaton() {
      AutomatonStructure automaton;
      const size_t state_count = source.getStatesCount();
      size_t state_no = 0;

      // List throught all the automaton states
      for (StateID ID = 0; ID < source.getStatesCount(); ID++) {
         output_streamer.output(verbose_str, "Building automaton state: " + toString(++state_no) + "/" + toString(state_count) + ".", OutputStreamer::no_newl | OutputStreamer::rewrite_ln);
         // Fill auxiliary data
         automaton.addState(ID, source.isFinal(ID));
         // Add transitions for this state
         addTransitions(automaton, ID);
      }
      output_streamer.clear_line(verbose_str);

      return automaton;
   }
};

#endif // PARSYBONE_AUTOMATON_BUILDER_INCLUDED
