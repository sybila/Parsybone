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

}

TEST_F(SynthesisTest, TestGeneral) {
   user_options.compute_robustness = false;
   user_options.compute_wintess = false;

   c_2_s_2_o_man->checkGeneral();
   EXPECT_TRUE(c_2_s_2_o_man->results.isAccepting());

   b_k_c_man->checkGeneral();
   EXPECT_TRUE(b_k_c_man->results.isAccepting());
}

TEST_F(SynthesisTest, TestRobustness) {
   user_options.compute_robustness = true;
   user_options.compute_wintess = true;
   c_2_s_2_o_man->checkFinite();
   EXPECT_TRUE(c_2_s_2_o_man->results.isAccepting());
}

#endif // SYNTHESIS_TESTS_HPP
