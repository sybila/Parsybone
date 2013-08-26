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

bool containsTrans(const string & witness, const vector<string> & trans ) {
   for (const string & tran : trans)
      if (witness.find(tran) == witness.npos)
         return false;
   return true;
}

TEST_F(SynthesisTest, SetTwoOnCircuitFull) {
   ColorStorage storage(c_2_set_two_ones);
   ModelChecker checker(c_2_set_two_ones, storage);
   WitnessSearcher searcher(c_2_set_two_ones, storage);
   RobustnessCompute robustness(c_2_set_two_ones, storage);

   CheckerSettings settings;
   settings.mark_initals = true;
   settings.minimal = true;

   SynthesisResults results = checker.conductCheck(settings);
   ASSERT_TRUE(results.is_accepting);
   searcher.findWitnesses(results, settings);
   const vector<StateTransition> & trans = searcher.getTransitions();
   EXPECT_FALSE(trans.empty());

   user_options.compute_wintess = user_options.use_long_witnesses = true;
   string witness = WitnessSearcher::getOutput(c_2_set_two_ones, trans);
   EXPECT_TRUE(containsTrans(witness, {"(0,1;0)>(0,0;1)", "(0,0;1)>(1,0;1)", "(1,0;1)>(1,1;1)", "(1,1;1)>(0,1;2)}"}));

   robustness.compute(results, trans, settings);
   EXPECT_DOUBLE_EQ(1., robustness.getRobustness()) << "There is only a single way of the circuit, thus robustness must be 1.";
}

TEST_F(SynthesisTest, CycleOnCircuitCheck) {
   ColorStorage storage(c_2_cyclic);
   ModelChecker checker(c_2_cyclic, storage);
   CheckerSettings settings;
   SynthesisResults reaches;
   SynthesisResults results;

   settings.minimal = true;
   settings.mark_initals = true;
   reaches = checker.conductCheck(settings);
   ASSERT_EQ(3, reaches.found_depth.size());

   settings.mark_initals = false;
   const int STATE_1 = 1, STATE_2 = 7, STATE_3 = 10;

   ASSERT_TRUE(reaches.found_depth.end() != reaches.found_depth.find(STATE_1));
   settings.initial_states = settings.final_states = {STATE_1};
   results = checker.conductCheck(settings);
   EXPECT_EQ(4, results.lower_bound);
   EXPECT_TRUE(results.is_accepting);

   ASSERT_TRUE(reaches.found_depth.end() != reaches.found_depth.find(STATE_2));
   settings.initial_states = settings.final_states = {STATE_2};
   results = checker.conductCheck(settings);
   EXPECT_EQ(INF, results.lower_bound);
   EXPECT_FALSE(results.is_accepting);

   ASSERT_TRUE(reaches.found_depth.end() != reaches.found_depth.find(STATE_3));
   settings.initial_states = settings.final_states = {STATE_3};
   results = checker.conductCheck(settings);
   EXPECT_EQ(4, results.lower_bound);
   EXPECT_TRUE(results.is_accepting);
}

TEST_F(SynthesisTest, CycleOnCircuitAnalysis) {
   ColorStorage storage(c_2_cyclic);
   ModelChecker checker(c_2_cyclic, storage);
   WitnessSearcher searcher(c_2_cyclic, storage);
   RobustnessCompute robustness(c_2_cyclic, storage);
   CheckerSettings settings;
   SynthesisResults results ;

   user_options.compute_wintess = user_options.use_long_witnesses = true;
   const StateID ID = 10; // the state to make the cycle from - it's known to be reachable with robustness of 0.25.
   double robutness_val = 0.;
   string witness;

   settings.minimal = true;
   settings.final_states = {ID};
   settings.mark_initals = true;
   results = checker.conductCheck(settings);
   ASSERT_TRUE(results.is_accepting);
   searcher.findWitnesses(results, settings);
   witness += WitnessSearcher::getOutput(c_2_cyclic, searcher.getTransitions());
   robustness.compute(results, searcher.getTransitions(), settings);
   robutness_val = robustness.getRobustness();

   settings.initial_states = {ID};
   settings.mark_initals = false;
   results = checker.conductCheck(settings);
   ASSERT_TRUE(results.is_accepting);
   searcher.findWitnesses(results, settings);
   witness += WitnessSearcher::getOutput(c_2_cyclic, searcher.getTransitions());
   robustness.compute(results, searcher.getTransitions(), settings);
   robutness_val *= robustness.getRobustness();

   EXPECT_TRUE(containsTrans(witness, {"(1,0;0)>(1,1;1)","(1,1;1)>(0,1;2)","(0,1;2)>(0,0;1)","(0,0;1)>(1,0;0)","(1,0;0)>(1,1;1)"}));
   EXPECT_DOUBLE_EQ(0.25, robutness_val);
}

TEST_F(SynthesisTest, TestBounds) {
   string witness; double robust;
   user_options.bounded_check = true;
   user_options.compute_robustness = true;
   c_2_s_2_o_man->setBFSbound(4);
   EXPECT_EQ(4, c_2_s_2_o_man->checkFinite(witness, robust));
   c_2_s_2_o_man->setBFSbound(3);
   EXPECT_EQ(INF, c_2_s_2_o_man->checkFinite(witness, robust)) << "Should not proceed as the bound is too low.";

   EXPECT_EQ(5, c_2_c_man->checkFull(witness, robust));
   EXPECT_TRUE(containsTrans(witness, {"(1,0;0)>(1,1;1)","(1,1;1)>(0,1;2)","(0,1;2)>(0,0;1)","(0,0;1)>(1,0;0)"}));
   EXPECT_DOUBLE_EQ(0.25, robust) << "Should be reduced only to 25% as the other path is 7 setps long.";

   // ParamNo temp = ModelTranslators::getSpaceSize(one_three);

   user_options.bounded_check = false;
   user_options.bound_size = INF;
}

#endif // SYNTHESIS_TESTS_HPP
