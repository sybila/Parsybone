#ifndef PARSYBONE_AUTOMATON_PARSER_INCLUDED
#define PARSYBONE_AUTOMATON_PARSER_INCLUDED

#include "../auxiliary/data_types.hpp"
#include "../auxiliary/common_functions.hpp"
#include "xml_helper.hpp"
#include "model.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// This object is responsible for parsing and translation of data related to the tested property.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class AutomatonParser {
	Model & model; ///< Reference to the model object that will be filled

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PARSING:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Starting from the STATE node, the function parses all the EDGE tags and reads the data from them.
	 */
	void parseEdges(const rapidxml::xml_node<> * const state_node, StateID source_ID) const {
		// Regulation data
		std::string label_string; std::size_t target_ID;

		// Step into first STATE tag, end when the current node does not have next sibling (all STATE tags were parsed)
		for (rapidxml::xml_node<> * edge = XMLHelper::getChildNode(state_node, "EDGE"); edge; edge = edge->next_sibling("EDGE")) {
			// Get the mask string.
			XMLHelper::getAttribute(label_string, edge, "label");
			// Get max value and conver to integer.
			XMLHelper::getAttribute(target_ID, edge, "target");

			// Add a new regulation to the specified target
			model.addConditions(source_ID, target_ID, std::move(label_string));
		}
	}

	/**
	 * Starting from the AUTOMATON node, the function parses all the STATE tags and all their TRANSITION tags and reads the data from them.
	 * If not found, final attribute is defaulted to false and name to state's ordinal number.
	 */
	void firstParse(const rapidxml::xml_node<> * const automaton_node) const {
		// State data
		bool final; std::string name;

		// Step into first STATE tag, end when the current node does not have next sibling (all STATE tags were parsed)
		rapidxml::xml_node<> *state = XMLHelper::getChildNode(automaton_node, "STATE");
		for (SpecieID ID = 0; state; ID++, state = state->next_sibling("STATE") ) {
			// Find out whether the state is final
			if(!XMLHelper::getAttribute(final, state, "final", false))
				final = false;

			// Get a name of the state, or use its ID as a string
			if(!XMLHelper::getAttribute(name, state, "name", false))
				name = toString(ID);

			// Create a new state
			model.addState(name, final);
		}
	}

	/**
	 * Starting from the AUTOMATON node, the function parses all the STATE tags and all their TRANSITION tags and reads the data from them.
	 * If not found, final attribute is defaulted to false and name to state's ordinal number.
	 */
	void secondParse(const rapidxml::xml_node<> * const automaton_node) const {
		// Step into first STATE tag, end when the current node does not have next sibling (all STATE tags were parsed)
		rapidxml::xml_node<> *state = XMLHelper::getChildNode(automaton_node, "STATE");
		for (SpecieID ID = 0; state; ID++, state = state->next_sibling("STATE") ) {
			// Get all the transitions of the state and store them to the model.
			parseEdges(state, ID);
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	AutomatonParser(const AutomatonParser & other); ///< Forbidden copy constructor.
	AutomatonParser& operator=(const AutomatonParser & other); ///< Forbidden assignment operator.

public:
	AutomatonParser(Model & _model) : model(_model) { } ///< Simple constructor, passes references

	/**
	 * Main parsing function. It expects a pointer to inside of a MODEL node.
	 */
	void parse(const rapidxml::xml_node<> * const current_node) {
		// Parse Buchi Automaton
		output_streamer.output(verbose_str, "Started reading of the Buchi automaton.");
		// Parse states
		firstParse(XMLHelper::getChildNode(current_node, "AUTOMATON"));
		// Parse transition for the states from the previous parse
		secondParse(XMLHelper::getChildNode(current_node, "AUTOMATON"));
	}
};

#endif // PARSYBONE_AUTOMATON_PARSER_INCLUDED
