/*
 * Copyright (C) 2012 - Adam Streck
 *
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 *
 * ParSyBoNe is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 *
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno.
 * See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_AUTOMATON_BUILDER_INCLUDED
#define PARSYBONE_AUTOMATON_BUILDER_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AutomatonBuilder transform graph of the automaton into set of transitions that know values necessary for transition to be feasible.
// Correspondence to the states of the automaton itself assured by storing the source in the transition and correct ordering of the vector of transitions.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <set>
#include <cctype>
#include <stdexcept>

#include <boost/algorithm/string.hpp>

#include "../parsing/model.hpp"
#include "automaton_structure.hpp"
#include "../auxiliary/output_streamer.hpp"

class AutomatonBuilder {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const UserOptions & user_options;
	const Model & model;
	AutomatonStructure & automaton;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTATION FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	

	/**
	 * Creates complete vector of with all levels combinations with no constrains i.e. constrains for tt transition
	 *
	 * @return	vector of all possible values of all the species
	 */
	const std::vector<std::set<std::size_t> > buildAllValues() {
		std::vector<std::set<std::size_t> > all_values;
		// For each specie insert all its posible levels
		for (std::size_t specie_num = 0; specie_num < model.getSpeciesCount(); specie_num++) {
			std::set<std::size_t> values;
			for (std::size_t value = 0; value <= model.getMax(specie_num); value++) {
				values.insert(value);
			}
			all_values.push_back(std::move(values));
		}
		return all_values;
	}

	/**
	 * Function takes data parsed from the label and puts the restrictions on the values of species. Restrictions are atoms in the form  [negate]Specie op value
	 *
	 * @param current_values	set with values that might be restricted
	 * @param all_values	origin holding all the values with no constrains
	 * @param op	operator of comparation - either '<' or '=' or '>'
	 * @param negate	true if the atom is negated
	 * @param compare_value	value on the right side of the atom
	 * @param specie_ID	ID of the compared specie
	 */
	void eraseForbiddenValues(std::set<std::size_t> & current_values, const std::vector<std::set<std::size_t> > & all_values, const char op, 
	                           const bool negate, const std::size_t compare_value, const std::size_t specie_ID) {
		// List through the original values
		std::for_each(all_values[specie_ID].begin(), all_values[specie_ID].end(), [op, negate, compare_value, &current_values](const std::size_t val) {
			if (negate) { // Erase values satisfying the condition (not satisfying its negation)
				if (op == '<' && val < compare_value )
					current_values.erase(val);
				else if ( op == '>' && val > compare_value )
					current_values.erase(val);
				else if ( op == '=' && val == compare_value )
					current_values.erase(val);
			}
			else { // Erase values not satisfying the condition
				if (op == '<' && val >= compare_value )
					current_values.erase(val);
				else if ( op == '>' && val <= compare_value )
					current_values.erase(val);
				else if ( op == '=' && val != compare_value )
					current_values.erase(val);
			}
		});
	}

	/**
	 * Creates a vector of allowed values for individual species by removing non-allowed values from the vector of all possible values accordingly to atoms of a formula.
	 *
	 * @param subformulas	vector of atoms or negative atoms
	 * @param all_values	vector of all possible values of all the species
	 *
	 * @return	vector of values of the species that are allowed
	 */
	std::vector<std::set<std::size_t> > applyConstrains(std::vector<std::string> & subformulas, const std::vector<std::set<std::size_t> > & all_values) {
		// Data to return
		std::vector<std::set<std::size_t> > allowed_values(all_values);
		// Cycle through atoms or negations of atoms
		for (auto formula = subformulas.begin(); formula != subformulas.end(); formula++) {
			// Check negativity and delete the ! if needed
			bool negate = ((*formula)[0] == '!');
			if (negate) *formula = formula->substr( 1, formula->size());
			// Find position of the comparation operator
			const std::size_t op_position = formula->find_first_of("<>=");

			// Get all the data from the formula
			std::string specie_name = formula->substr(0, op_position);
			char op = (*formula)[op_position]; // comparation operator
			std::size_t compare_value;	
			try {
				compare_value = boost::lexical_cast<std::size_t,std::string>(formula->substr(op_position+1, formula->size()));
			} catch (boost::bad_lexical_cast e) {
				std::cerr << "Error occured while parsing a label. It seems that you have entered non-numerical value as a value. " << e.what() << "\n";
				throw std::runtime_error("boost::lexical_cast<size_t, std::string>(formula.substr(op_position+1, formula.size() - 1)) failed");
			}

			// Find index of the specie
			const std::size_t ID = model.findID(specie_name);

			// Erase the forbidden values
			eraseForbiddenValues(allowed_values[ID], all_values, op, negate, compare_value, ID);	
		}
		return allowed_values;
	}

	/**
	 * From edge label create constrains on the transition
	 *
	 * @param constrains	string containing the edge label
	 * @param all_values	mask for the constrains for tt transition
	 *
	 * @return	constrains that control satisfaction of the label
	 */
	std::vector<std::set<std::size_t> > parseConstrains(std::string constrains, const std::vector<std::set<std::size_t> > & all_values) {
		// If the label is always true, do not even bother with constrains
		if (constrains.compare("tt") == 0) {
			std::vector<std::set<std::size_t> > allowed_values(all_values);
			return std::move(allowed_values);
		}
		
		// Check for incorrect characters - to be removed later.
		std::for_each(constrains.begin(), constrains.end(), [&constrains](char ch) {
			if (!(isalpha(ch) || isdigit(ch) || ch == '<' || ch == '>' || ch == '=' || ch == '!' || ch == '&'))
				throw std::runtime_error(std::string("String: ").append(constrains).append(" contains invalid character: ").append(&ch).c_str());
		});

		// Get all subparts of dual clause (splitting)
		std::vector<std::string> subformulas;
		try {
			boost::split(subformulas, constrains, boost::is_any_of("&"));
		} catch (std::exception & e) {
			std::cerr << "Error occured while parsing a label. " << e.what() << "\n";
			throw std::runtime_error("boost::split(subformulas, constrains, boost::is_any_of(\"&\")) failed");
		}

		return std::move(applyConstrains(subformulas, all_values));
	}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTING FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	AutomatonBuilder(const AutomatonBuilder & other);            // Forbidden copy constructor.
	AutomatonBuilder& operator=(const AutomatonBuilder & other); // Forbidden assignment operator.

public:
	/**
	 * Constructor just attaches the references to data holders
	 */
	AutomatonBuilder(const UserOptions &_user_options, const Model & _model, AutomatonStructure & _automaton) 
		: user_options(_user_options), model(_model), automaton(_automaton) { }

	/**
	 * Create the transitions from the model and fill the automaton with them
	 */
	void buildAutomaton() {
		// Reference data
		const std::vector<std::set<std::size_t> > all_values = std::move(buildAllValues());
		// Index of the first transition for each transition in the vector of transition
		std::size_t state_begin = 0;

		output_streamer.output(verbose, "Buchi automaton structure states, total number of states: ", OutputStreamer::no_newl)
			           .output(model.getStatesCount(), OutputStreamer::no_newl).output(".");
		// List throught all the automaton states
		for (std::size_t state_num = 0; state_num < model.getStatesCount(); state_num++) {
			// Auxiliary data - position of the first function with source being state[state_num]
			automaton.addStateBegin(state_begin);
			automaton.addFinality(model.isFinal(state_num));

			// List through all the edges of the state
			const std::vector<Model::Egde> & edges = model.getEdges(state_num); 
			// Transform each edge into transition and pass it to the automaton
			for (std::size_t edge_num = 0; edge_num < model.getEdges(state_num).size(); edge_num++) {
				// Compute data and pass them
				automaton.addTransition(state_num, edges[edge_num].first, std::move(parseConstrains(edges[edge_num].second, all_values)));
				state_begin++;
			}
		}
		// Add the first index after last transition - used to check the range to search the transition in
		automaton.addStateBegin(state_begin);
	}
};

#endif