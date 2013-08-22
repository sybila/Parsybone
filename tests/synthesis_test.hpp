/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef SYNTHESIS_TESTS_HPP
#define SYNTHESIS_TESTS_HPP

#include "synthesis_test_data.hpp"

TEST_F(SynthesisTest, TestColoring) {
   ColorStorage c_2_s_2_o_stor(c_2_set_two_ones);
   ModelChecker c_2_s_2_o_check(c_2_set_two_ones, c_2_s_2_o_stor);
   EXPECT_TRUE(c_2_s_2_o_check.conductCheck(CheckerSettings()).is_accepting);
}

TEST_F(SynthesisTest, TestRobustness) {

}

#endif // SYNTHESIS_TESTS_HPP
