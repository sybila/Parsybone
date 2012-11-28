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
#include "formulae_parser.hpp"
#include "xml_helper.hpp"
#include "model.hpp"

#include "boost/algorithm/string.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Class for parsing of the regulatory network.
///
/// This object is responsible for parsing and translation of data related to the GRN.
/// Most of the possible semantics mistakes are under control and cause exceptions.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class NetworkParser {
   Model & model; ///< Reference to the model object that will be filled.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TRANSLATORS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * In a current regulation get source of that regulation, if possible.
	 */
	SpecieID getSourceID(const rapidxml::xml_node<> * const regulation, const SpecieID target_ID ) const {
		string source; SpecieID source_ID;

		// Find the source and check correctness
		XMLHelper::getAttribute(source, regulation, "source");
		source_ID = model.findID(source);
		if (source_ID >= model.getSpeciesCount())
			throw invalid_argument("ID of a regulation of the specie " + toString(model.getName(target_ID)) + " is incorrect");

		return source_ID;
	}

	/**
	 * Obtain a treshold of a current regulation and check if it is correct and unique.
	 */
	size_t getThreshold(const rapidxml::xml_node<> * const regulation, const SpecieID target_ID, const SpecieID source_ID ) const {
		size_t threshold;

		// Try to find a threshold, if not present, set to 1
		if(!XMLHelper::getAttribute(threshold, regulation, "threshold", false))
			threshold = 1;
		else if (threshold > model.getMax(source_ID) || threshold == 0) // Control the value
			throw invalid_argument("the threshold' value " + toString(threshold) + " is not within the range of the regulator " + toString(model.getName(source_ID)));

		// Test uniqueness of this combination (source, threshold)
		auto regulations = model.getRegulations(target_ID);
		forEach(regulations, [&,threshold,source_ID](Model::Regulation & regulation) {
			if (threshold == regulation.threshold && source_ID == regulation.source)
				throw invalid_argument("multiple definition of a regulation of a specie " + toString(model.getName(source_ID)));
		});

		return threshold;
	}

	/**
	 * @param uspec_type	what to do with usnpecified regulations
	 *
	 * @return	enumeration item with given specification
	 */
	static UnspecifiedParameters getUnspecType(string unspec_type) {
      if      (unspec_type.compare("error") == 0)
			return error_reg;
      else if (unspec_type.compare("basal") == 0)
			return basal_reg;
      else if (unspec_type.compare("param") == 0)
			return param_reg;
		else
			throw runtime_error("wrong value given as an uspec attribute");
	}

	/**
	 * Obtain an information about how unspecified kinetic parameters should be handled.
    * Value is not mandatory, if missing, uses param_reg.
	 */
	UnspecifiedParameters getUnspecified(const rapidxml::xml_node<> * const specie_node) const {
		string unspec_str; UnspecifiedParameters unspec;
		// Try to get the value, otherwise use param_reg
      if (XMLHelper::getAttribute(unspec_str, specie_node, "undef", false))
			unspec = getUnspecType(unspec_str);
		else
			unspec = param_reg;
		return unspec;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PARSERS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Starting from the SPECIE node, the function parses all the REGUL tags and reads the data from them.
	 * If not provided, attributes are defaulted - threshold to 1, label to Label::free
	 */
	void parseRegulations(const rapidxml::xml_node<> * const specie_node, size_t specie_ID) const {
		// Regulation data
		string label;

		map<SpecieID, size_t> numbers;
		size_t counter_number = 0;

		// Cycle through REGUL TAGS
		for (rapidxml::xml_node<>* regulation = XMLHelper::getChildNode(specie_node, "REGUL"); regulation; regulation = regulation->next_sibling("REGUL") ) {
			auto source_ID = getSourceID(regulation, specie_ID);
			auto threshold = getThreshold(regulation, specie_ID, source_ID);
			if (!XMLHelper::getAttribute(label, regulation, "label", false))
				label = Label::Free;

			if(numbers.find(source_ID) == numbers.end())
				numbers.insert(make_pair(source_ID, ++counter_number));

			// Add a new regulation to the specified target
			model.addRegulation(source_ID, specie_ID, threshold, label, numbers.find(source_ID)->second);
		}
	}

	/**
	 * This function obtains a present regulator of a specie as an ordinal number of the regulation in the vector of all regulations of this specie.
	 * If the source is specified in the form "source:value", only the regulator that has the value as a threshold is accepted. If no value is present, threshold of 1 is defaulted.
	 */
	size_t getPresentRegulator(const string & source_str, const SpecieID target_ID) const{
		SpecieID ID;
		size_t threshold = 1; // Defaulted threshold value

		auto colon_pos = source_str.find(":");
		if (colon_pos == string::npos)
			ID = model.findID(source_str);
		else {
			ID = model.findID(source_str.substr(0,colon_pos));
			try {
				threshold = lexical_cast<size_t>(source_str.substr(colon_pos+1));
			}
			catch (bad_lexical_cast & e) {
				output_streamer.output(error_str, "Error while trying to obtain threshold within a regulatory context " + source_str + ": " + e.what() + ".");
				throw runtime_error("lexical_cast<size_t>(" + source_str.substr(colon_pos+1) + ") failed");
			}
		}

		if (ID >= model.getSpeciesCount())
			throw invalid_argument("One of the regulators of the specie " + toString(model.getName(target_ID)) + " was not found in the specie list");

		size_t reg_num = INF; size_t counter = 0;
		forEach(model.getRegulations(target_ID),[&](const Model::Regulation & regulation){
			if (regulation.source == ID)
				if (colon_pos == string::npos || regulation.threshold == threshold) {
					reg_num = counter;
					return;
				}
			counter++;
		});

		if (reg_num == INF)
			throw invalid_argument("regulator " + source_str + " of the specie " + toString(model.getName(target_ID)) + " was not found in the list of regulators");

		return reg_num;
	}

	/**
	 * Controls coherence of regulations - this is only necessary for multi-edge models, where two different thresholds could contradict each other.
	 *
	 * @return true if the regulation is coherent, false otherwise
	 */
	bool isContextCoherent(const SpecieID specie_ID, const vector<bool> & context) const {
		auto regulations = model.getRegulations(specie_ID);

		// Compare all pair
		for (size_t i = 0; i < regulations.size(); i++) {
			for (size_t j = 0; j < regulations.size(); j++) {
				if (regulations[i].source == regulations[j].source && i != j)
					// Return false if there are two present regulations with the same source
					if (context[i] & context[j])
						return false;
			}
		}
		return true;
	}

	/**
    * Use a string defining context together with a value to create a single kintetic parameter.
    * Presence of a regulator can be given as one of: name, name:threshold, ID, ID:threshold.
    * Context is described using present regulators.
	 */
	void fillFromContext(const string context, set<vector<bool> > & specified, SpecieID specie_ID, int target_value) const {
		// Obtain strings of the sources
		vector<string> sources;
		if (!context.empty()) try {
			split(sources, context, is_any_of(","));
		} catch (std::exception & e) {
			output_streamer.output(error_str, string("Error occured while parsing a context. ").append(e.what()));
			throw runtime_error("split(\"sources\", " + context + ", \"is_any_of(\",\")\") failed");
		}

		// Create the mask of present regulators in this context
		vector<bool> mask(model.getRegulations(specie_ID).size(), false);
		for (auto source_it = sources.begin(); source_it != sources.end(); source_it++)
			mask[getPresentRegulator(*source_it, specie_ID)] = true;

		// Add the new regulatory context, if it is coherent and not yet present
		if (!isContextCoherent(specie_ID, mask)){
			throw invalid_argument("context " + context + " of the specie " + toString(model.getName(specie_ID)) + " is incoherent");
		} // Throw an exception if the context is already present
		else if (!specified.insert(mask).second) {
			throw invalid_argument("context redefinition found for the specie " + toString(model.getName(specie_ID)));
		}

		model.addParameter(specie_ID, mask, target_value);
	}

	/**
	 * Use a logic formula to create all kinetic parameters for a specie.
	 * Presence of a regulator can be given as one of: name, name:threshold, ID, ID:threshold.
	 * Context is given as a valuation of a formula.
	 */
	void fillFromLogic(const string logic, size_t specie_ID) const {
		// Get reference values
		vector<bool> bottom(model.getRegulations(specie_ID).size(), false);
		vector<bool> top(model.getRegulations(specie_ID).size(), true);
		vector<bool> tested = bottom;
		map<string, bool> valuation; // Atomic propositions about presence of regulators (true for present, false for absent)

		do {
			if (!isContextCoherent(specie_ID, tested))
				continue;

			// Add current valuations for both a species ID and name (if any)
			valuation.clear();
			for (size_t regul_num = 0; regul_num < tested.size(); regul_num++) {
				// Get the current regulation propeties
				StateID source_ID = (model.getRegulations(specie_ID))[regul_num].source;
				string source_name = model.getName(source_ID);
				size_t threshold = (model.getRegulations(specie_ID))[regul_num].threshold;

				// Fill in the proposition about the presente of the regulator for all possible combinations depicting its name
				valuation.insert(make_pair(source_name, tested[regul_num]));
				valuation.insert(make_pair(source_name + ":" + toString(threshold), tested[regul_num]));
			}

			model.addParameter(specie_ID, tested, FormulaeParser::resolve(valuation, logic));

		} while(iterate<bool>(top, bottom, tested));
	}

	/**
	 * Compute all the possibilities for a regulatory context and add them if they are not already specified.
	 * Based on the unspec attribute of the specie, uses basal value / parametrization / causes error.
	 */
	void addUnspecified(set<vector<bool> > & specified, size_t specie_ID, UnspecifiedParameters unspec) const {
		vector<bool> bottom(model.getRegulations(specie_ID).size(), false);
		vector<bool> top(model.getRegulations(specie_ID).size(), true);
		vector<bool> tested = bottom;

		do {
			// If tested option is new (not already present in the specified vector)
			if (specified.insert(tested).second && isContextCoherent(specie_ID, tested)) {
				switch (unspec) {
					case basal_reg:
						model.addParameter(specie_ID, tested, model.getBasal(specie_ID));
						break;

					case param_reg:
						model.addParameter(specie_ID, tested, -1);
						break;

					case error_reg:
						throw runtime_error("some required parameter specification is missing for the specie " + toString(model.getName(specie_ID)));
						break;
				}
			}
		} while(iterate<bool>(top, bottom, tested));
	}

	/**
	 * Searches for the LOGIC tag and if such is present, uses it for creation of parameters for the specie.
	 *
	 * @return true if the LOGIC was found and used
	 */
	bool parseLogic(const rapidxml::xml_node<> * const specie_node, size_t specie_ID) const {
		// Try to get the tag
		rapidxml::xml_node<>* logic = XMLHelper::getChildNode(specie_node, "LOGIC", false);

		// If the tag is present, use it
		if (logic != 0) {
			if (logic->next_sibling("LOGIC") || logic->next_sibling("PARAM"))
				throw invalid_argument("LOGIC tag does not stay alone in the definition of the specie " + toString(model.getName(specie_ID)));

			// Get and apply the formula
			string formula;
			XMLHelper::getAttribute(formula, logic, "formula");
			fillFromLogic(formula, specie_ID);

			return true;
		}
		else
			return false;
	}

	/**
	 * Starting from the SPECIE node, the function parses all the PARAM tags and reads the data from them.
	 */
	void parseParameters(const rapidxml::xml_node<> * const specie_node, size_t specie_ID) const {
		// Parameters data data
		string context; string target_val_str; int target_value = -1;
		auto unspec = getUnspecified(specie_node);
		set<vector<bool> > specified; // Used for possibility of partial specification

		// Step into first PARAM tag, end when the current node does not have next sibling (all PARAM tags were parsed)
      for (rapidxml::xml_node<> * parameter = XMLHelper::getChildNode(specie_node, "PARAM", false); parameter; parameter = parameter->next_sibling("PARAM") ) {
			// Get the mask string.
			XMLHelper::getAttribute(context, parameter, "context");

         // Get the targte value (set to -1 if uknown or unspecified) and check it
         if (!XMLHelper::getAttribute(target_val_str, parameter, "value", false))
            target_value = -1;
         else if (target_val_str.compare("?") == 0)
            target_value = -1;
         else {
            XMLHelper::getAttribute(target_value, parameter, "value", false);
            if (target_value < 0 || target_value > static_cast<int>(model.getMax(specie_ID)))
               throw invalid_argument("target value " + toString(target_value) + " out of range for specie " + toString(model.getName(specie_ID)));
         }

			fillFromContext(context, specified, specie_ID, target_value);
		}

		addUnspecified(specified, specie_ID, unspec);
	}

	/**
	 * Starting from the STRUCTURE node, the function parses all the SPECIE tags and reads the data from them.
	 * If not provided, attributes are defaulted - name is equal to ordinal number starting from 0, max to 1 and basal value to 0.
	 */
	void firstParse(const rapidxml::xml_node<> * const structure_node) const {
		// Start the naming from capital A.
		char specie_name = 'A';
		// Specie data
		string name; size_t max; size_t basal;

		// Step into first SPECIE tag, end when the current node does not have next sibling (all SPECIES tags were parsed)
		rapidxml::xml_node<> *specie = XMLHelper::getChildNode(structure_node, "SPECIE");
		for (SpecieID ID = 0; specie; ID++, specie = specie->next_sibling("SPECIE"), specie_name++) {
			// Get a name of the specie.
			if (!XMLHelper::getAttribute(name, specie, "name", false))
				name = toString(specie_name);
			// Throw an error if the name is not correct.
			else if (name.length() < 2 || !isalpha(static_cast<int>(name[0])))
				throw invalid_argument("Name of the specie \"" + name + "\" is incorrect. Specie name can start only with a letter and must be at least 2 symbols in lenght.");
			// Get a max value and conver to integer.
			if (!XMLHelper::getAttribute(max, specie, "max", false))
				max = 1;
			// Get a basal value and conver to integer.
			if(!XMLHelper::getAttribute(basal, specie, "basal", false))
				basal = 0;

         if (basal > max)
            throw invalid_argument("basal value is greater than maximal value for specie " + toString(ID));

			// Create a new specie
			model.addSpecie(name, max, basal);
		}
	}

	/**
	 * Starting from the STRUCTURE node, the function parses all the SPECIE tags and reads the data from them.
	 */
	void secondParse(const rapidxml::xml_node<> * const structure_node) const {
		// Step into first SPECIE tag, end when the current node does not have next sibling (all SPECIES tags were parsed)
		rapidxml::xml_node<> *specie = XMLHelper::getChildNode(structure_node, "SPECIE");
		for (SpecieID ID = 0; specie; ID++, specie = specie->next_sibling("SPECIE") ) {
				// Get all the regulations of the specie and store them to the model.
			parseRegulations(specie, ID);
			// Try to search for a specification using a formula
			if (!parseLogic(specie, ID)) {
				// If not present, use normal parameters
				parseParameters(specie, ID);
			}
		}
		model.sortParameters();
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION METHODS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	NetworkParser(const NetworkParser & other); ///< Forbidden copy constructor.
	NetworkParser& operator=(const NetworkParser & other); ///< Forbidden assignment operator.

public:
   NetworkParser(Model & _model) : model(_model) { } ///< Simple constructor, passes references.

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
