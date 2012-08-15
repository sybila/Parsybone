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

void test(bool (*test_function)(void), std::string test_name){
	std::cout << "# Executing test: " << test_name << std::endl;
	if (test_function)
		std::cout << "correct" << std::endl;
	else
		std::cout << "failed" << std::endl;
}

bool testFormulaeParser() {
	std::map<std::string, bool> vars;
	vars.insert(std::make_pair("A",true));
	vars.insert(std::make_pair("B",false));

	std::vector<std::string> formulas;
	formulas.push_back("A");
	formulas.push_back("!B");
	formulas.push_back("(A|B)");
	formulas.push_back("(!(A&A)|!B)");

	for (auto formula_it = formulas.begin(); formula_it != formulas.end(); formula_it++) {
		if (!FormulaeParser::resolve(vars, *formula_it)) {
			std::cout << "formula " << *formula_it << " is false.";
			return false;
		}
	}
	return true;
}

int main(int argc, char* argv[]) {

	test(testFormulaeParser, "testFormulaeParser");

	return 0;
}
