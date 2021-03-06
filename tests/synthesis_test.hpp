/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warranty. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
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

TEST_F(SynthesisTest, AnalysisOnTrivial) {
   vector<StateTransition> witness; double robust;
   for (ParamNo param_no = 0; param_no < KineticsTranslators::getSpaceSize(kin_com_tri); param_no++) {
      sym_com_tri.checkFinite(witness, robust, 1, INF, true, true, 1, INF);
      EXPECT_EQ(1., robust);
   }
}

TEST_F(SynthesisTest, SetTwoOnCircuitFull) {
   ColorStorage storage(pro_cir_one);
   ModelChecker checker(pro_cir_one, storage);
   WitnessSearcher searcher(pro_cir_one, storage);
   RobustnessCompute robustness(pro_cir_one, storage);

   CheckerSettings settings;
   settings.mark_initals = true;
   settings.minimize_cost = true;

   SynthesisResults results = checker.conductCheck(settings);
   ASSERT_TRUE(results.isAccepting());
   searcher.findWitnesses(results, settings);
   const vector<StateTransition> & trans = searcher.getTransitions();
   EXPECT_FALSE(trans.empty());

   string witness = WitnessSearcher::getOutput(true, pro_cir_one, trans);
   EXPECT_TRUE(containsTrans(witness, {"(0,1;0)>(0,0;1)", "(0,0;1)>(1,0;1)", "(1,0;1)>(1,1;1)", "(1,1;1)>(0,1;2)}"}));

   robustness.compute(results, trans, settings);
   EXPECT_DOUBLE_EQ(1., robustness.getRobustness()) << "There is only a single way of the circuit, thus robustness must be 1.";
}

TEST_F(SynthesisTest, CycleOnCircuitCheck) {
   ColorStorage storage(pro_cir_cyc);
   ModelChecker checker(pro_cir_cyc, storage);
   CheckerSettings settings;
   SynthesisResults reaches;
   SynthesisResults results;

   settings.minimize_cost = true;
   settings.mark_initals = true;
   reaches = checker.conductCheck(settings);
   ASSERT_EQ(2, reaches.found_depth.size());

   settings.mark_initals = false;
   const unsigned int STATE_1 = 3, STATE_2 = 6, STATE_3 = 7;

   ASSERT_TRUE(reaches.found_depth.end() == reaches.found_depth.find(STATE_1));

   // Due to synchronicity, it's not possible to cycle from 7 (does not lie on the main circuit).
   ASSERT_TRUE(reaches.found_depth.end() != reaches.found_depth.find(STATE_2));
   settings.initial_states = settings.final_states = {STATE_2};
   results = checker.conductCheck(settings);
   EXPECT_EQ(INF, results.getLowerBound());
   EXPECT_FALSE(results.isAccepting());

   ASSERT_TRUE(reaches.found_depth.end() != reaches.found_depth.find(STATE_3));
   settings.initial_states = settings.final_states = {STATE_3};
   results = checker.conductCheck(settings);
   EXPECT_EQ(4, results.getLowerBound());
   EXPECT_TRUE(results.isAccepting());
}

TEST_F(SynthesisTest, CycleOnCircuitAnalysis) {
   ColorStorage storage(pro_cir_cyc);
   ModelChecker checker(pro_cir_cyc, storage);
   WitnessSearcher searcher(pro_cir_cyc, storage);
   RobustnessCompute robustness(pro_cir_cyc, storage);
   CheckerSettings settings;
   SynthesisResults results ;

   const StateID ID = 10; // the state to make the cycle from - it's known to be reachable with robustness of 0.25.
   double robutness_val = 0.;
   string witness;

   settings.minimize_cost = true;
   settings.final_states = {ID};
   settings.mark_initals = true;
   results = checker.conductCheck(settings);
   ASSERT_TRUE(results.isAccepting());
   searcher.findWitnesses(results, settings);
   witness += WitnessSearcher::getOutput(true, pro_cir_cyc, searcher.getTransitions());
   robustness.compute(results, searcher.getTransitions(), settings);
   robutness_val = robustness.getRobustness();

   settings.initial_states = {ID};
   settings.mark_initals = false;
   results = checker.conductCheck(settings);
   ASSERT_TRUE(results.isAccepting());
   searcher.findWitnesses(results, settings);
   witness += WitnessSearcher::getOutput(true, pro_cir_cyc, searcher.getTransitions());
   robustness.compute(results, searcher.getTransitions(), settings);
   robutness_val *= robustness.getRobustness();

   EXPECT_TRUE(containsTrans(witness, {"(1,0;0)>(1,1;1)","(1,1;1)>(0,1;2)","(0,1;2)>(0,0;1)","(0,0;1)>(1,0;0)"}));
   EXPECT_DOUBLE_EQ(0.25, robutness_val);
}

TEST_F(SynthesisTest, TestPeakOnCircuit) {
   // Change to the K_2
   vector<StateTransition> witness; double robust;
   vector<string> witnesses;
   vector<double> robustnesses;
   bool full_found = false; // There exists a full branch path
   for (ParamNo param_no = 0; param_no < KineticsTranslators::getSpaceSize(kin_com_tri); param_no++) {
      size_t cost = sym_com_top.checkFull(witness, robust, param_no, INF, true, true);
      if (cost == 4) {
         witnesses.push_back(WitnessSearcher::getOutput(true, pro_com_top, witness));
         witness.clear();
         robustnesses.push_back(robust);
         full_found |= containsTrans(witnesses.back(), {"(1,0;0)>(1,1;1)","(0,1;0)>(1,1;1)","(1,1;1)>(1,0;2)","(1,1;1)>(0,1;2)"});
      }
   }
   EXPECT_TRUE(full_found);
   EXPECT_EQ(witnesses.size(),robustnesses.size());
   // None of the witnesses is empty.
   for(const string & wit:witnesses) {
      EXPECT_FALSE(wit.empty());
   }
   // All the robustnesses are in the probability range.
   for(const double rob:robustnesses) {
      EXPECT_PRED1([](const double val){return (val >= 0. && val <= 1.);}, rob);
   }
}

TEST_F(SynthesisTest, TestBounds) {
   vector<StateTransition> witness; double robust;
   EXPECT_EQ(4, sym_cir_one.checkFinite(witness, robust, 1, 4, false, false, 1, INF));
   witness.clear();
   EXPECT_EQ(INF, sym_cir_one.checkFinite(witness, robust, 1, 3, false, false, 1, INF)) << "Should not proceed as the bound is too low.";

   witness.clear();
   EXPECT_EQ(5, sym_cir_cyc.checkFull(witness, robust, 1, INF, true, true));
   EXPECT_TRUE(containsTrans(WitnessSearcher::getOutput(true, pro_cir_cyc, witness), {"(1,0;0)>(1,1;1)","(1,1;1)>(0,1;2)","(0,1;2)>(0,0;1)","(0,0;1)>(1,0;0)"}));
   EXPECT_DOUBLE_EQ(0.25, robust) << "Should be reduced only to 25% as the other path is 7 setps long.";

   vector<size_t> sizes1;
   vector<size_t> sizes2;
   for (ParamNo param_no = 0; param_no < KineticsTranslators::getSpaceSize(kin_com_tri); param_no++) {
      sizes1.push_back(sym_com_cyc.checkFull(witness, robust, param_no, INF, true, true));
	  sizes2.push_back(sym_com_cyc.checkFull(witness, robust, param_no, 3, true, true));
   }
   EXPECT_EQ(count(sizes2.begin(), sizes2.end(), INF),
             count_if(sizes1.begin(), sizes1.end(),[](const size_t val){return val > 3;}));
}

TEST_F(SynthesisTest, TestStable) {
	vector<StateTransition> witness; double robust;
	for (ParamNo param_no = 0; param_no < KineticsTranslators::getSpaceSize(kin_com_sta); param_no++) {
		sym_com_sta.checkFinite(witness, robust, param_no, INF, true, true, 1, INF);

		// First transition must be transient, second stable.
		bool correct = true;
		for (StateTransition & trans : witness)
		if (pro_com_sta.getBAID(trans.first) == 0)
			correct &= (pro_com_sta.getKSID(trans.first) != (pro_com_sta.getKSID(trans.second)));
		else if (pro_com_sta.getBAID(trans.first) == 1)
			correct &= (pro_com_sta.getKSID(trans.first) == (pro_com_sta.getKSID(trans.second)));

		EXPECT_TRUE(correct);
	}
}

TEST_F(SynthesisTest, TestBistable) {
	vector<StateTransition> witness; double robust;
	for (ParamNo param_no = 0; param_no < KineticsTranslators::getSpaceSize(kin_com_bst); param_no++) {
		EXPECT_EQ(2u, ltl_bst.getMinAcc());
		EXPECT_EQ(INF, ltl_bst.getMaxAcc());
		sym_com_bst.checkFinite(witness, robust, param_no, INF, true, true, ltl_bst.getMinAcc(), ltl_bst.getMaxAcc());
		if (!witness.empty())
			EXPECT_TRUE(containsTrans(WitnessSearcher::getOutput(true, pro_cir_cyc, witness), { "(0,1;1)>(0,1;2)", "(1,0;1)>(1,0;2)" }));
	}
}

TEST_F(SynthesisTest, TestExperiment) {
	for (ParamNo param_no = 0; param_no < KineticsTranslators::getSpaceSize(kin_cir_exp); param_no++) {
		vector<StateTransition> witness_path; double robust;
		sym_cir_exp.checkFull(witness_path, robust, param_no, INF, true, true);
		string witness = WitnessSearcher::getOutput(true, pro_cir_exp, witness_path);
		// Experiment requires B to be 1 all the time
		EXPECT_TRUE(witness.find("0,0") == string::npos);
		EXPECT_TRUE(witness.find("1,0") == string::npos);
	}
}



#endif // SYNTHESIS_TESTS_HPP
