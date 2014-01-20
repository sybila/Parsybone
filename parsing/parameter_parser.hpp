#ifndef PARAMETER_PARSER_HPP
#define PARAMETER_PARSER_HPP

#include "../model/model_translators.hpp"
#include "../parsing/xml_helper.hpp"

class ParameterParser {
private:
	/**
	 * @brief parseParameters Reads the constraints.
	 */
	static vector<pair<string, string>> parseParKin(const rapidxml::xml_node<> * const specie_node) {
		vector<pair<string, string>>  result;

		// List through all the PARAM nodes.
		for (auto parameter : XMLHelper::NodesRange(specie_node, "PARAM", false)) {
			// Obtain context specified.
			string context;
			XMLHelper::getAttribute(context, parameter, "context");
			string value = "?";
			XMLHelper::getAttribute(value, parameter, "value", false);
			result.push_back(make_pair(context, value));
		}

		return result;
	}

	/**
	* @brief parseParameters Reads the constraints.
	*/
	static vector<string> parseParCons(const rapidxml::xml_node<> * const specie_node) {
		vector<string>  result;

		// List through all the CONSTR nodes.
		for (auto parameter : XMLHelper::NodesRange(specie_node, "CONSTR", false)) {
			string expression;
			XMLHelper::getAttribute(expression, parameter, "expr");
			result.push_back(expression);
		}

		return result;
	}

public:
	/**
	 * @brief parse   Reads explicit parameter specification from the model file.
	 */
	static void parse(const rapidxml::xml_node<> * const network_node, Model & model) {
		// For each specie find explicit descriptions
		auto specie = XMLHelper::getChildNode(network_node, "SPECIE");
		for (SpecieID ID = 0; specie; ID++, specie = specie->next_sibling("SPECIE")) {
			// Create all contexts with all the possible values.
			model.species[ID].par_kin = parseParKin(specie);
			model.species[ID].par_cons = parseParCons(specie);
		}
	}
};

#endif // PARAMETER_PARSER_HPP
