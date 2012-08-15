/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This is the testing appliaction for the Parsybone program
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "auxiliary/time_manager.hpp"
#include "auxiliary/output_streamer.hpp"
#include "auxiliary/user_options.hpp"
#include "parsing/argument_parser.hpp"
#include "parsing/model_parser.hpp"
#include "parsing/parsing_manager.hpp"
#include "parsing/formulae_parser.hpp"
#include "construction/construction_manager.hpp"
#include "construction/product_builder.hpp"
#include "synthesis/synthesis_manager.hpp"

// program-related data
const float program_version = 1.0;

int main(int argc, char* argv[]) {
	std::map<std::string, bool> vars;
	vars.insert(std::make_pair("A",true));
	vars.insert(std::make_pair("B",false));
	bool result;
	std::string formula1 = "(A|B)"; std::string formula2 = "!B"; std::string formula3 = "(!(A&A)|!B)";
	result = FormulaeParser::resolve(vars, formula1);
	result = FormulaeParser::resolve(vars, formula2);
	result = FormulaeParser::resolve(vars, formula3);

	return 0;
}
