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

class ParsingTest : public ::testing::Test {
protected:
   const string example_model_pmf = "../example_model.pmf";
   string example_series_ppf = "../example_series.ppf";
   string example_automaton_ppf = "../example_automaton.ppf";

   void SetUp() override {

   }
};

#endif // PARSING_TEST_DATA_HPP
