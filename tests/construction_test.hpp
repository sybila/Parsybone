/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warranty. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef CONSTRUCTION_TEST_H
#define CONSTRUCTION_TEST_H

#include "construction_test_data.hpp"

TEST_F(StructureTest, TestMultiedge) {
	Model mod_tri;
	mod_tri.addSpecie("A", 2, Model::Component);
	mod_tri.addRegulation(0, 0, 1, "-");
	mod_tri.addRegulation(0, 0, 2, "!-");
	EXPECT_NO_THROW(ConstructionManager::computeKinetics(mod_tri, ltl_tri));
}

TEST_F(StructureTest, TestCorrectAutomata) {
	AutomatonBuilder aub_mul_mul(mod_mul, ltl_mul);
	AutomatonStructure aus_mul_mul = aub_mul_mul.buildAutomaton();

	ASSERT_EQ(1, aus_mul_mul.getInitialStates().size());
	EXPECT_EQ(0, aus_mul_mul.getInitialStates().front());
	ASSERT_EQ(1, aus_mul_mul.getTransitionCount(0)) << "One outgoing transition for the intial state of o_t_series.";
	ASSERT_EQ(3, aus_mul_mul.getStateCount());
	ASSERT_EQ(1, aus_mul_mul.getFinalStates().size());
	EXPECT_EQ(2, aus_mul_mul.getFinalStates().front());

	AutomatonBuilder aub_mul_cyc(mod_mul, ltl_cyc);
	AutomatonStructure aus_mul_cyc = aub_mul_cyc.buildAutomaton();
	ASSERT_EQ(1, aus_mul_cyc.getInitialStates().size());
	EXPECT_EQ(0, aus_mul_cyc.getInitialStates().front());
	ASSERT_EQ(2, aus_mul_cyc.getTransitionCount(0)) << "Two outgoing transitions for the intial state of o_t_cyclic.";
	ASSERT_EQ(3, aus_mul_cyc.getStateCount());
	ASSERT_EQ(1, aus_mul_cyc.getFinalStates().size());
	EXPECT_EQ(1, aus_mul_cyc.getFinalStates().front());

	AutomatonBuilder aub_mul_tri(mod_tri, ltl_tri);
	AutomatonStructure aub_tri_aut = aub_mul_tri.buildAutomaton();
	ASSERT_EQ(1, aub_tri_aut.getInitialStates().size());
	EXPECT_EQ(0, aub_tri_aut.getInitialStates().front());
	ASSERT_EQ(1, aub_tri_aut.getTransitionCount(0)) << "Only self-loop expected for aub_tri_aut.";
	ASSERT_EQ(1, aub_tri_aut.getStateCount());
	ASSERT_EQ(1, aub_tri_aut.getFinalStates().size());
	EXPECT_EQ(0, aub_tri_aut.getFinalStates().front());
}

TEST_F(StructureTest, TestCorrectUnparametrizedStucture) {
	Kinetics kin_mul_tri = ConstructionManager::computeKinetics(mod_mul, ltl_tri);
	UnparametrizedStructureBuilder usb_mul_tri(mod_mul, ltl_tri, kin_mul_tri);
	UnparametrizedStructure ust_mul_tri = usb_mul_tri.buildStructure();
	ASSERT_EQ(8, ust_mul_tri.getStateCount());
	EXPECT_EQ(0, ust_mul_tri.getStateLevels(0).front());
	EXPECT_EQ(3, ust_mul_tri.getStateLevels(7).back());
	ASSERT_EQ(2, ust_mul_tri.getTransitionCount(0)) << "Exactly two transitions should be possible from (0,0) .";

	UnparametrizedStructureBuilder usb_com_tri(mod_com, ltl_tri, kin_com_tri);
	UnparametrizedStructure ust_com_tri = usb_com_tri.buildStructure();
	ASSERT_EQ(4, ust_com_tri.getStateCount());
	EXPECT_EQ(0, ust_com_tri.getStateLevels(0).front());
	EXPECT_EQ(1, ust_com_tri.getStateLevels(3).back());
	ASSERT_EQ(2, ust_com_tri.getTransitionCount(0)) << "Exactly two transitions should be possible from (0,0) ";
	EXPECT_EQ(16, ust_com_tri.getTransitionConst(0, 0).step_size);
	EXPECT_EQ(1, ust_com_tri.getTransitionConst(0, 1).step_size);
}

TEST_F(StructureTest, TestCorrectProduct) {
	ASSERT_EQ(12, pro_com_cyc.getStateCount());
	ASSERT_EQ(4, pro_com_cyc.getInitialStates().size());
	EXPECT_TRUE(pro_com_cyc.isInitial(0));
	ASSERT_EQ(4, pro_com_cyc.getFinalStates().size());
	EXPECT_EQ(6, pro_com_cyc.getProductID(2, 1));
	EXPECT_EQ(2, pro_com_cyc.getKSID(6));
	EXPECT_EQ(1, pro_com_cyc.getBAID(6));

	EXPECT_EQ(1, pro_cir_one.getInitialStates().size());
	EXPECT_TRUE(pro_cir_one.isInitial(pro_cir_one.getProductID(2, 0))) << "Only (0,1;0) should be initial.";
	EXPECT_EQ(4, pro_cir_one.getFinalStates().size()) << "All possible TS states should have final version.";

	EXPECT_EQ(4, pro_cir_cyc.getInitialStates().size()) << "All possible TS states should have initial version.";
	EXPECT_EQ(4, pro_cir_cyc.getFinalStates().size()) << "All possible TS states should have final version.";

	ASSERT_EQ(2, pro_tri_tri.getStateCount());
}

#endif // CONSTRUCTION_TEST_H
