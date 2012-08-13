/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_TRANSLATOR_INCLUDED
#define PARSYBONE_TRANSLATOR_INCLUDED

#include "../auxiliary/data_types.hpp"

class Translator {

public:
	/**
	 * @param label	label on the edge for some arbitrary interaction - can be + or - or nothing
	 *
	 * @return	enumeration item with given specification
	 */
	static const EdgeConstrain readConstrain(const std::string & label) {
		// Test possible options
		if (label.compare("") == 0)
			return none_cons;
		else if (label.compare("+") == 0)
			return pos_cons;
		else if (label.compare("-") == 0)
			return neg_cons;
		else
			throw std::runtime_error("Wrong sing in interaction label.");
	}

	/**
	 * @param mask_string	mask of the active interactions in given regulatory context in the form of a string
	 *
	 * @return Vector of boolean values that represents the input mask.
	 */
	static std::vector<bool> getMask(std::string mask_string) {
		// Vector that will hold the mask instead of the string
		std::vector<bool> mask;
		// For all characters in the mask
		std::for_each(mask_string.begin(), mask_string.end(),[&mask](char ch) {
			// Check correctness of the symbols
			if (ch != '0' && ch !='1')
				throw std::runtime_error("Error occured while parsing a regulation. It seems that you have entered value other than 0 or 1 in the mask.");
			// Push the value to the vector
			try {
				mask.push_back(boost::lexical_cast<bool, char>(ch));
			} catch (boost::bad_lexical_cast e) {
				output_streamer.output(error_str, std::string("Error occured while parsing a regulation: ").append(e.what()));
				throw std::runtime_error("boost::lexical_cast<size_t, char>(temp_attr->value()) failed");
			}
		});
		return mask;
	}

	/**
	 * @param uspec_type	what to do with usnpecified regulations
	 *
	 * @return	enumeration item with given specification
	 */
	static const UnspecifiedRegulations getUnspecType(std::string unspec_type) {
		if      (unspec_type.compare("error"))
			return error_reg;
		else if (unspec_type.compare("basal"))
			return basal_reg;
		else if (unspec_type.compare("param"))
			return param_reg;
		else
			throw std::runtime_error("Wrong value given as an uspec attribute.");
	}

	virtual ~Translator() = 0; ///< Pure virtual desctructor
};

#endif // PARSYBONE_TRANSLATOR_INCLUDED
