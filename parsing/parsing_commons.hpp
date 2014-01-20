#ifndef PARSING_COMMONS_HPP
#define PARSING_COMMONS_HPP

#include "../auxiliary/data_types.hpp"

namespace ParsingCommons {
	bool isValidSpecName(const string & spec_name) {
		bool valid = isalpha(spec_name[0]) || spec_name[0] == '_';
		for (char ch : spec_name)
			valid &= isalnum(ch) || ch == '_';
		return spec_name.length() >= 2 && valid;
	}

	void specNameExc(const string & spec_name) {
		throw invalid_argument("Name of the specie \"" + spec_name + "\" is incorrect. Specie names can contain only letters,numbers and underscore.");
	}
}

#endif // PARSING_COMMONS_HPP
