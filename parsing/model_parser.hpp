/*
 * Copyright (C) 2012 - Adam Streck
 *
 * This file is part of PoSeIDoN (Parameter Synthetizer for Discrete Networks) verification tool
 *
 * Poseidon is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Poseidon is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 *
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno.
 * See http://sybila.fi.muni.cz/ .
 */

// TODO add exceptions specification

#ifndef POSEIDON_MODEL_PARSER_INCLUDED
#define POSEIDON_MODEL_PARSER_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ModelParser parses provided input stream and stores data in the provided Model object.
// Data are mostly kept in the raw form, same as in the model file.
// Most of the possible mistakes and typos would case an exception and failure of the program.
// Only syntactic correcntess is checked. Wrong semantics would pass through here!
// There is only single public functions (apart from the constructor) - parseInput(), that performs the whole process.
// The functions are rather long, but their meaning is quite straithforward and repetitive. Most of the code are controls of correctness.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <algorithm>
#include <cstring>

#include <boost/assert.hpp>
#include <boost/lexical_cast.hpp>

#include "model.hpp"
#include "../auxiliary/data_types.hpp"

#include "../rapidxml-1.13/rapidxml.hpp"
#include "../rapidxml-1.13/rapidxml_iterators.hpp"
#include "../rapidxml-1.13/rapidxml_print.hpp"
#include "../rapidxml-1.13/rapidxml_utils.hpp"

class Model;

class ModelParser {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Provided with constructor
	Model & model; // Model that will hold the data
	std::istream & input_stream; // Input stream to read from
	const UserOptions & user_options;

	// Created with and for parsing
	rapidxml::xml_document<>  model_xml; // Main parsing node
	std::unique_ptr<char []>  parsed_data; // Data obtained from the stream

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	 * Creates RapidXML document from the input stream.
	 */
	void createDocument() {
		// Copy input data from stream into a string line by line
		std::string input_line, input_data;
		for (int lineno = 1; std::getline(input_stream, input_line); ++lineno) {
			input_data += input_line + "\n";
		}

		// Copy data from String to newly created c-string
		parsed_data.reset(new char[input_data.size() + 1]);
		std::strcpy(parsed_data.get(), input_data.c_str());

		// Setup the parser with the c-string
		try {
			model_xml.parse<0>(parsed_data.get());
		} catch (rapidxml::parse_error e) {
			std::cerr << "Error occured while trying to reconstruct xml document from the stream: " << e.what() << ". \n";
			throw std::runtime_error("rapidxml::xml_document<>.parse(char *) failed");
		}
	}

	/**
	 * Starting from the SPECIE node, the function parses all the INTER tags and reads the data from them.
	 */
	void parseInteractions(const rapidxml::xml_node<> * const specie_node, size_t specie_ID) const {
		// Temporaries
		rapidxml::xml_node<>      *interaction;
		rapidxml::xml_attribute<> *temp_attr;
		// Interaction data
		std::size_t source; std::size_t threshold;

		// Step into INTERACTIONS tag
		interaction = specie_node->first_node("INTERACTIONS");
		if (interaction == 0)
			throw std::runtime_error("Parser did not found the INTERACTIONS node in some SPECIE");

		// Step into first INTER tag
		interaction = interaction->first_node("INTER");
		if (interaction == 0)
			throw std::runtime_error("Parser did not found any INTER node between INTERACTIONS in some SPECIE");

		while (true) { // End when the current node does not have next sibling (all INTER tags were parsed)

			// Get source ID and conver to integer.
			if (interaction->first_attribute("source") == 0)
				throw std::runtime_error("Parser did not found the source attribute in some INTER node");
			else { 
				temp_attr = interaction->first_attribute("source");
				try {
					source = boost::lexical_cast<size_t, char*>(temp_attr->value());
				} catch (boost::bad_lexical_cast e) {
					std::cerr << "Error occured while parsing a interaction. It seems that you have entered non-numerical value as a source attribute. " << e.what() << "\n";
					throw std::runtime_error("boost::lexical_cast<size_t, char*>(temp_attr->value()) failed");
				}
			}

			// Get threshold and conver to integer.
			if (interaction->first_attribute("threshold") == 0)
				throw std::runtime_error("Parser did not found the threshold attribute in some INTER node");
			else { 
				temp_attr = interaction->first_attribute("threshold");
				try {
					threshold = boost::lexical_cast<size_t, char*>(temp_attr->value());
				} catch (boost::bad_lexical_cast e) {
					std::cerr << "Error occured while parsing an interaction. It seems that you have entered non-numerical value as a threshold attribute. " << e.what() << "\n";
					throw std::runtime_error("boost::lexical_cast<size_t, char*>(temp_attr->value()) failed");
				}
			}

			// Add a new interaction to the specified target
			model.addInteraction(source, specie_ID, threshold);

			// Continue stepping into INTER tags while possible
			if (interaction->next_sibling("INTER"))
				interaction = interaction->next_sibling("INTER");
			else break;
		}
	}

	/**
	 * @param mask_string	mask of the active interactions in given regulatory context in the form of a string
	 *
	 * @return Vector of boolean values that represents the input mask.
	 */
	std::vector<bool> getMask(std::string mask_string) const {
		std::vector<bool> mask;
		std::for_each(mask_string.begin(), mask_string.end(),[&mask](char ch) {
			if (ch != '0' && ch !='1') 
				throw std::runtime_error("Error occured while parsing a regulation. It seems that you have entered value other than 0 or 1 in the mask.");
			try {
				mask.push_back(boost::lexical_cast<bool, char>(ch));
			} catch (boost::bad_lexical_cast e) {
				std::cerr << "Error occured while parsing a regulation. " << e.what() << "\n";
				throw std::runtime_error("boost::lexical_cast<size_t, char>(temp_attr->value()) failed");
			}
		});
		return mask;
	}

	/**
	 * Starting from the SPECIE node, the function parses all the REGUL tags and reads the data from them.
	 */
	void parseRegulations(const rapidxml::xml_node<> * const specie_node, size_t specie_ID) const {
		// Temporaries
		rapidxml::xml_node<>      *regulation;
		rapidxml::xml_attribute<> *temp_attr;
		// Interaction data
		std::string mask_string; int target_value;

		// Step into REGULATIONS tag
		regulation = specie_node->first_node("REGULATIONS");
		if (regulation == 0)
			throw std::runtime_error("Parser did not found the REGULATIONS node in some SPECIE");
		// Step into first REGUL tag
		regulation = regulation->first_node("REGUL");
		if (regulation == 0)
			throw std::runtime_error("Parser did not found any REGUL node between REGULATIONS in some SPECIE");

		while (true) { // End when the current node does not have next sibling (all REGUL tags were parsed)

			// Get the mask string.
			if (regulation->first_attribute("mask") == 0)
				throw std::runtime_error("Parser did not found the mask attribute in some REGUL node");
			else { 
				temp_attr = regulation->first_attribute("mask"); 
				mask_string = temp_attr->value();
			}

			// Get max value and conver to integer.
			if (regulation->first_attribute("t_value") == 0)
				throw std::runtime_error("Parser did not found the t_value attribute in some REGUL node");
			else { 
				temp_attr = regulation->first_attribute("t_value");
				try {
					target_value = boost::lexical_cast<int, char*>(temp_attr->value());
				} catch (boost::bad_lexical_cast e) {
					std::cerr << "Error occured while parsing a regulation. It seems that you have entered non-numerical value as a t_value attribute. " << e.what() << "\n";
					throw std::runtime_error("boost::lexical_cast<int, char*>(temp_attr->value()) failed");
				}
			}

			// Add a new regulation to the specified target
			model.addRegulation(specie_ID, std::move(getMask(mask_string)),target_value);

			// Continue stepping into REGUL tags while possible
			if (regulation->next_sibling("REGUL"))
				regulation = regulation->next_sibling("REGUL");
			else break;
		}
	}

	/**
	 * Starting from the STRUCTURE node, the function parses all the SPECIE tags and reads the data from them.
	 */
	void parseSpecies(const rapidxml::xml_node<> * const structure_node) const {
		// Temporaries
		rapidxml::xml_node<>      *specie;
		rapidxml::xml_attribute<> *temp_attr;
		// Specie data
		std::string name; size_t max; size_t basal;

		// Step into first SPECIE tag
		if (structure_node->first_node("SPECIE") == 0)
			throw std::runtime_error("Parser did not found the SPECIE node");
		else specie = structure_node->first_node("SPECIE");

		while (true) { // End when the current node does not have next sibling (all SPECIES tags were parsed)

			// Get name of the specie.
			if (specie->first_attribute("name") == 0)
				throw std::runtime_error("Parser did not found the name attribute in some SPECIE node");
			else { 
				temp_attr = specie->first_attribute("name"); 
				name = temp_attr->value();
			}

			// Get max value and conver to integer.
			if (specie->first_attribute("max") == 0)
				throw std::runtime_error("Parser did not found the max attribute in some SPECIE node");
			else { 
				temp_attr = specie->first_attribute("max");
				try {
					max = boost::lexical_cast<size_t, char*>(temp_attr->value());
				} catch (boost::bad_lexical_cast e) {
					std::cerr << "Error occured while parsing a specie. It seems that you have entered non-numerical value as a max attribute. " << e.what() << "\n";
					throw std::runtime_error("boost::lexical_cast<size_t, char*>(temp_attr->value()) failed");
				}
			}

			// Get basal value and conver to integer.
			if (specie->first_attribute("basal") == 0)
				throw std::runtime_error("Parser did not found the max attribute in some SPECIE node");
			else { 
				temp_attr = specie->first_attribute("basal");
				try {
					basal = boost::lexical_cast<size_t, char*>(temp_attr->value()); 
				} catch (boost::bad_lexical_cast e) {
					std::cerr << "Error occured while parsing a specie. It seems that you have entered non-numerical value as a basal attribute. " << e.what() << "\n";
					throw std::runtime_error("boost::lexical_cast<size_t, char*>(temp_attr->value()) failed");
				}
			}

			// Create a new specie
			size_t specie_ID = model.addSpecie(name, max, basal);

			// Get all the interactions of the specie and store them to the model.
			parseInteractions(specie, specie_ID);

			// Get all the regulations of the specie and store them to the model.
			parseRegulations(specie, specie_ID);

			// Continue stepping into SPECIE tags while possible
			if (specie->next_sibling("SPECIE"))
				specie = specie->next_sibling("SPECIE");
			else break;
		} 
	}

	/**
	 * Starting from the SPECIE node, the function parses all the REGUL tags and reads the data from them.
	 */
	void parseTransitions(const rapidxml::xml_node<> * const state_node, size_t state_ID) const {
		// Temporaries
		rapidxml::xml_node<>      *transition;
		rapidxml::xml_attribute<> *temp_attr;
		// Interaction data
		std::string label_string; std::size_t target_ID;

		// Step into REGULATIONS tag
		transition = state_node->first_node("TRANSITIONS");
		if (transition == 0)
			throw std::runtime_error("Parser did not found the TRANSITIONS node in some STATE");
		// Step into first REGUL tag
		transition = transition->first_node("TRANS");
		if (transition == 0)
			throw std::runtime_error("Parser did not found any TRANS node between TRANSITIONS in some STATE");

		while (true) { // End when the current node does not have next sibling (all REGUL tags were parsed)

			// Get the mask string.
			if (transition->first_attribute("label") == 0)
				throw std::runtime_error("Parser did not found the label attribute in some TRANS node");
			else { 
				temp_attr = transition->first_attribute("label"); 
				label_string = temp_attr->value();
			}

			// Get max value and conver to integer.
			if (transition->first_attribute("target") == 0)
				throw std::runtime_error("Parser did not found the target attribute in some TRANS node");
			else { 
				temp_attr = transition->first_attribute("target");
				try {
					target_ID = boost::lexical_cast<size_t, char*>(temp_attr->value());
				} catch (boost::bad_lexical_cast e) {
					std::cerr << "Error occured while parsing a transition. It seems that you have entered non-numerical value as a target attribute. " << e.what() << "\n";
					throw std::runtime_error("boost::lexical_cast<size_t, char*>(temp_attr->value()) failed");
				}
			}

			// Add a new regulation to the specified target
			model.addConditions(state_ID, target_ID, std::move(label_string));

			// Continue stepping into REGUL tags while possible
			if (transition->next_sibling("TRANS"))
				transition = transition->next_sibling("TRANS");
			else break;
		}
	}

	/**
	 * Starting from the AUTOMATON node, the function parses all the STATE tags and all their TRANSITION tags and reads the data from them.
	 */
	void parseStates(const rapidxml::xml_node<> * const automaton_node) const {
		// Temporaries
		rapidxml::xml_node<>      *state;
		rapidxml::xml_attribute<> *temp_attr;
		// State data
		bool final;

		// Step into first SPECIE tag
		if (automaton_node->first_node("STATE") == 0)
			throw std::runtime_error("Parser did not found the STATE node");
		else state = automaton_node->first_node("STATE");

		while (true) { // End when the current node does not have next sibling (all STATES tags were parsed)

			// Find out whether the state is final
			if (state->first_attribute("final") == 0)
				throw std::runtime_error("Parser did not found the final attribute in some STATE node");
			else { 
				temp_attr = state->first_attribute("final");
				try {
					final = boost::lexical_cast<bool, char*>(temp_attr->value()); 
				} catch (boost::bad_lexical_cast e) {
					std::cerr << "Error occured while parsing a state. It seems that you have entered non-boolean value as a final attribute. " << e.what() << "\n";
					throw std::runtime_error("boost::lexical_cast<bool, char*>(temp_attr->value()) failed");
				}
			}

			// Create a new state
			size_t state_ID = model.addState(final);

			// Get all the transitions of the state and store them to the model.
			parseTransitions(state, state_ID);

			// Continue stepping into STATE tags while possible
			if (state->next_sibling("STATE"))
				state = state->next_sibling("STATE");
			else break;
		} 
	}

	ModelParser(const ModelParser & other);            // Forbidden copy constructor.
	ModelParser& operator=(const ModelParser & other); // Forbidden assignment operator.

public:
	/**
	 * Constructor has to provide references to an input stream to read from and model object to store parsed information.
	 */
	ModelParser(const UserOptions &_user_options, std::istream & _input_stream, Model & _model) : user_options(_user_options), input_stream(_input_stream), model( _model) {}

	/**
	 * Functions that causes the parser to read the input from the stream, parse it and store model information in the model object.
	 *
	 * @return	version of the parsed file.
	 */
	float parseInput() {

		createDocument();
		rapidxml::xml_node<> *current_node;
		float file_version;

		// Step into first MODEL (main) tag
		current_node = model_xml.first_node();
		if (strcmp(current_node->name(), "MODEL") != 0)
			throw std::runtime_error(std::string("Parsed found out that input does not start with the tag <MODEL> but with the <").append(current_node->name()).append("> instead").c_str());

		// Find version number
		if (current_node->first_attribute("ver") == 0)
			throw std::runtime_error("Parser did not found the ver attribute in the MODEL node");
		else {
			try {
				rapidxml::xml_attribute<> *temp_attr = current_node->first_attribute("ver");
				file_version = boost::lexical_cast<float, char*>(temp_attr->value()); 
			} catch (boost::bad_lexical_cast e) {
				std::cerr << "Error occured while parsing the MODEL tag. It seems that you have entered non-float value as a ver attribute. " << e.what() << "\n";
				throw std::runtime_error("boost::lexical_cast<float, char*>(temp_attr->value()) failed");
			}
		}

		// Step into STRUCTURE tag
		if (current_node->first_node("STRUCTURE") == 0)
			throw std::runtime_error("Parser did not found the STRUCTURE node");
		else current_node = current_node->first_node("STRUCTURE");

		// Parse the species of the structure
		parseSpecies(current_node);

		// Step into AUTOMATON tag
		if (current_node->next_sibling("AUTOMATON") == 0)
			throw std::runtime_error("Parser did not found the AUTOMATON node");
		else current_node = current_node->next_sibling("AUTOMATON");

		// Parse the states of the automaton
		parseStates(current_node);

		return file_version;
	};
};

#endif

/*
	Ideas:
There could be single template function to correct search for the tag and one to correctly read the attribute.

*/