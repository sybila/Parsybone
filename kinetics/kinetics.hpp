/*
* Copyright (C) 2012-2014 - Adam Streck
* This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
* ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
* ParSyBoNe is released without any warranty. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
* For affiliations see http://www.mi.fu-berlin.de/en/math/groups/dibimath and http://sybila.fi.muni.cz/ .
*/

#ifndef MODEL_HPP
#define MODEL_HPP

#include "../auxiliary/common_functions.hpp"
#include "../auxiliary/output_streamer.hpp"

struct Kintetics {
	struct ParameterData {
		bool functional;
		Levels list_of_params;
	};

	struct Specie {
		map<string, ParameterData> params;
		Configurations partial_coloring;
		ParamNo step_size;
	};

	map <string, Specie> species;
};

#endif // MODEL_HPP