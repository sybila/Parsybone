#ifndef PARSYBONE_AUTOMATON_PARSER_INCLUDED
#define PARSYBONE_AUTOMATON_PARSER_INCLUDED

#include "../auxiliary/data_types.hpp"
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
		rapidxml::xml_node<>      *transition;
		// Regulation data
		std::string label_string; std::size_t target_ID;

		// Step into first EDGE tag
		transition = XMLHelper::getChildNode(state_node, "EDGE");

		while (true) { // End when the current node does not have next sibling (all EDGE tags were parsed)
			// Get the mask string.
			XMLHelper::getAttribute(label_string, transition, "label");
			// Get max value and conver to integer.
			XMLHelper::getAttribute(target_ID, transition, "target");

			// Add a new regulation to the specified target
			model.addConditions(source_ID, target_ID, std::move(label_string));

			// Continue stepping into REGUL tags while possible
			if (transition->next_sibling("EDGE"))
				transition = transition->next_sibling("EDGE");
			else break;
		}
	}

	/**
	 * Starting from the AUTOMATON node, the function parses all the STATE tags and all their TRANSITION tags and reads the data from them.
	 */
	void parseStates(const rapidxml::xml_node<> * const automaton_node) const {
		rapidxml::xml_node<>      *state;
		// State data
		bool final;

		// Step into first SPECIE tag
		state = XMLHelper::getChildNode(automaton_node, "STATE");

		while (true) { // End when the current node does not have next sibling (all STATES tags were parsed)
			// Find out whether the state is final
			XMLHelper::getAttribute(final, state, "final");

			// Create a new state
			StateID ID = model.addState(final);

			// Get all the transitions of the state and store them to the model.
			parseEdges(state, ID);

			// Continue stepping into STATE tags while possible
			if (state->next_sibling("STATE"))
				state = state->next_sibling("STATE");
			else break;
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	AutomatonParser(const AutomatonParser & other); ///< Forbidden copy constructor.
	AutomatonParser& operator=(const AutomatonParser & other); ///< Forbidden assignment operator.

public:
	AutomatonParser(Model & _model) : model(_model) { } ///< Simple constructor, passes references

	void parse(const rapidxml::xml_node<> * const current_node) {
		// Parse Buchi Automaton
		output_streamer.output(verbose_str, "Started reading of the Buchi automaton.");
		parseStates(XMLHelper::getChildNode(current_node, "AUTOMATON"));
	}
};

#endif // PARSYBONE_AUTOMATON_PARSER_INCLUDED
