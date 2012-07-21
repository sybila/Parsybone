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
#include "automaton_structure.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// AutomatonBuilder transform graph of the automaton into set of transitions that know values necessary for transition to be feasible.
/// Correspondence to the states of the automaton itself assured by storing the source in the transition and correct ordering of the vector of transitions.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class AutomatonBuilder {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const Model & model; ///< Model that holds the data
	AutomatonStructure & automaton; ///< Automaton that will be created
	AllowedValues all_values; ///< Commonly used structure holding values of the KS that allow some transition

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTATION FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	/**
	 * Creates complete vector of with all levels combinations with no constrains i.e. constrains for tt transition
	 */
	void buildAllValues() {
		// For each specie insert all its posible levels
		for (StateID ID = 0; ID < model.getSpeciesCount(); ID++) {
			// Storage
			std::set<std::size_t> values;
			// Get all the values from the range 0..max(specie)
			for (std::size_t value = 0; value <= model.getMax(ID); value++) {
				values.insert(value);
			}
			// Pass the temporarry
			all_values.push_back(std::move(values));
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PARSING FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	/**
	 * Function takes data parsed from the label and puts the restrictions on the values of species. Restrictions are atoms in the form  [negate]Specie op value
	 *
	 * @param current_values	set with values that might be restricted
	 * @param op	operator of comparation - either '<' or '=' or '>'
	 * @param negate	true if the atom is negated
	 * @param compare_value	value on the right side of the atom
	 * @param specie_ID	ID of the compared specie
	 */
	void eraseForbiddenValues(std::set<std::size_t> & current_values, const char op, const bool negate, const std::size_t compare_value, const std::size_t specie_ID) const {
		// List through the original values and test if they satisfy the property, if not, remove them
		std::for_each(all_values[specie_ID].begin(), all_values[specie_ID].end(), [op, negate, compare_value, &current_values](const std::size_t val) {
			if (negate) { // Erase values satisfying the condition (not satisfying its negation)
				if (op == '<' && val < compare_value )
					current_values.erase(val);
				else if ( op == '>' && val > compare_value )
					current_values.erase(val);
				else if ( op == '=' && val == compare_value )
					current_values.erase(val);
			}
			else { // Erase values not satisfying the condition (operators are complementary)
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
	 *
	 * @return	vector of values of the species that are allowed
	 */
	AllowedValues applyConstrains(std::vector<std::string> & subformulas) const {
		// Data to return
		AllowedValues allowed_values(all_values);

		// Cycle through atomic formulas and get ALL the data
		for (auto formula = subformulas.begin(); formula != subformulas.end(); formula++) {

			// Check negativity and delete the ! if needed
			bool negate = ((*formula)[0] == '!');
			if (negate) *formula = formula->substr( 1, formula->size());

			// Find position of the comparation operator
			const std::size_t op_position = formula->find_first_of("<>=");

			// Specie ID
			std::string specie_name = formula->substr(0, op_position);
			const std::size_t ID = model.findID(specie_name);

			// Operator
			char op = (*formula)[op_position]; // comparation operator

			// Value that is compared
			std::size_t compare_value;	
			try {
				compare_value = boost::lexical_cast<std::size_t,std::string>(formula->substr(op_position+1, formula->size()));
			} catch (boost::bad_lexical_cast e) {
				output_streamer.output(error_str, std::string("Error occured while parsing a label. It seems that you have entered non-numerical value as a value. ").append(e.what()));
				throw std::runtime_error("boost::lexical_cast<size_t, std::string>(formula.substr(op_position+1, formula.size() - 1)) failed");
			}

			// Erase the forbidden values for this atomic formula
			eraseForbiddenValues(allowed_values[ID], op, negate, compare_value, ID);	
		}
		return allowed_values;
	}

	/**
	 * From edge label create constrains on the transition
	 *
	 * @param constrains	string containing the edge label
	 *
	 * @return	constrains that control satisfaction of the label
	 */
	AllowedValues parseConstrains(std::string constrains) const {
		// If the label is always true or alway false, do not even bother with constrains and return all / none
		if (constrains.compare("tt") == 0) {
			AllowedValues allowed_values(all_values);
			return std::move(allowed_values);
		}
		else if (constrains.compare("ff") == 0) {
			AllowedValues none;
			return std::move(none);
		}
		
		// Check if all the characters are within allowed (nums, alpha, <, >, =, !, &)
		std::for_each(constrains.begin(), constrains.end(), [&constrains](char ch) {
			if (!(isalpha(ch) || isdigit(ch) || ch == '<' || ch == '>' || ch == '=' || ch == '!' || ch == '&'))
				throw std::runtime_error(std::string("String: ").append(constrains).append(" contains invalid character: ").append(&ch).c_str());
		});

		// Get all subparts of dual clause (splitting)
		std::vector<std::string> subformulas;
		try {
        boost::split(subformulas, constrains, boost::is_any_of("&"));
		} catch (std::exception & e) {
			output_streamer.output(error_str, std::string("Error occured while parsing a label. ").append(e.what()));
			throw std::runtime_error("boost::split(subformulas, constrains, boost::is_any_of(\"&\")) failed");
		}

		// Return values with 
		return std::move(applyConstrains(subformulas));
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
			AllowedValues constrained_values = std::move(parseConstrains(edges[edge_num].second));
			// If the transition is possible for at least some values, add it
			if (!constrained_values.empty()) {
				automaton.addTransition(ID, edges[edge_num].first, std::move(constrained_values));
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
		// Create vector all all values that can be present
		buildAllValues();
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
