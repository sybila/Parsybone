/*
* Copyright (C) 2012-2014 - Adam Streck
* This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
* ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
* ParSyBoNe is released without any warranty. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
* For affiliations see http://www.mi.fu-berlin.de/en/math/groups/dibimath and http://sybila.fi.muni.cz/ .
*/

#pragma once

#include "../auxiliary/common_functions.hpp"
#include "../auxiliary/output_streamer.hpp"

struct Kinetics {
	struct Param {
		string context;
		Levels targets; ///< Towards which level this context may regulate.
		map<StateID, Levels> requirements; ///< Levels of the source species.
	
		Levels target_in_subcolor;
		bool functional;
	};
	using Params = vector<Param>;

	struct Specie {
		string name;
		Params params;
		ParamNo col_count;
		ParamNo step_size;
	};

	vector<Specie> species;
};