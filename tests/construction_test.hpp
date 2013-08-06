#ifndef CONSTRUCTION_TEST_H
#define CONSTRUCTION_TEST_H

#include "construction_test_data.hpp"

TEST_F(StructureTest, TestCorrectAutomata) {
   ASSERT_EQ(1, o_t_series.getAutomaton().getInitialStates().size());
   EXPECT_EQ(0, o_t_series.getAutomaton().getInitialStates().front());
   ASSERT_EQ(2, o_t_series.getAutomaton().getTransitionCount(0)) << "Two outgoing transitions for the intial state of o_t_series.";
   ASSERT_EQ(3, o_t_series.getAutomaton().getStateCount());
   ASSERT_EQ(1, o_t_series.getAutomaton().getFinalStates().size());
   EXPECT_EQ(2, o_t_series.getAutomaton().getFinalStates().front());

   ASSERT_EQ(1, o_t_cyclic.getAutomaton().getInitialStates().size());
   EXPECT_EQ(0, o_t_cyclic.getAutomaton().getInitialStates().front());
   ASSERT_EQ(2, o_t_cyclic.getAutomaton().getTransitionCount(0)) << "Two outgoing transitions for the intial state of o_t_cyclic.";
   ASSERT_EQ(3, o_t_cyclic.getAutomaton().getStateCount());
   ASSERT_EQ(1, o_t_cyclic.getAutomaton().getFinalStates().size());
   EXPECT_EQ(1, o_t_cyclic.getAutomaton().getFinalStates().front());

   ASSERT_EQ(1, trivial.getAutomaton().getInitialStates().size());
   EXPECT_EQ(0, trivial.getAutomaton().getInitialStates().front());
   ASSERT_EQ(1, trivial.getAutomaton().getTransitionCount(0)) << "Only self-loop expected for trivial_prop.";
   ASSERT_EQ(1, trivial.getAutomaton().getStateCount());
   ASSERT_EQ(1, trivial.getAutomaton().getFinalStates().size());
   EXPECT_EQ(0, trivial.getAutomaton().getFinalStates().front());
}

TEST_F(StructureTest, TestCorrectUnparametrizedStucture) {
   ASSERT_EQ(8, o_t_cyclic.getUnparametrizedStructure().getStateCount());
   EXPECT_EQ(0, o_t_cyclic.getUnparametrizedStructure().getStateLevels(0).front());
   EXPECT_EQ(3, o_t_cyclic.getUnparametrizedStructure().getStateLevels(7).back());
   ASSERT_EQ(4, o_t_cyclic.getUnparametrizedStructure().getTransitionCount(0)) << "Exactly four transitions should be possible from (0,0) (self-loop may be over both components).";

   ASSERT_EQ(4, b_k_2_trivial.getUnparametrizedStructure().getStateCount());
   EXPECT_EQ(0, b_k_2_trivial.getUnparametrizedStructure().getStateLevels(0).front());
   EXPECT_EQ(1, b_k_2_trivial.getUnparametrizedStructure().getStateLevels(3).back());
   ASSERT_EQ(4, b_k_2_trivial.getUnparametrizedStructure().getTransitionCount(0)) << "Exactly four transitions should be possible from (0,0) (self-loop may be over both components).";
   EXPECT_EQ(1, b_k_2_trivial.getUnparametrizedStructure().getStepSize(0, 0));
   EXPECT_EQ(16, b_k_2_trivial.getUnparametrizedStructure().getStepSize(0, 3));
}

TEST_F(StructureTest, TestCorrectProduct) {
   ASSERT_EQ(12, b_k_2_cyclic.getProduct().getStateCount());
   ASSERT_EQ(4, b_k_2_cyclic.getProduct().getInitialStates().size());
   EXPECT_TRUE(b_k_2_cyclic.getProduct().isInitial(0));
   ASSERT_EQ(4, b_k_2_cyclic.getProduct().getFinalStates().size());
   EXPECT_EQ(7, b_k_2_cyclic.getProduct().getProductID(2,1));
   EXPECT_EQ(2, b_k_2_cyclic.getProduct().getKSID(7));
   EXPECT_EQ(1, b_k_2_cyclic.getProduct().getBAID(7));

   ASSERT_EQ(2, trivial.getProduct().getStateCount());
}

#endif // CONSTRUCTION_TEST_H
