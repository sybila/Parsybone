/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warranty. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// @file This is a testing suite for the Parsybone program

#include "tests/parsing_test.hpp"
#include "tests/core_level_tests.hpp"
#include "tests/formulae_parser_test.hpp"
#include "tests/constraint_parser_test.hpp"
#include "tests/model_test.hpp"
#include "tests/kinetics_test.hpp"
#include "tests/construction_test.hpp"
#include "tests/synthesis_test.hpp"
#include "tests/output_test.hpp"

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest( &argc, argv );
	int result = RUN_ALL_TESTS();

#if (_MSC_VER == 1800)
		cin.get();
#endif 

	return result;
}
