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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Class for parsing of the regulatory network.
///
/// This object is responsible for parsing and translation of data related to the GRN.
/// Most of the possible semantics mistakes are under control and cause exceptions.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class NetworkParser {
    friend class Tester;
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
		 for(const auto & regul:regulations) {
			 if (threshold == regul.threshold && source_ID == regul.source)
				 throw invalid_argument("multiple definition of a regulation of a specie " + toString(model.getName(source_ID)));
		 }

		 return threshold;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// PARSERS:
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void fillActivationLevels() const {
		for (auto ID:range(model.getSpeciesCount())) {
			auto space = model.getThresholds(ID);
			for (auto regul:model.getRegulations((ID))) {
				ActLevel begin = regul.threshold;
				auto thresholds = space.find(regul.source)->second;
				auto th_it = find(thresholds.begin(), thresholds.end(), begin) + 1;
				ActLevel end = (th_it == thresholds.end()) ? model.getMax(regul.source) + 1 : *th_it;

				model.addActivityLevels(regul.source, ID, range(begin, end));
			}
		}
	}

	/**
	 * Starting from the SPECIE node, the function parses all the REGUL tags and reads the data from them.
	 * If not provided, attributes are defaulted - threshold to 1, label to Label::free
	 */
	void parseRegulations(const rapidxml::xml_node<> * const specie_node, size_t specie_ID) const {
		// Regulation data
		string label;

		// Cycle through REGUL TAGS
		for (rapidxml::xml_node<>* regulation = XMLHelper::getChildNode(specie_node, "REGUL"); regulation; regulation = regulation->next_sibling("REGUL") ) {
			auto source_ID = getSourceID(regulation, specie_ID);
			auto threshold = getThreshold(regulation, specie_ID, source_ID);
			if (!XMLHelper::getAttribute(label, regulation, "label", false))
				label = Label::Free;

			// Add a new regulation to the specified target
			model.addRegulation(source_ID, specie_ID, threshold, label);
		}

		fillActivationLevels();
	}

	/**
	  * Searches for the LOGIC tag and if such is present, uses it for creation of parameters for the specie.
	  *
	  * @return true if the LOGIC was found and used
	  */
	string parseLogic(const rapidxml::xml_node<> * const specie_node, size_t specie_ID) const {
		// Try to get the tag
		rapidxml::xml_node<>* logic = XMLHelper::getChildNode(specie_node, "LOGIC", false);

		// If the tag is present, use it
		if (logic != 0) {
			if (logic->next_sibling("LOGIC"))
				throw invalid_argument("LOGIC tag is present multiple times in the definition of the specie " + toString(model.getName(specie_ID)));

			// Get and apply the formula
			string formula;
			XMLHelper::getAttribute(formula, logic, "formula");

			return formula;
		}
		else
			return "";
	}

	/**
	 * @brief getCanonic
	 * @param context
	 * @param target_ID
	 * @return
	 */
	string getCanonic(const string & context, const SpecieID target_ID) const {
		string new_context;
		auto names = model.getRegulatorsNames(target_ID);
		for (auto name:names) {
			new_context += name;
			auto pos = context.find(name);
			auto colon_pos = pos + name.length();
			if (pos == context.npos)
				new_context += ":0,";
			else if (context[colon_pos] != ':')
				new_context += ":1,";
			else {
				new_context += ":";
				while(isdigit(context[++colon_pos]))
					new_context.append(1, context[colon_pos]);
				new_context += ",";
			}
		}
		return new_context.substr(0, new_context.length() - 1);
	}

	void replaceExplicit(Model::Parameters & parameters, const rapidxml::xml_node<> * const specie_node, const SpecieID target_ID) const {
		for (rapidxml::xml_node<> * parameter = XMLHelper::getChildNode(specie_node, "PARAM", false); parameter; parameter = parameter->next_sibling("PARAM") ) {
			string context = "", target_val_str = "";
			ActLevel target_val = 0;
			Levels targets;

			// Get the mask string.
			XMLHelper::getAttribute(context, parameter, "context");
			string new_context = getCanonic(context, target_ID);

			// Get the targte value (set to -1 if uknown or unspecified) and check it
			if (XMLHelper::getAttribute(target_val_str, parameter, "value", false)) {
				if (target_val_str.compare("?") == 0) {
					targets = model.getRange(target_ID);
				} else {
					XMLHelper::getAttribute(target_val, parameter, "value", false);
					if (target_val < model.getMin(target_ID) || target_val > model.getMax(target_ID))
						throw invalid_argument("target value " + target_val_str + " out of range for specie " + model.getName(target_ID));
					targets.push_back(target_val);
				}
			}
			else
				targets = model.getRange(target_ID);

			bool found = false;
			for(auto & param:parameters) {
				if (param.context.compare(new_context) == 0) {
					param.targets = targets;
					found = true;
					break;
				}
			}
			if (! found)
				throw runtime_error("Given context " + context + "not mached, probably incorrect.");
		}
	}

	Model::Parameters createParameters(const SpecieID target_ID, string formula) const {
		auto all_thresholds = model.getThresholds(target_ID);
		auto regulations = model.getRegulations(target_ID);
		auto IDs = model.getRegulatorsIDs(target_ID);
		auto names = model.getRegulatorsNames(target_ID);
		Levels bottom, context, top;
		Model::Parameters parameters;

		// These containers hold number of thresholds pre regulator.
		for (auto & source_thresholds:all_thresholds) {
			bottom.push_back(0);
			context.push_back(0);
			top.push_back(source_thresholds.second.size());
		}

		// Loop over all the contexts.
		do {
			FormulaeParser::Vals present_regulators;
			Model::Parameter parameter = {"", map<StateID, Levels>(), Levels()};

			// Loop over all the sources.
			for (auto source_num:range(context.size())) {
				string source_name = names[source_num];
				StateID source_ID = IDs[source_num];
				auto & thresholds = all_thresholds.find(source_ID)->second;
				ActLevel threshold = (context[source_num] == 0) ? 0 : thresholds[context[source_num] - 1];
				string regulation_name = source_name + ":" + toString(threshold);
				if (!formula.empty()) {
					if (threshold == 1)
						present_regulators.insert(FormulaeParser::Val(source_name, 1));
					present_regulators.insert(FormulaeParser::Val(regulation_name, 1));
				}
				parameter.context += regulation_name + ",";
				ActLevel next_th = (context[source_num] == thresholds.size()) ? model.getMax(source_ID) + 1 : thresholds[context[source_num]];
				Levels activity_levels = range(threshold, next_th);
				parameter.requirements.insert(make_pair(source_ID, activity_levels));
			}
			if (!formula.empty()) {
				for (auto regul:model.getRegulations(target_ID)) {
					if (present_regulators.find(regul.name) == present_regulators.end()) {
						present_regulators.insert(FormulaeParser::Val(regul.name, 0));
						if (regul.threshold == 1)
							present_regulators.insert(FormulaeParser::Val(model.getName(regul.source), 0));
					}
				}
				parameter.targets = Levels(1, FormulaeParser::resolve(present_regulators, formula));
			} else {
				parameter.targets = model.getTargets(target_ID);
			}

			parameter.context = parameter.context.substr(0, parameter.context.length() - 1);
			parameters.push_back(parameter);
		} while(iterate(top, bottom, context));

		return parameters;
	}

	/**
	 * Starting from the STRUCTURE node, the function parses all the SPECIE tags and reads the data from them.
	 * If not provided, attributes are defaulted - name is equal to ordinal number starting from 0, max to 1, targets to the whole range.
	 */
	void firstParse(const rapidxml::xml_node<> * const structure_node) const {
		// Start the naming from capital A.
		char specie_name = 'A';
		// Specie data
		string name; size_t max; size_t basal = 0; Levels targets;

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

			if(XMLHelper::getAttribute(basal, specie, "basal", false)) {
				targets.push_back(basal);
				if (basal > max)
					throw invalid_argument("basal value is greater than maximal value for specie " + toString(ID));
			} else {
				targets = range(max + 1);
			}

			// Create a new specie
			model.addSpecie(name, max, targets);
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
		}

		fillActivationLevels();

		specie = XMLHelper::getChildNode(structure_node, "SPECIE");
		for (SpecieID ID = 0; specie; ID++, specie = specie->next_sibling("SPECIE") ) {
			// Create all contexts with all of their possible combinations.
			auto formula = parseLogic(specie, ID);
			auto parameters = createParameters(ID, formula);
			replaceExplicit(parameters, specie, ID);
			model.addParameters(ID, parameters);
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION METHODS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	NetworkParser(const NetworkParser & other) = delete; ///< Forbidden copy constructor.
	NetworkParser& operator=(const NetworkParser & other) = delete; ///< Forbidden assignment operator.

public:
	NetworkParser(Model & _model) : model(_model) { } ///< Simple constructor, passes a references.

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
