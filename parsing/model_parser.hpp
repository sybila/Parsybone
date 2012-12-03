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
/// \brief Starting point of the model parsing.
///
/// ModelParser is an entry point for parsing of a model file. Most of the parsing is done by dependent classes, ModelParser only sets the parsing up for further usage.
/// For the reference on how to create a model see the manual/README.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ModelParser {
   // Provided with constructor
   Model & model; ///< Reference to the model object that will be filled
   ifstream * input_stream; ///< File to parse the data from

	// Created with and for parsing
   rapidxml::xml_document<>  model_xml; ///< Main parsing node
   unique_ptr<char []>  parsed_data; ///< Data obtained from the stream

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PARSING:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   const rapidxml::xml_node<> * initiateParsing() const {
      // Temporaries
      rapidxml::xml_node<> *current_node;

		// Step into first MODEL (main) tag
		current_node = model_xml.first_node();
		if (strcmp(current_node->name(), "MODEL") != 0)
			throw runtime_error(string("Parsed found out that input does not start with the tag <MODEL> but with the <")
											 .append(current_node->name()).append("> instead").c_str());

		/* // Find a version number
		XMLHelper::getAttribute(file_version, current_node, "ver");
      if (file_version < program_version) {
         output_streamer.output(verbose_str, string("Version of the file (").append(toString(file_version)).append(") is lower than version of the program (")
                        .append(toString(program_version)).append("). Some data can be missing in the file."));
      } else if (file_version > program_version) {
         output_streamer.output(verbose_str, string("Version of the file (").append(toString(file_version)).append(") is higher than version of the program (")
                        .append(toString(program_version)).append("). Some functionality  of the program can be missing."));
      }
      // Pass additional information (version number)
      model.addAdditionalInformation(file_version); */

		return current_node;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION METHODS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Creates RapidXML document from the input stream.
	 */
	void createDocument() {
		// Copy input data from stream into a string line by line
		string input_line, input_data;
         for (int lineno = 1; getline(*input_stream, input_line); ++lineno) {
			input_data += input_line + "\n";
		}

		// Copy data from String to newly created c-string
		parsed_data.reset(new char[input_data.size() + 1]);
		strcpy(parsed_data.get(), input_data.c_str());

		// Setup the parser with the c-string
		try {
			model_xml.parse<0>(parsed_data.get());
		} catch (rapidxml::parse_error e) {
         output_streamer.output(error_str, string("Error occured while trying to reconstruct xml document from the stream: ").append(e.what()).append("."));
         throw runtime_error("Rapidxml::xml_document<>.parse(char *) failed");
		}
	}
	
	ModelParser(const ModelParser & other); ///< Forbidden copy constructor.
	ModelParser& operator=(const ModelParser & other); ///< Forbidden assignment operator.

public:
	ModelParser(Model & _model, ifstream * _input_stream) : model( _model), input_stream(_input_stream) {} ///< Simple constructor, passes references

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
            throw invalid_argument("Multiple occurences of property specification (AUTOMATON or SERIES tag)");
         if (user_options.analysis())
            throw invalid_argument("Advancet analysis methods not available for the general LTL propery, wrong usage of argument -r, -w or -W");
      }
      else if (model_node->first_node("SERIES")) {
         TimeSeriesParser series_parser(model);
         series_parser.parse(model_node);
         if ((model_node->first_node("SERIES"))->next_sibling("SERIES") || model_node->first_node("AUTOMATON"))
            throw invalid_argument("Multiple occurences of property specification (AUTOMATON or SERIES tag)");
         user_options.time_series = true;
      }
      else
         throw invalid_argument("AUTOMATON or SERIES tag missing - no property to be tested found");
   }
};

#endif // PARSYBONE_MODEL_PARSER_INCLUDED
