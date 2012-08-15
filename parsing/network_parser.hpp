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
#include "formulae_parser.hpp"
#include "translator.hpp"
#include "xml_helper.hpp"
#include "model.hpp"

#include "boost/algorithm/string.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// This object is responsible for parsing and translation of data related to the GRN.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class NetworkParser {
	Model & model; ///< Reference to the model object that will be filled

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PARSING:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * In a current regulation get source of that regulation, if possible.
	 */
	const SpecieID getSourceID(const rapidxml::xml_node<> * const regulation, const SpecieID target_ID ) const {
		std::string source; SpecieID source_ID;

		XMLHelper::getAttribute(source, regulation, "source");

		source_ID = model.findID(source);

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
		auto regulations = model.getRegulations(target_ID);
		forEach(regulations, [threshold,source_ID](Model::Regulation & regulation) {
			if (threshold == regulation.threshold && source_ID == regulation.source)
				throw std::invalid_argument(std::string("multiple definition of a regulation of a specie ").append(toString(source_ID)));
		});

		return threshold;
	}

	/**
	 * @param uspec_type	what to do with usnpecified regulations
	 *
	 * @return	enumeration item with given specification
	 */
	static const UnspecifiedParameters getUnspecType(std::string unspec_type) {
		if      (unspec_type.compare("error"))
			return error_reg;
		else if (unspec_type.compare("basal"))
			return basal_reg;
		else if (unspec_type.compare("param"))
			return param_reg;
		else
			throw std::runtime_error("Wrong value given as an uspec attribute.");
	}

	/**
	 * Obtain an information about how unspecified kinetic parameters should be handled
	 */
	const UnspecifiedParameters getUnspecified(const rapidxml::xml_node<> * const specie_node) const {
		std::string unspec_str; UnspecifiedParameters unspec;
		// Try to get the value, otherwise use param_reg
		if (XMLHelper::getAttribute(unspec_str, specie_node, "unspec_str", false))
			unspec = getUnspecType(unspec_str);
		else
			unspec = param_reg;
		return unspec;
	}

	/**
	 * Starting from the SPECIE node, the function parses all the REGUL tags and reads the data from them.
	 * If not provided, attributes are defaulted - threshold to 1, label to none_cons, observable to false
	 */
	void parseRegulations(const rapidxml::xml_node<> * const specie_node, size_t specie_ID) const {
		// Regulation data
		std::string label; EdgeConstrain constrain; bool observable;

		// Step into a first REGUL tag
		rapidxml::xml_node<>* regulation = XMLHelper::getChildNode(specie_node, "REGUL");

		while (true) { // End when the current node does not have next sibling (all REGUL tags were parsed)
			auto source_ID = getSourceID(regulation, specie_ID);
			auto threshold = getThreshold(regulation, specie_ID, source_ID);
			if (!XMLHelper::getAttribute(label, regulation, "label", false))
				label = "";
			constrain = Translator::readConstrain(label);
			if(!XMLHelper::getAttribute(observable, regulation, "observ", false))
				observable = false;

			// Add a new regulation to the specified target
			model.addRegulation(source_ID, specie_ID, threshold, constrain, observable);

			// Continue stepping into REGUL tags while possible
			if (regulation->next_sibling("REGUL"))
				regulation = regulation->next_sibling("REGUL");
			else break;
		}
	}

	void fillFromContext(const std::string context, std::set<std::vector<bool> > & specified, size_t specie_ID, int target_value) const {
		// Obtain strings of the sources
		std::vector<std::string> sources;
		if (!context.empty()) try {
			boost::split(sources, context, boost::is_any_of(","));
		} catch (std::exception & e) {
			output_streamer.output(error_str, std::string("Error occured while parsing a context. ").append(e.what()));
			throw std::runtime_error("boost::split(sources, context, boost::is_any_of(\",\")) failed");
		}

		// Control existence of the
		forEach(sources, [&](std::string & source) {
			if (model.findID(source) > model.getSpeciesCount())
			throw std::invalid_argument(std::string("One of the regulators of the specie ").append(toString(specie_ID)).append(" was not found in the specie list"));
		});

		// Create a mask by comparison of source strings against regulations sources of the specie
		std::vector<bool> mask;
		auto regulations = model.getRegulations(specie_ID);
		for (std::size_t regul_num = 0; regul_num < regulations.size(); regul_num++) {
			mask.push_back( std::find(sources.begin(), sources.end(), toString(regulations[regul_num].source)) == sources.end() );
		}

		// Add a new regulation to the specified target
		if (!specified.insert(mask).second) {
			throw std::invalid_argument(std::string("Context redefinition found for the specie ").append(toString(specie_ID)));
		}
		model.addParameter(specie_ID, mask, target_value);
	}

	void fillFromLogic(const std::string logic, size_t specie_ID) const {
		std::vector<bool> tested(model.getRegulations(specie_ID).size(), false);
		std::vector<bool> top(model.getRegulations(specie_ID).size(), true);
		do {
			std::map<std::string, bool> valuation;
			for (std::size_t regul_num = 0; regul_num < tested.size(); regul_num++) {
				StateID source_ID = (model.getRegulations(specie_ID))[regul_num].source;
				valuation.insert(std::make_pair(toString(source_ID), tested[regul_num]));
				valuation.insert(std::make_pair(model.getName(source_ID), tested[regul_num]));
			}

			model.addParameter(specie_ID, tested, FormulaeParser::resolve(valuation, logic));

			if (top == tested)
				return;
			// Iterate
			for (auto specie_it = tested.begin(); specie_it != tested.end(); specie_it++) {
				if (*specie_it) {
					*specie_it = false;
				}
				else {
					*specie_it = true;
					break;
				}
			}
		} while (true);
	}

	void addUnspecified(std::set<std::vector<bool> > & specified, size_t specie_ID, UnspecifiedParameters unspec) const {
		std::vector<bool> tested(model.getRegulations(specie_ID).size(), false);
		std::vector<bool> top(model.getRegulations(specie_ID).size(), true);
		do {
			if (specified.insert(tested).second) {
				switch (unspec) {
					case basal_reg:
						model.addParameter(specie_ID, tested, model.getBasal(specie_ID));
						break;

					case param_reg:
						model.addParameter(specie_ID, tested, -1);
						break;

					case error_reg:
						throw std::runtime_error(std::string("Some required parameter specification is missing for the specie ").append(toString(specie_ID)));
						break;
				}
			}

			if (top == tested)
				return;

			// Iterate
			for (auto specie_it = tested.begin(); specie_it != tested.end(); specie_it++) {
				if (*specie_it) {
					*specie_it = false;
				}
				else {
					*specie_it = true;
					break;
				}
			}
		} while (true);
	}

	/**
	 * Starting from the SPECIE node, the function parses all the PARAM tags and reads the data from them.
	 */
	void parseParameters(const rapidxml::xml_node<> * const specie_node, size_t specie_ID) const {
		// Parameters data data
		std::string spec_string; int target_value;
		auto unspec = getUnspecified(specie_node);
		std::set<std::vector<bool> > specified;

		// Step into first REGUL tag
		rapidxml::xml_node<>* regulation = XMLHelper::getChildNode(specie_node, "PARAM");

		while (true) { // End when the current node does not have next sibling (all PARAM tags were parsed)
			// Get the mask string.
			if ( XMLHelper::getAttribute(spec_string, regulation, "context", false) ) {
				if (!XMLHelper::getAttribute(target_value, regulation, "value"))
					target_value = - 1;
				fillFromContext(spec_string, specified, specie_ID, target_value);
			} else if ( XMLHelper::getAttribute(spec_string, regulation, "logic", false) ) {
				fillFromLogic(spec_string, specie_ID);
				return;
			}
			else {
				throw std::invalid_argument(std::string("Not context nor logic specified for the parameters in the specie ").append(toString(specie_ID)));
			}

			// Continue stepping into REGUL tags while possible
			if (regulation->next_sibling("PARAM"))
				regulation = regulation->next_sibling("PARAM");
			else break;
		}

		addUnspecified(specified, specie_ID, unspec);
	}

	/**
	 * Starting from the STRUCTURE node, the function parses all the SPECIE tags and reads the data from them.
	 */
	void secondParse(const rapidxml::xml_node<> * const structure_node) const {
		// Step into first SPECIE tag
		rapidxml::xml_node<> *specie = XMLHelper::getChildNode(structure_node, "SPECIE");

		for (StateID ID = 0; ID < model.getSpeciesCount(); ID++) {
			// Get all the regulations of the specie and store them to the model.
			parseRegulations(specie, ID);
			// Get all the parameters of the specie and store them to the model.
			parseParameters(specie, ID);

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

	/**
	 * Main parsing function. It expects a pointer to inside of a MODEL node.
	 */
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
