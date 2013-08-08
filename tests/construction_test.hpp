/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef CONSTRUCTION_TEST_H
#define CONSTRUCTION_TEST_H

#include "construction_test_data.hpp"

TEST_F(StructureTest, TestCorrectAutomata) {
   AutomatonBuilder o_t_series_builder(one_three, o_t_series_prop);
   AutomatonStructure o_t_series_aut = o_t_series_builder.buildAutomaton();

   ASSERT_EQ(1, o_t_series_aut.getInitialStates().size());
   EXPECT_EQ(0, o_t_series_aut.getInitialStates().front());
   ASSERT_EQ(2, o_t_series_aut.getTransitionCount(0)) << "Two outgoing transitions for the intial state of o_t_series.";
   ASSERT_EQ(3, o_t_series_aut.getStateCount());
   ASSERT_EQ(1, o_t_series_aut.getFinalStates().size());
   EXPECT_EQ(2, o_t_series_aut.getFinalStates().front());

   AutomatonBuilder o_t_cyclic_builder(one_three, cA_cyclic_prop);
   AutomatonStructure o_t_cyclic_aut = o_t_cyclic_builder.buildAutomaton();
   ASSERT_EQ(1, o_t_cyclic_aut.getInitialStates().size());
   EXPECT_EQ(0, o_t_cyclic_aut.getInitialStates().front());
   ASSERT_EQ(2, o_t_cyclic_aut.getTransitionCount(0)) << "Two outgoing transitions for the intial state of o_t_cyclic.";
   ASSERT_EQ(3, o_t_cyclic_aut.getStateCount());
   ASSERT_EQ(1, o_t_cyclic_aut.getFinalStates().size());
   EXPECT_EQ(1, o_t_cyclic_aut.getFinalStates().front());

   AutomatonBuilder o_t_trivial_builder(trivial_model, trivial_prop);
   AutomatonStructure trivial_aut = o_t_trivial_builder.buildAutomaton();
   ASSERT_EQ(1, trivial_aut.getInitialStates().size());
   EXPECT_EQ(0, trivial_aut.getInitialStates().front());
   ASSERT_EQ(1, trivial_aut.getTransitionCount(0)) << "Only self-loop expected for trivial_aut.";
   ASSERT_EQ(1, trivial_aut.getStateCount());
   ASSERT_EQ(1, trivial_aut.getFinalStates().size());
   EXPECT_EQ(0, trivial_aut.getFinalStates().front());
}

TEST_F(StructureTest, TestCorrectUnparametrizedStucture) {
   BasicStructureBuilder o_t_b_s_builder(one_three);
   BasicStructure o_t_b_s = o_t_b_s_builder.buildStructure();
   UnparametrizedStructureBuilder o_t_u_s_builder(one_three, o_t_b_s);
   UnparametrizedStructure o_t_u_s = o_t_u_s_builder.buildStructure();
   ASSERT_EQ(8, o_t_u_s.getStateCount());
   EXPECT_EQ(0, o_t_u_s.getStateLevels(0).front());
   EXPECT_EQ(3, o_t_u_s.getStateLevels(7).back());
   ASSERT_EQ(4, o_t_u_s.getTransitionCount(0)) << "Exactly four transitions should be possible from (0,0) (self-loop may be over both components).";

   BasicStructureBuilder b_k_2_b_s_builder(bool_k_2);
   BasicStructure b_k_2_b_s = b_k_2_b_s_builder.buildStructure();
   UnparametrizedStructureBuilder b_k_2_u_s_builder(bool_k_2, b_k_2_b_s);
   UnparametrizedStructure b_k_2_u_s = b_k_2_u_s_builder.buildStructure();
   ASSERT_EQ(4, b_k_2_u_s.getStateCount());
   EXPECT_EQ(0, b_k_2_u_s.getStateLevels(0).front());
   EXPECT_EQ(1, b_k_2_u_s.getStateLevels(3).back());
   ASSERT_EQ(4, b_k_2_u_s.getTransitionCount(0)) << "Exactly four transitions should be possible from (0,0) (self-loop may be over both components).";
   EXPECT_EQ(1, b_k_2_u_s.getStepSize(0, 0));
   EXPECT_EQ(16, b_k_2_u_s.getStepSize(0, 3));
}

TEST_F(StructureTest, TestCorrectProduct) {
   ASSERT_EQ(12, b_k_2_cyclic.getStateCount());
   ASSERT_EQ(4, b_k_2_cyclic.getInitialStates().size());
   EXPECT_TRUE(b_k_2_cyclic.isInitial(0));
   ASSERT_EQ(4, b_k_2_cyclic.getFinalStates().size());
   EXPECT_EQ(7, b_k_2_cyclic.getProductID(2,1));
   EXPECT_EQ(2, b_k_2_cyclic.getKSID(7));
   EXPECT_EQ(1, b_k_2_cyclic.getBAID(7));

   ASSERT_EQ(2, trivial.getStateCount());
}

#endif // CONSTRUCTION_TEST_H
