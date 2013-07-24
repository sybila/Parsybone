/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This is a testing suite for the Parsybone program

using namespace std;

#include "tests/formulae_parser_test.hpp"
#include "tests/model_functions_test.hpp"

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}
