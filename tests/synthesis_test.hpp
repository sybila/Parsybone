#ifndef SYNTHESIS_TESTS_HPP
#define SYNTHESIS_TESTS_HPP

#include "synthesis_test_data.hpp"

TEST_F(SynthesisTest, TestColoring) {

}

TEST_F(SynthesisTest, TestRobustness) {
   user_options.compute_robustness = true;
   user_options.compute_wintess = true;
   user_options.time_series = true;

   c_2_s_2_o_man->doPreparation();
   c_2_s_2_o_man->doColoring();
   c_2_s_2_o_man->doAnalysis();
   EXPECT_GE(1, ParamsetHelper::count(c_2_s_2_o_man->analyzer->getMask()));
}

#endif // SYNTHESIS_TESTS_HPP
