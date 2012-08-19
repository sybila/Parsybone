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
/// AutomatonBuilder transform graph of the automaton into set of transitions that know values necessary for transition to be feasible.
/// Correspondence to the states of the automaton itself assured by storing the source in the transition and correct ordering of the vector of transitions.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class AutomatonBuilder {
	const Model & model; ///< Model that holds the data
	AutomatonStructure & automaton; ///< Automaton that will be created

   Levels maxes; ///< Maximal activity levels of the species
   Levels mins; ///< Minimal activity levels of the species

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTATION FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
   /**
    * Compute a vector of maximal levels and store information about states
    */
   void computeBoundaries() {
      for(std::size_t specie_num = 0; specie_num < model.getSpeciesCount(); specie_num++) {
         // Maximal values of species
         maxes.push_back(model.getMax(specie_num));
         mins.push_back(model.getMin(specie_num));
      }
   }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PARSING FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	

   std::vector<std::string> getAtoms(const std::string & label) const {
      std::vector<std::string> atoms;
      bool reading = false;
      std::string atom;
      for (std::size_t pos = 0; pos < label.length(); pos++) {
         char ch = label[pos];
         if (!(isalpha(ch) || isdigit(ch) || ch == '<' || ch == '>' || ch == '=' || ch == '!' || ch == '|' || ch == '&' || ch == '(' || ch == ')'))
            throw std::runtime_error(std::string("String: ").append(label).append(" contains invalid character: ").append(&ch).c_str());
         if (!(ch == '!' || ch == '|' || ch == '&' || ch == '(' || ch == ')'))
            atom.push_back(ch);
         else if (!atom.empty() && atom.compare("tt") && atom.compare("ff"))
            atoms.push_back(std::move(atom));
      }
      if (!atom.empty() && atom.compare("tt") && atom.compare("ff"))
         atoms.push_back(std::move(atom));

      return atoms;
   }

   const std::set<std::size_t> fillInterval(const std::size_t start, const std::size_t end) const {
      std::set<std::size_t> interval;
      for (std::size_t pos = start; pos <= end; pos++) {
         interval.insert(pos);
      }
      return interval;
   }

   std::vector<std::pair<SpecieID, std::set<std::size_t> > > getValues(const std::vector<std::string> atoms) const {
      std::vector<std::pair<SpecieID, std::set<std::size_t> > > values;
      for (auto atom = atoms.begin(); atom != atoms.end(); atom++) {
         std::size_t symbol = atom->find_first_of("<>=");
         if (symbol == std::string::npos)
            throw std::invalid_argument(std::string("No comparison operator found in the atom ").append(*atom));

         SpecieID ID = model.findID(atom->substr(0, symbol));
         if (ID == ~0)
            throw std::invalid_argument(std::string("Invalid specie name in the atom ").append(*atom));

         std::size_t value;
         try {
            value = boost::lexical_cast<size_t, std::string>(atom->substr(symbol+1));
         } catch (boost::bad_lexical_cast e) {
            output_streamer.output(error_str, std::string("Error while parsing a propery subformula ").append(*atom).append(": ").append(e.what()));
            throw std::runtime_error("boost::lexical_cast<size_t, std::string>(atom->substr(0, symbol-1) failed");
         }

         if (atom->find("<") != std::string::npos)
            values.push_back(std::make_pair(ID, fillInterval(value, model.getMax(ID))));
         else if (atom->find("=") != std::string::npos)
            values.push_back(std::make_pair(ID, fillInterval(value, value)));
         else if (atom->find(">") != std::string::npos)
            values.push_back(std::make_pair(ID, fillInterval(model.getMin(ID), value)));
      }
      return values;
   }

   const std::map<std::string, bool> getValuation(const std::vector<std::string> & atoms,
      const std::vector<std::pair<SpecieID, std::set<std::size_t> > > & values, const Levels & levels) const {
      std::map<std::string, bool> valuation;
      for (std::size_t atom_num = 0; atom_num < atoms.size(); atom_num++) {
         auto value = values[atom_num];
         bool is_valid = value.second.find(levels[value.first]) != value.second.end();
         valuation.insert(std::make_pair(atoms[atom_num], is_valid));
      }
      return valuation;
   }

   AllowedValues getAllowed(const std::string & label) const {
      auto atoms = getAtoms(label);
      auto values = getValues(atoms);
      AllowedValues allowed;
      Levels iterated = mins;
      do {
         auto valuation = getValuation(atoms, values, iterated);
         if (FormulaeParser::resolve(valuation, label))
            allowed.push_back(iterated);
      } while (iterate(maxes, mins, iterated));
      return allowed;
   }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTING FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	AutomatonBuilder(const AutomatonBuilder & other); ///< Forbidden copy constructor.
	AutomatonBuilder& operator=(const AutomatonBuilder & other); ///< Forbidden assignment operator.

	/**
	 * Creates transitions from labelled edges of BA and passes them to automaton structure
	 *
	 * @param state_num	index of the state of BA 
	 * @param start_position	index of the last transition created
	 */
	void addTransitions(const StateID ID, std::size_t & transition_count) const {
		const std::vector<Model::Egde> & edges = model.getEdges(ID); 

		// Transform each edge into transition and pass it to the automaton
		for (std::size_t edge_num = 0; edge_num < model.getEdges(ID).size(); edge_num++) {
			// Compute allowed values from string of constrains
         AllowedValues allowed_values = std::move(getAllowed(edges[edge_num].second));
			// If the transition is possible for at least some values, add it
         if (!allowed_values.empty()) {
            automaton.addTransition(ID, edges[edge_num].first, std::move(allowed_values));
				transition_count++;
			}
		}
	}

public:
	/**
	 * Constructor just attaches the references to data holders
	 */
	AutomatonBuilder(const Model & _model, AutomatonStructure & _automaton) 
		            : model(_model), automaton(_automaton) {
      computeBoundaries();
	}

	/**
	 * Create the transitions from the model and fill the automaton with them
	 */
	void buildAutomaton() {
		output_streamer.output(stats_str, "Costructing Buchi automaton.");
		output_streamer.output(stats_str, "Total number of states: ", OutputStreamer::no_newl | OutputStreamer::tab)
			.output(model.getStateCount(), OutputStreamer::no_newl).output(".");
		std::size_t transition_count = 0;

		// List throught all the automaton states
		for (StateID ID = 0; ID < model.getStateCount(); ID++) {
			// Fill auxiliary data
			automaton.addState(ID, model.isFinal(ID));
			// Add transitions for this state
			addTransitions(ID, transition_count);
		}

		output_streamer.output(stats_str, "Total number of transitions: ", OutputStreamer::no_newl | OutputStreamer::tab)
			.output(transition_count, OutputStreamer::no_newl).output(".");
	}
};

#endif
