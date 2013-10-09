/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSING_TEST_HPP
#define PARSING_TEST_HPP

#include "parsing_test_data.hpp"
#include "../parsing/parsing_manager.hpp"

TEST_F(ParsingTest, ParseExamples) {
   Model example_m;
   EXPECT_NO_THROW(example_m = ParsingManager::parseModel(example_model_pmf));
   PropertyAutomaton example_a;
   EXPECT_NO_THROW(example_a = ParsingManager::parseProperty(example_automaton_ppf));
   PropertyAutomaton example_s;
   EXPECT_NO_THROW(example_s = ParsingManager::parseProperty(example_series_ppf));
}

#endif // PARSING_TEST_HPP
