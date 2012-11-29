/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_AUTOMATON_BUILDER_INCLUDED
#define PARSYBONE_AUTOMATON_BUILDER_INCLUDED

#include "../parsing/model.hpp"
#include "../parsing/formulae_parser.hpp"
#include "automaton_structure.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  \brief Transform graph of the automaton into a set of labeled transitions in an AutomatonStructure object.
///
/// This builder creates a basic automaton controlling property - this automaton is based on the AutomatonInterface.
/// Automaton is provided with string labels on the edges that are parsed and resolved for the graph.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class AutomatonBuilder {
   const Model & model; ///< Model that holds the data.
   AutomatonStructure & automaton; ///< Automaton that will be created.

   Levels maxes; ///< Maximal activity levels of the species.
   Levels mins; ///< Minimal activity levels of the species.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTATION METHODS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
   /**
    * Compute a vector of maximal levels and store information about states.
    */
   void computeBoundaries() {
      for(size_t specie_num = 0; specie_num < model.getSpeciesCount(); specie_num++) {
         // Maximal values of species
         maxes.push_back(model.getMax(specie_num));
         mins.push_back(model.getMin(specie_num));
      }
   }

   /**
    * Create a set with values from the range [start, end].
    */
   const set<size_t> fillInterval(const size_t start, const size_t end) const {
      set<size_t> interval;
      for (size_t pos = start; pos <= end; pos++) {
         interval.insert(pos);
      }
      return interval;
   }

   /**
    * For each atom decide its valuation in the current state.
    */
   const map<string, bool> getValuation(const vector<string> & atoms, const vector<pair<SpecieID, set<size_t> > > & values,
                                                  const Levels & levels) const {
      // Go through atoms
      map<string, bool> valuation;
      for (size_t atom_num = 0; atom_num < atoms.size(); atom_num++) {
         auto value = values[atom_num];
         // Atom is valid, if the species' level is in the range
         bool is_valid = value.second.find(levels[value.first]) != value.second.end();
         valuation.insert(make_pair(atoms[atom_num], is_valid));
      }

      return valuation;
   }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PARSING METHODS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	

   /**
    * Obtain a vector of individual atoms from the label (their values will then be resolved for valuation).
    */
   vector<string> getAtoms(const string & label) const {
      vector<string> atoms;
      string atom;

      // Go through the label
      for (size_t pos = 0; pos < label.length(); pos++) {
         char ch = label[pos];
         if (!(isspace(ch) || isalpha(ch) || isdigit(ch) || ch == '<' || ch == '>' || ch == '=' || ch == '!' || ch == '|' || ch == '&' || ch == '(' || ch == ')'))
            throw runtime_error(string("String: ").append(label).append(" contains invalid character: ").append(&ch).c_str());

         // If the atom is not a logic symbol, add it to current atom, otherwise store the atom, if any
         if (!(ch == '!' || ch == '|' || ch == '&' || ch == '(' || ch == ')'))
            atom.push_back(ch);
         else if (!atom.empty() && atom.compare("tt") && atom.compare("ff"))
            atoms.push_back(move(atom));
      }
      // Store the last atom, if any (only if the atom is the formula itself)
      if (!atom.empty() && atom.compare("tt") && atom.compare("ff"))
         atoms.push_back(move(atom));

      return atoms;
   }


   /**
    * For each atom, compute possible values of the specie in which the atom is true.
    */
   vector<pair<SpecieID, set<size_t> > > getValues(const vector<string> atoms) const {
      vector<pair<SpecieID, set<size_t> > > values;

      for (auto atom = atoms.begin(); atom != atoms.end(); atom++) {
         // Find comparison operator
         size_t symbol = atom->find_first_of("<>=");
         if (symbol == string::npos)
            throw invalid_argument(string("No comparison operator found in the atom ").append(*atom));

         // Find ID of the specie mentioned in the atom
         SpecieID ID = model.findID(atom->substr(0, symbol));
         if (ID >= model.getSpeciesCount())
            throw invalid_argument(string("Invalid specie name in the atom ").append(*atom));

         // Find a value the specie is compared to
         size_t value;
         try {
            value = lexical_cast<size_t, string>(atom->substr(symbol+1));
         } catch (bad_lexical_cast e) {
            output_streamer.output(error_str, string("Error while parsing a propery subformula ").append(*atom).append(": ").append(e.what()));
            throw runtime_error("lexical_cast<size_t, string>(atom->substr(0, symbol-1) failed");
         }
         if (value < model.getMin(ID) || value > model.getMax(ID))
            throw invalid_argument(string("Value of the specie out of specie range in the atom ").append(*atom));

         // Fill in all the values satisfying the atom
         if (atom->find("<") != string::npos)
            values.push_back(make_pair(ID, fillInterval(value, model.getMax(ID))));
         else if (atom->find("=") != string::npos)
            values.push_back(make_pair(ID, fillInterval(value, value)));
         else if (atom->find(">") != string::npos)
            values.push_back(make_pair(ID, fillInterval(model.getMin(ID), value)));
      }
      return values;
   }

   /**
    * Computes a vector containing all Levels that are acceptable for a transition with a given label.
    */
   Configurations getAllowed(const string & label) const {
      // Get atoms of the lable
      auto atoms = getAtoms(label);
      // Decide in which activation levels of species those atoms are true
      auto values = getValues(atoms);

      // Try all combinations of values that are possible and for each resolve the label
      Configurations allowed;
      Levels iterated = mins;
      do {
         auto valuation = getValuation(atoms, values, iterated);
         if (FormulaeParser::resolve(valuation, label))
            allowed.push_back(iterated);
      } while (iterate(maxes, mins, iterated));

      return allowed;
   }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTING METHODS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	AutomatonBuilder(const AutomatonBuilder & other); ///< Forbidden copy constructor.
	AutomatonBuilder& operator=(const AutomatonBuilder & other); ///< Forbidden assignment operator.

	/**
    * Creates transitions from labelled edges of BA and passes them to automaton structure.
	 *
	 * @param state_num	index of the state of BA 
	 * @param start_position	index of the last transition created
	 */
	void addTransitions(const StateID ID, size_t & transition_count) const {
      const vector<Model::Egde> & edges = model.getEdges(ID);

		// Transform each edge into transition and pass it to the automaton
		for (size_t edge_num = 0; edge_num < model.getEdges(ID).size(); edge_num++) {
			// Compute allowed values from string of constrains
			Configurations allowed_values = move(getAllowed(edges[edge_num].second));
			// If the transition is possible for at least some values, add it
         if (!allowed_values.empty()) {
            automaton.addTransition(ID, edges[edge_num].first, move(allowed_values));
				transition_count++;
			}
		}
	}

public:
	/**
    * Constructor computes boundaries of the state space and passes references.
	 */
   AutomatonBuilder(const Model & _model, AutomatonStructure & _automaton) : model(_model), automaton(_automaton) {
      computeBoundaries();
	}

	/**
    * Create the transitions from the model and fill the automaton with them.
	 */
	void buildAutomaton() {
		output_streamer.output(stats_str, "Costructing Buchi automaton.");
		output_streamer.output(stats_str, "Total number of states: ", OutputStreamer::no_newl | OutputStreamer::tab)
                     .output(model.getStatesCount(), OutputStreamer::no_newl).output(".");
		size_t transition_count = 0;

		// List throught all the automaton states
		for (StateID ID = 0; ID < model.getStatesCount(); ID++) {
			// Fill auxiliary data
			automaton.addState(ID, model.isFinal(ID));
			// Add transitions for this state
			addTransitions(ID, transition_count);
		}

		output_streamer.output(stats_str, "Total number of transitions: ", OutputStreamer::no_newl | OutputStreamer::tab)
                     .output(transition_count, OutputStreamer::no_newl).output(".");
	}
};

#endif // PARSYBONE_AUTOMATON_BUILDER_INCLUDED
