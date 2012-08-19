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
#include "automaton_parser.hpp"
#include "network_parser.hpp"
#include "time_series_parser.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ModelParser is an entry point for parsing of a model file. Most of the parsing is done by dependent classes, ModelParser only sets the parsing up for further usage.
/// For the reference on how to create a model see the manual.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ModelParser {
    // Provided with constructor
    Model & model;  ///< Reference to the model object that will be filled
    std::ifstream * input_stream; ///< File to parse the data from

	// Created with and for parsing
    rapidxml::xml_document<>  model_xml; ///< Main parsing node
    std::unique_ptr<char []>  parsed_data; ///< Data obtained from the stream

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PARSING:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   const rapidxml::xml_node<> * initiateParsing() const {
      // Temporaries
      rapidxml::xml_node<> *current_node;
      float file_version;
      std::string unspec;

		// Step into first MODEL (main) tag
		current_node = model_xml.first_node();
		if (strcmp(current_node->name(), "MODEL") != 0)
			throw std::runtime_error(std::string("Parsed found out that input does not start with the tag <MODEL> but with the <")
											 .append(current_node->name()).append("> instead").c_str());
		// Find version number
		XMLHelper::getAttribute(file_version, current_node, "ver");
		XMLHelper::getAttribute(unspec, current_node, "unspec", false);

		// Pass additional information
		model.addAdditionalInformation(file_version);

		return current_node;
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
	
	ModelParser(const ModelParser & other); ///< Forbidden copy constructor.
	ModelParser& operator=(const ModelParser & other); ///< Forbidden assignment operator.

public:
	ModelParser(Model & _model, std::ifstream * _input_stream) : model( _model), input_stream(_input_stream) {} ///< Simple constructor, passes references

	/**
	 * Functions that causes the parser to read the input from the stream, parse it and store model information in the model object.
	 */
	void parseInput() {
		createDocument();

		auto model_node = initiateParsing();

		NetworkParser network_parser(model);
		network_parser.parse(model_node);

      // Find property tag and control its uniqueness
      if (model_node->first_node("AUTOMATON")) {
         AutomatonParser automaton_parser(model);
         automaton_parser.parse(model_node);
         if ((model_node->first_node("AUTOMATON"))->next_sibling("AUTOMATON") || model_node->first_node("SERIES"))
            throw std::invalid_argument("multiple occurences of property specification (AUTOMATON or SERIES tag)");
      }
      else if (model_node->first_node("SERIES")) {
         TimeSeriesParser series_parser(model);
         series_parser.parse(model_node);
         if ((model_node->first_node("SERIES"))->next_sibling("SERIES") || model_node->first_node("AUTOMATON"))
            throw std::invalid_argument("multiple occurences of property specification (AUTOMATON or SERIES tag)");         
         user_options.time_series = true;
      }
      else
         throw std::invalid_argument("AUTOMATON or SERIES tag missing - no property to be tested found.");
   }
};

#endif // PARSYBONE_MODEL_PARSER_INCLUDED
