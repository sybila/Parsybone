/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_XML_HELPER_INCLUDED
#define PARSYBONE_XML_HELPER_INCLUDED

#include "../auxiliary/data_types.hpp"

#include "rapidxml-1.13/rapidxml.hpp"
#include "rapidxml-1.13/rapidxml_iterators.hpp"
#include "rapidxml-1.13/rapidxml_print.hpp"
#include "rapidxml-1.13/rapidxml_utils.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// This class is pure static and encapsulates simple parsing functions for bigger robustness.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class XMLHelper {

public:
	/**
	 * Gets pointer to the descendant of the current node.
	 *
	 * @param current_node	pointer to the ancestor of requested node
	 * @param node_name	string with the name of the decendant
	 *
	 * @return	pointer to the descendant if sucessful
	 */
	static rapidxml::xml_node<> * getChildNode(const rapidxml::xml_node<> * const current_node, const char* node_name) {
		rapidxml::xml_node<> * return_node = 0;
		// try to get the node
		return_node = current_node->first_node(node_name);
		if (return_node == 0)
			throw std::runtime_error(std::string("Parser did not found the ").append(node_name).append(" node"));
		return return_node;
	}

	/**
	 * Gets pointer to the sibling of the current node.
	 *
	 * @param current_node	pointer to the ancestor of requested node
	 * @param node_name	string with the name of the decendant
	 *
	 * @return	pointer to the sybling if sucessful
	 */
	static rapidxml::xml_node<> * getSiblingNode(const rapidxml::xml_node<> * const current_node, const char* node_name) {
		rapidxml::xml_node<> * return_node = 0;
		// try to get the node
		return_node = current_node->next_sibling(node_name);
		if (return_node == 0)
			throw std::runtime_error(std::string("Parser did not found the ").append(node_name).append(" node"));
		return return_node;
	}

	/**
	 * Gets value of the attribute in the correct data type.
	 *
	 * @param requested_data	variable that will be filled with requested value
	 * @param current_node	pointer to the node holding requested attribute
	 * @param attribute_name	string with the name of the attribute
	 * @param mandatory	true if absence of the attribute should raise exception
	 *
	 * @return true if the argument was present, false otherwise
	 */
	template <class returnType>
	static bool getAttribute(returnType & requested_data, const rapidxml::xml_node<> * const current_node, const char* attribute_name, bool mandatory = true) {
		rapidxml::xml_attribute<> *temp_attr = 0;
		// Try to get the attribute
		temp_attr = current_node->first_attribute(attribute_name);
		// Check if the attribute has been required
		if (temp_attr == 0) {
			if (mandatory)
				throw std::runtime_error(std::string("Parser did not found the mandatory attribute ").append(attribute_name));
			else
				return false;
		}
		else {
			// Try to convert attribute into requested data type
			try {
				requested_data = boost::lexical_cast<returnType, char*>(temp_attr->value());
			} catch (boost::bad_lexical_cast e) {
				output_streamer.output(error_str, std::string("Error while parsing an attribute ").append(attribute_name).append(": ").append(e.what()));
				throw std::runtime_error("boost::lexical_cast<returnType, char*>(temp_attr->value()) failed");
			}
		}
		return true;
	}

	virtual ~XMLHelper() = 0; ///< Pure virtual desctructor
};

#endif // PARSYBONE_XML_HELPER_INCLUDED
