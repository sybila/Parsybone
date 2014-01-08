/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSING_TEST_DATA_HPP
#define PARSING_TEST_DATA_HPP

#include <gtest/gtest.h>
#include "../auxiliary/data_types.hpp"

class ParsingTest : public ::testing::Test {
protected:
   const string source_path = "../";
   const string example_model = "example_model";
   const string example_series = "example_series";
   const string example_automaton = "example_automaton";

   void SetUp() override {

   }
};

#endif // PARSING_TEST_DATA_HPP
