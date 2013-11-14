/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef AUTOMATON_HELPER_HPP
#define AUTOMATON_HELPER_HPP

#include "../auxiliary/data_types.hpp"

namespace AutomatonHelper {
   /**
    * @return a set with values from the range [start, end].
    */
   const set<size_t> fillInterval(const size_t start, const size_t end) {
      set<size_t> interval;
      for (size_t pos = start; pos <= end; pos++) {
         interval.insert(pos);
      }
      return interval;
   }

   /**
    * @return for each atom decide its valuation in the current state.
    */
   const map<string, bool> getValuation(const vector<string> & atoms, const vector<pair<SpecieID, set<size_t> > > & values, const Levels & levels)  {
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

   /**
    * @return a vector of individual atoms from the label (their values will then be resolved for valuation).
    */
   vector<string> getAtoms(const string & label) {
      vector<string> atoms;
      string atom;

      // Go through the label
      for (const size_t pos : scope(label)) {
         char ch = label[pos];
         if (!(isspace(ch) || isalnum(ch) || ch == '_' || ch == '<' || ch == '>' || ch == '=' || ch == '!' || ch == '|' || ch == '&' || ch == '(' || ch == ')'))
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
    * @return  comparison operator position
    */
   size_t findCompPos(const string & atom) {
      size_t comp_pos = atom.find_first_of("<>=");
      if (comp_pos == string::npos)
         throw invalid_argument(string("No comparison operator found in the atom ").append(atom));
      return comp_pos;
   }

   /**
    * @return  ID of the specie mentioned in the atom
    */
   SpecieID findSpec(const string & atom, const Model & model, const size_t comp_pos) {
      SpecieID ID = ModelTranslators::findID(model, atom.substr(0, comp_pos));
      if (ID >= model.species.size())
         throw invalid_argument(string("Invalid specie name in the atom ").append(atom));
      return ID;
   }

   /**
    * @return  a value the specie is compared to
    */
   size_t findCompVal(const string & atom, const Model & model, const size_t comp_pos, const SpecieID ID) {
      size_t value;
      try {
         value = lexical_cast<size_t, string>(atom.substr(comp_pos+1));
      } catch (bad_lexical_cast e) {
         output_streamer.output(error_str, string("Error while parsing a propery subformula ").append(atom).append(": ").append(e.what()));
         throw runtime_error("lexical_cast<size_t, string>(atom->substr(0, comp_pos-1) failed");
      }
      if (value < model.getMin(ID) || value > model.getMax(ID))
         throw invalid_argument(string("Value of the specie out of specie range in the atom ").append(atom));
      return value;
   }

   /**
    * For each atom, compute possible values of the specie in which the atom is true.
    */
   vector<pair<SpecieID, set<size_t> > > getValues(const vector<string> atoms, const Model & model) {
      vector<pair<SpecieID, set<size_t> > > values;

      for (const string & atom : atoms) {
         const size_t comp_pos = findCompPos(atom);
         const SpecieID ID = findSpec(atom, model, comp_pos);
         const size_t comp_value = findCompVal(atom, model, comp_pos, ID);

         // Fill in all the values satisfying the atom
         if (atom.find("<") != string::npos)
            values.push_back(make_pair(ID, AutomatonHelper::fillInterval(model.getMin(ID), comp_value - 1)));
         else if (atom.find("=") != string::npos)
            values.push_back(make_pair(ID, AutomatonHelper::fillInterval(comp_value, comp_value)));
         else if (atom.find(">") != string::npos)
            values.push_back(make_pair(ID, fillInterval(comp_value + 1, model.getMax(ID))));
      }

      return values;
   }
}

#endif // AUTOMATON_HELPER_HPP
