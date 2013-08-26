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

TEST_F(SynthesisTest, SetTwoOnCircuitTest) {
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
   EXPECT_TRUE([&witness](){
      for (const string & trans : {"(0,1;0)>(0,0;1)", "(0,0;1)>(1,0;1)", "(1,0;1)>(1,1;1)", "(1,1;1)>(0,1;2)}"})
         if (witness.find(trans) == witness.npos)
            return false;
      return true;
   }()) << "All required transitions are present.";

   robustness.compute(results, trans, settings);
   EXPECT_DOUBLE_EQ(1., robustness.getRobustness()) << "There is only a single way of the circuit, thus robustness must be 1.";
}

TEST_F(SynthesisTest, CycleOnCircuitTest) {
   ColorStorage storage(c_2_cyclic);
   ModelChecker checker(c_2_cyclic, storage);
   WitnessSearcher searcher(c_2_cyclic, storage);
   RobustnessCompute robustness(c_2_cyclic, storage);

   CheckerSettings settings;
   settings.mark_initals = true;
   settings.minimal = false;

   SynthesisResults results = checker.conductCheck(settings);
   ASSERT_TRUE(results.is_accepting);

   // Test how it would work for witnesses in finite check.
   searcher.findWitnesses(results, settings);
   const vector<StateTransition> & trans = searcher.getTransitions();
   EXPECT_FALSE(trans.empty());

   user_options.compute_wintess = user_options.use_long_witnesses = true;
   string witness = WitnessSearcher::getOutput(c_2_cyclic, trans);
   EXPECT_TRUE([&witness](){
      for (const string & trans : {"(1,0;0)>(1,1;1)", "(1,1;0)>(0,1;1)}"})
         if (witness.find(trans) == witness.npos)
            return false;
      return true;
   }()) << "All required transitions are present.";

   robustness.compute(results, trans, settings);
   EXPECT_DOUBLE_EQ(.5, robustness.getRobustness()) << "Only half of the initial states make the step, thus robustness should be 0.5.";

   map<StateID, size_t> finals = results.found_depth;
   double robutness_val = 0.;
   witness = "";
   for (const pair<StateID, size_t> & final : finals) {
      settings.minimal = true;
      settings.initial_states = {final.first};
      settings.final_states = {final.first};
      settings.mark_initals = false;

      results = checker.conductCheck(settings);
      if (results.is_accepting) {
         if (user_options.analysis()) {
            searcher.findWitnesses(results, settings);
            robustness.compute(results, searcher.getTransitions(), settings);
            double robust_reach = robustness.getRobustness();
            witness += WitnessSearcher::getOutput(c_2_cyclic, searcher.getTransitions());

            settings.mark_initals = true;
            settings.initial_states.clear();
            results = checker.conductCheck(settings);

            searcher.findWitnesses(results, settings);
            robustness.compute(results, searcher.getTransitions(), settings);
            robust_reach *= robustness.getRobustness();
            robutness_val += robust_reach;
            witness += WitnessSearcher::getOutput(c_2_cyclic, searcher.getTransitions());
         }
      }
   }
   EXPECT_GE(1., robutness_val);
}


#endif // SYNTHESIS_TESTS_HPP
