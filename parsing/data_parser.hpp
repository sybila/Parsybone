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
#include "../model/model_translators.hpp"
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
	
public:
	/**
	 * Functions that causes the parser to read the input from the stream, parse it and store model information in the model object.
	 */
   Model parseNetwork(ifstream * input_stream) {
      createDocument(input_stream);
		auto model_node = initiateParsing();

      Model model;
      NetworkParser::parseNetwork(model_node, model);
      NetworkParser::parseConstraints(model_node, model);
      ParameterParser::parse(model_node, model);

      input_stream->clear();
      input_stream->seekg(0, ios::beg);
      return model;
   }

   /**
    * @brief parseProperties create a property automaton
    * @param input_stream
    */
   PropertyAutomaton parseProperty(ifstream * input_stream) {
      createDocument(input_stream);
      auto model_node = initiateParsing();

      PropertyAutomaton property;

      for (rapidxml::xml_node<> * automaton = XMLHelper::getChildNode(model_node, "AUTOMATON", false); automaton; automaton = automaton->next_sibling("AUTOMATON") ) {
         property = AutomatonParser::parse(automaton, "automaton");
      }
      for (rapidxml::xml_node<> * series = XMLHelper::getChildNode(model_node, "SERIES", false); series; series = series->next_sibling("SERIES") ) {
         property = TimeSeriesParser::parse(series, "series");
         user_options.time_series = true;
      }

      if (property.getAutomatonName().empty())
         throw runtime_error("No property found");

      input_stream->clear();
      input_stream->seekg(0, ios::beg);
      return property;
   }
};

#endif // PARSYBONE_DATA_PARSER_INCLUDED
