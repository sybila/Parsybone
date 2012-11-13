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
/// \brief This namespace encapsulates simple parsing functions with tests for bigger robustness.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace XMLHelper {
	/**
	 * Gets pointer to the descendant of the current node.
	 *
    * @param[in] current_node	pointer to the ancestor of requested node
    * @param[in] node_name	string with the name of the decendant
    * @param[in] mandatory	true if absence of the node should raise exception
	 *
	 * @return	pointer to the descendant if sucessful
	 */
   static rapidxml::xml_node<> * getChildNode(const rapidxml::xml_node<> * const current_node, const char* node_name, const bool mandatory = true) {
		rapidxml::xml_node<> * return_node = 0;
		// try to get the node
		return_node = current_node->first_node(node_name);
		if (return_node == 0) {
			if (mandatory)
				throw std::runtime_error(std::string("Parser did not found the mandatory ").append(node_name).append(" node"));
			else
				return 0;
		}
		return return_node;
	}

	/**
	 * Reads the value of the specified attribute and converts it to correct data type if possible.
	 *
    * @param[out] requested_data	variable that will be filled with requested value
    * @param[in] current_node	pointer to the node holding requested attribute
    * @param[in] attribute_name	string with the name of the attribute
    * @param[in] mandatory	true if absence of the attribute should raise exception
	 *
	 * @return true if the argument was present, false otherwise
	 */
	template <class returnType>
   static bool getAttribute(returnType & requested_data, const rapidxml::xml_node<> * const current_node, const char* attribute_name, const bool mandatory = true) {
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
				throw std::runtime_error("boost::lexical_cast<" + std::string(typeid(returnType).name()) + ", char*>(" + std::string(temp_attr->value()) + ") failed");
			}
		}
		return true;
	}
}

#endif // PARSYBONE_XML_HELPER_INCLUDED
