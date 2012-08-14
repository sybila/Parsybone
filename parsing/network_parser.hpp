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
#include "../auxiliary/common_functions.hpp"
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
	 * In a current interaction get source of that interaction, if possible.
	 */
	const SpecieID getSourceID(const rapidxml::xml_node<> * const regulation, const SpecieID target_ID ) const {
		std::string source; SpecieID source_ID;

		XMLHelper::getAttribute(source, regulation, "source");

		try { // Try direct translation
			source_ID = boost::lexical_cast<StateID, std::string>(source);
		}
		catch (boost::bad_lexical_cast) { // Try lookup by name
			source_ID = model.findID(source);
		}

		if (source_ID >= model.getSpeciesCount())
			throw std::invalid_argument(std::string("ID of a regulation of the specie ").append(toString(target_ID)).append(" is incorrect."));

		return source_ID;
	}

	/**
	 * Obtain a treshold of a current regulation and check if it is correct and unique.
	 */
	const std::size_t getThreshold(const rapidxml::xml_node<> * const regulation, const SpecieID target_ID, const SpecieID source_ID ) const {
		std::size_t threshold;
		// Try to find a threshold, if not present, set to 1
		if(!XMLHelper::getAttribute(threshold, regulation, "threshold"))
			threshold = 1;
		else if (threshold > model.getMax(source_ID) || threshold == 0) // Control the value
			throw std::invalid_argument(std::string("threshold of a regulation of specie ").append(toString(source_ID))
												 .append(" is incorrect (bigger than maximal level of the source or equal to 0)."));
		// Test uniqueness
		auto interactions = model.getInteractions(target_ID);
		forEach(interactions, [threshold,source_ID](Model::Interaction & interaction) {
			if (threshold == interaction.threshold && source_ID == interaction.source)
				throw std::invalid_argument(std::string("multiple definition of a regulation of a specie ").append(toString(source_ID)));
		});

		return threshold;
	}

	/**
	 * Starting from the SPECIE node, the function parses all the REGUL tags and reads the data from them.
	 * If not provided, attributes are defaulted - threshold to 1, label to none_cons, observable to false
	 */
	void parseInteractions(const rapidxml::xml_node<> * const specie_node, size_t specie_ID) const {
		// Interaction data
		std::string label; EdgeConstrain constrain; bool observable;

		// Step into a first REGUL tag
		rapidxml::xml_node<>* interaction = XMLHelper::getChildNode(specie_node, "REGUL");

		while (true) { // End when the current node does not have next sibling (all REGUL tags were parsed)
			auto source_ID = getSourceID(interaction, specie_ID);
			auto threshold = getThreshold(interaction, specie_ID, source_ID);
			if (!XMLHelper::getAttribute(label, interaction, "label", false))
				label = "";
			constrain = Translator::readConstrain(label);
			if(!XMLHelper::getAttribute(observable, interaction, "observ", false))
				observable = false;

			// Add a new interaction to the specified target
			model.addInteraction(source_ID, specie_ID, threshold, constrain, observable);

			// Continue stepping into REGUL tags while possible
			if (interaction->next_sibling("REGUL"))
				interaction = interaction->next_sibling("REGUL");
			else break;
		}
	}

	/**
	 * Starting from the SPECIE node, the function parses all the PARAM tags and reads the data from them.
	 */
	void parseRegulations(const rapidxml::xml_node<> * const specie_node, size_t specie_ID) const {
		// Interaction data
		std::string mask_string; int target_value;

		// Step into first REGUL tag
		rapidxml::xml_node<>* regulation = XMLHelper::getChildNode(specie_node, "PARAM");

		while (true) { // End when the current node does not have next sibling (all PARAM tags were parsed)
			// Get the mask string.
			XMLHelper::getAttribute(mask_string, regulation, "mask");
			// Get max value and conver to integer.
			XMLHelper::getAttribute(target_value, regulation, "t_value");

			// Add a new regulation to the specified target
			model.addRegulation(specie_ID, std::move(Translator::getMask(mask_string)),target_value);

			// Continue stepping into REGUL tags while possible
			if (regulation->next_sibling("PARAM"))
				regulation = regulation->next_sibling("PARAM");
			else break;
		}
	}

	/**
	 * Starting from the STRUCTURE node, the function parses all the SPECIE tags and reads the data from them.
	 */
	void secondParse(const rapidxml::xml_node<> * const structure_node) const {
		// Step into first SPECIE tag
		rapidxml::xml_node<> *specie = XMLHelper::getChildNode(structure_node, "SPECIE");

		for (StateID ID = 0; ID < model.getSpeciesCount(); ID++) {
			// Get all the interactions of the specie and store them to the model.
			parseInteractions(specie, ID);
			// Get all the regulations of the specie and store them to the model.
			parseRegulations(specie, ID);

			// Step into the next specie (by previous parse their number and order is guaranteed)
			specie = specie->next_sibling("SPECIE");
		}
	}

	/**
	 * Starting from the STRUCTURE node, the function parses all the SPECIE tags and reads the data from them.
	 * If not provided, attributes are defaulted - name is equal to ordinal number starting from 0, max to 1 and basal value to 0.
	 */
	void firstParse(const rapidxml::xml_node<> * const structure_node) const {
		// Specie data
		std::string name; size_t max; size_t basal;

		// Step into first SPECIE tag
		rapidxml::xml_node<> *specie = XMLHelper::getChildNode(structure_node, "SPECIE");

		SpecieID ID = 0;
		while (true) { // End when the current node does not have next sibling (all SPECIES tags were parsed)
			// Get name of the specie.
			if (!XMLHelper::getAttribute(name, specie, "name", false))
				name = toString(ID);
			// Get max value and conver to integer.
			if (!XMLHelper::getAttribute(max, specie, "max", false))
				max = 1;
			// Get basal value and conver to integer.
			if(!XMLHelper::getAttribute(basal, specie, "basal", false))
				basal = 0;

			// Create a new specie
			model.addSpecie(name, max, basal);

			// Continue stepping into SPECIE tags while possible
			if (specie->next_sibling("SPECIE"))
				specie = specie->next_sibling("SPECIE");
			else break;
			ID++;
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	NetworkParser(const NetworkParser & other); ///< Forbidden copy constructor.
	NetworkParser& operator=(const NetworkParser & other); ///< Forbidden assignment operator.

public:
	NetworkParser(Model & _model) : model(_model) { } ///< Simple constructor, passes references

	void parse(const rapidxml::xml_node<> * const model_node) {
		// Parse Kripke Structure
		output_streamer.output(verbose_str, "Started reading of the Kripke structure.");
		// Create the species
		firstParse(XMLHelper::getChildNode(model_node, "STRUCTURE"));
		// Add regulatory logic
		secondParse(XMLHelper::getChildNode(model_node, "STRUCTURE"));
	}
};

#endif // PARSYBONE_NETWORK_PARSER_INCLUDED
