/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_MODEL_PARSER_INCLUDED
#define PARSYBONE_MODEL_PARSER_INCLUDED

#include "../auxiliary/output_streamer.hpp"
#include "model.hpp"
#include "translator.hpp"
#include "xml_helper.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ModelParser parses provided input stream and stores data in the provided Model object.
/// Data are mostly kept in the raw form, same as in the model file.
/// Most of the possible mistakes and typos would case an exception and failure of the program.
/// Only syntactic correcntess is checked. Wrong semantics would pass through here!
/// There is only single public functions (apart from the constructor) - parseInput(), that performs the whole process.
/// The functions are rather long, but their meaning is quite straithforward and repetitive. Most of the code are controls of correctness.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ModelParser {
    // Provided with constructor
    Model & model; ///< Model that will hold the data
    std::ifstream * input_stream; ///< File to parse the data from

	// Created with and for parsing
    rapidxml::xml_document<>  model_xml; ///< Main parsing node
    std::unique_ptr<char []>  parsed_data; ///< Data obtained from the stream

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PARSING:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Starting from the SPECIE node, the function parses all the INTER tags and reads the data from them.
	 */
	void parseInteractions(const rapidxml::xml_node<> * const specie_node, size_t specie_ID) const {
		rapidxml::xml_node<>      *interaction;
		// Interaction data
		std::size_t source = ~0;
		std::size_t threshold = ~0;
		std::string label = "";
		EdgeConstrain constrain = none_cons;
		bool observable = false;

		// Step into INTERACTIONS tag
		interaction = XMLHelper::getChildNode(specie_node, "INTERACTIONS");

		// Step into first INTER tag
		interaction = XMLHelper::getChildNode(interaction, "INTER");

		while (true) { // End when the current node does not have next sibling (all INTER tags were parsed)
			// Get source ID and conver to integer.
			XMLHelper::getAttribute(source, interaction, "source");
			// Get threshold and conver to integer.
			XMLHelper::getAttribute(threshold, interaction, "threshold");
			// Get an edge label
			if (!XMLHelper::getAttribute(label, interaction, "label", false))
				label = "";
			// Convert label into an edge constrain
			constrain = Translator::readConstrain(label);
			// Get observable attribute
			XMLHelper::getAttribute(observable, interaction, "observ", false);

			// Add a new interaction to the specified target
			model.addInteraction(source, specie_ID, threshold, constrain, observable);

			// Continue stepping into INTER tags while possible
			if (interaction->next_sibling("INTER"))
				interaction = interaction->next_sibling("INTER");
			else break;
		}
	}

	/**
	 * Starting from the SPECIE node, the function parses all the REGUL tags and reads the data from them.
	 */
	void parseRegulations(const rapidxml::xml_node<> * const specie_node, size_t specie_ID) const {
		rapidxml::xml_node<>      *regulation;
		// Interaction data
		std::string mask_string; int target_value;

		// Step into REGULATIONS tag
		regulation = XMLHelper::getChildNode(specie_node, "REGULATIONS");
		// Step into first REGUL tag
		regulation = XMLHelper::getChildNode(regulation, "REGUL");

		while (true) { // End when the current node does not have next sibling (all REGUL tags were parsed)
			// Get the mask string.
			XMLHelper::getAttribute(mask_string, regulation, "mask");
			// Get max value and conver to integer.
			XMLHelper::getAttribute(target_value, regulation, "t_value");

			// Add a new regulation to the specified target
			model.addRegulation(specie_ID, std::move(Translator::getMask(mask_string)),target_value);

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
		rapidxml::xml_node<>      *specie;
		// Specie data
		std::string name; size_t max; size_t basal;

		// Step into first SPECIE tag
		specie = XMLHelper::getChildNode(structure_node, "SPECIE");

		while (true) { // End when the current node does not have next sibling (all SPECIES tags were parsed)
			// Get name of the specie.
			XMLHelper::getAttribute(name, specie, "name");
			// Get max value and conver to integer.
			XMLHelper::getAttribute(max, specie, "max");
			// Get basal value and conver to integer.
			XMLHelper::getAttribute(basal, specie, "basal");

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
	void parseTransitions(const rapidxml::xml_node<> * const state_node, StateID source_ID) const {
		rapidxml::xml_node<>      *transition;
		// Interaction data
		std::string label_string; std::size_t target_ID;

		// Step into REGULATIONS tag
		transition = XMLHelper::getChildNode(state_node, "TRANSITIONS");
		// Step into first REGUL tag
		transition = XMLHelper::getChildNode(transition, "TRANS");

		while (true) { // End when the current node does not have next sibling (all REGUL tags were parsed)
			// Get the mask string.
			XMLHelper::getAttribute(label_string, transition, "label");
			// Get max value and conver to integer.
			XMLHelper::getAttribute(target_ID, transition, "target");

			// Add a new regulation to the specified target
			model.addConditions(source_ID, target_ID, std::move(label_string));

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
		rapidxml::xml_node<>      *state;
		// State data
		bool final;

		// Step into first SPECIE tag
		state = XMLHelper::getChildNode(automaton_node, "STATE");

		while (true) { // End when the current node does not have next sibling (all STATES tags were parsed)
			// Find out whether the state is final
			XMLHelper::getAttribute(final, state, "final");

			// Create a new state
			StateID ID = model.addState(final);

			// Get all the transitions of the state and store them to the model.
			parseTransitions(state, ID);

			// Continue stepping into STATE tags while possible
			if (state->next_sibling("STATE"))
				state = state->next_sibling("STATE");
			else break;
		} 
	}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Creates RapidXML document from the input stream.
	 */
	void createDocument() {
		// Copy input data from stream into a string line by line
		std::string input_line, input_data;
        for (int lineno = 1; std::getline(*input_stream, input_line); ++lineno) {
			input_data += input_line + "\n";
		}

		// Copy data from String to newly created c-string
		parsed_data.reset(new char[input_data.size() + 1]);
		std::strcpy(parsed_data.get(), input_data.c_str());

		// Setup the parser with the c-string
		try {
			model_xml.parse<0>(parsed_data.get());
		} catch (rapidxml::parse_error e) {
			output_streamer.output(error_str, std::string("Error occured while trying to reconstruct xml document from the stream: ").append(e.what()));
			throw std::runtime_error("rapidxml::xml_document<>.parse(char *) failed");
		}
	}
	
	ModelParser(const ModelParser & other);            // Forbidden copy constructor.
	ModelParser& operator=(const ModelParser & other); // Forbidden assignment operator.

public:
	/**
	 * Constructor has to provide references to an input stream to read from and model object to store parsed information.
	 */
    ModelParser(Model & _model, std::ifstream * _input_stream) : model( _model), input_stream(_input_stream) {}

	/**
	 * Functions that causes the parser to read the input from the stream, parse it and store model information in the model object.
	 *
	 * @return	version of the parsed file.
	 */
	void parseInput() {
		// Temporaries
		rapidxml::xml_node<> *current_node;
		float file_version;
		std::string unspecified_regulations;

		// Create the parser
		createDocument();

		// Step into first MODEL (main) tag
		current_node = model_xml.first_node();
		if (strcmp(current_node->name(), "MODEL") != 0)
			throw std::runtime_error(std::string("Parsed found out that input does not start with the tag <MODEL> but with the <")
			                         .append(current_node->name()).append("> instead").c_str());
		// Find version number
		XMLHelper::getAttribute(file_version, current_node, "ver");

		// Parse Kripke Structure
		output_streamer.output(verbose_str, "Started reading of the Kripke structure.");
		current_node = XMLHelper::getChildNode(current_node, "STRUCTURE");
		XMLHelper::getAttribute(unspecified_regulations, current_node, "unspec");
		parseSpecies(current_node);

		// Parse Buchi Automaton
		output_streamer.output(verbose_str, "Started reading of the Buchi automaton.");
		current_node = XMLHelper::getSiblingNode(current_node, "AUTOMATON");
		parseStates(current_node);

		// Pass additional information
		model.addAdditionalInformation(Translator::getUnspecType(unspecified_regulations), file_version);
    }
};

#endif
