/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_NETWORK_PARSER_INCLUDED
#define PARSYBONE_NETWORK_PARSER_INCLUDED

#include "../auxiliary/data_types.hpp"
#include "translator.hpp"
#include "xml_helper.hpp"
#include "model.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// This object is responsible for parsing and translation of data related to the GRN.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class NetworkParser {
	Model & model; ///< Reference to the model object that will be filled

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PARSING:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Starting from the SPECIE node, the function parses all the INTER tags and reads the data from them.
	 */
	void parseInteractions(const rapidxml::xml_node<> * const specie_node, size_t specie_ID) const {
		rapidxml::xml_node<> *interaction;
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	NetworkParser(const NetworkParser & other); ///< Forbidden copy constructor.
	NetworkParser& operator=(const NetworkParser & other); ///< Forbidden assignment operator.

public:
	NetworkParser(Model & _model) : model(_model) { } ///< Simple constructor, passes references

	void parse(const rapidxml::xml_node<> * const current_node) {
		// Parse Kripke Structure
		output_streamer.output(verbose_str, "Started reading of the Kripke structure.");
		parseSpecies(XMLHelper::getChildNode(current_node, "STRUCTURE"));
	}
};

#endif // PARSYBONE_NETWORK_PARSER_INCLUDED
