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
#include "../auxiliary/common_functions.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Methods used for translation of string data to variables during model parsing.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Translator {
	/**
	 * @param mask_string	mask of the active regulations in given regulatory context in the form of a string
	 *
	 * @return Vector of boolean values that represents the input mask.
	 */
	static vector<bool> getMask(string mask_string) {
		// Vector that will hold the mask instead of the string
		vector<bool> mask;
		// For all characters in the mask
		for_each(mask_string.begin(), mask_string.end(),[&mask](char ch) {
			// Check correctness of the symbols
			if (ch != '0' && ch !='1')
				throw runtime_error("Error occured while parsing a regulation. It seems that you have entered value other than 0 or 1 in the mask.");
			// Push the value to the vector
			try {
				mask.push_back(lexical_cast<bool, char>(ch));
			} catch (bad_lexical_cast e) {
			output_streamer.output(error_str, "string(""Error occured while parsing a regulation: " + e.what());
				throw runtime_error("lexical_cast<size_t, char>(" + toString(temp_attr->value()) ") failed");
			}
		});
		return mask;
	}
};

#endif // PARSYBONE_TRANSLATOR_INCLUDED
