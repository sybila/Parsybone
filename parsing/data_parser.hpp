/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_DATA_PARSER_INCLUDED
#define PARSYBONE_DATA_PARSER_INCLUDED

#include "../auxiliary/output_streamer.hpp"
#include "automaton_parser.hpp"
#include "network_parser.hpp"
#include "time_series_parser.hpp"
#include "parameter_parser.hpp"
#include "parameter_reader.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Starting point of the model parsing.
///
/// ModelParser is an entry point for parsing of a model file. Most of the parsing is done by dependent classes, ModelParser only sets the parsing up for further usage.
/// For the reference on how to create a model see the manual/README.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class DataParser {
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
         throw runtime_error("Parsed found out that input does not start with the tag <MODEL> but with the <" + string(current_node->name()) + "> instead");

		return current_node;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION METHODS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Creates RapidXML document from the input stream.
	 */
   void createDocument(ifstream * input_stream) {
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
	
   DataParser(const DataParser & other) = delete; ///< Forbidden copy constructor.
   DataParser& operator=(const DataParser & other)  = delete; ///< Forbidden assignment operator.

public:
   DataParser() = default;

	/**
	 * Functions that causes the parser to read the input from the stream, parse it and store model information in the model object.
	 */
   Model parseNetwork(ifstream * input_stream) {
      createDocument(input_stream);
		auto model_node = initiateParsing();

      Model model;

		NetworkParser network_parser(model);
		network_parser.parse(model_node);
      // Add levels to the regulations.
      ReadingHelper::fillActivationLevels(model);
      // Compute parameter values.
      ReadingHelper::fillParameters(model);
      // Replace explicitly defined parameters.
      ParameterParser param_parser;
      auto specifications = param_parser.parse(model_node);
      ParameterReader param_reader;
      param_reader.computeParams(specifications, model);

      input_stream->clear();
      input_stream->seekg(0, ios::beg);
      return model;
   }

   PropertyAutomaton parseProperty(ifstream * input_stream) {
      createDocument(input_stream);
      auto model_node = initiateParsing();

      PropertyAutomaton property("A1");

      // Find property tag and control its uniqueness
      if (model_node->first_node("AUTOMATON")) {
         AutomatonParser automaton_parser;
         property = automaton_parser.parse(model_node);
         if ((model_node->first_node("AUTOMATON"))->next_sibling("AUTOMATON") || model_node->first_node("SERIES"))
            throw invalid_argument("Multiple occurences of property specification (AUTOMATON or SERIES tag)");
         if (user_options.analysis())
            throw invalid_argument("Advancet analysis methods not available for the general LTL propery, wrong usage of argument -r, -w or -W");
      }
      else if (model_node->first_node("SERIES")) {
         TimeSeriesParser series_parser;
         property = series_parser.parse(model_node);
         if ((model_node->first_node("SERIES"))->next_sibling("SERIES") || model_node->first_node("AUTOMATON"))
            throw invalid_argument("Multiple occurences of property specification (AUTOMATON or SERIES tag)");
         user_options.time_series = true;
      }
      else
         throw invalid_argument("AUTOMATON or SERIES tag missing - no property to be tested found");

      input_stream->clear();
      input_stream->seekg(0, ios::beg);
      return property;
   }
};

#endif // PARSYBONE_DATA_PARSER_INCLUDED
