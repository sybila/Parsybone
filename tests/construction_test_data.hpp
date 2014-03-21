/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warranty. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef TESTING_STRUCTURE_SOURCE_HPP
#define TESTING_STRUCTURE_SOURCE_HPP

#include "../construction/construction_manager.hpp"
#include "kinetics_test_data.hpp"

class StructureTest : public KineticsTest {
protected:
	ProductStructure pro_cas_one;
	ProductStructure pro_cir_one;
	ProductStructure pro_cir_cyc;
	ProductStructure pro_cir_exp;
	ProductStructure pro_com_bst;
	ProductStructure pro_com_cyc;
	ProductStructure pro_com_sta;
	ProductStructure pro_com_top;
	ProductStructure pro_com_tri;
	ProductStructure pro_mul_cyc;
	ProductStructure pro_mul_mul;
	ProductStructure pro_tri_tri;

	void SetUp() override {
		KineticsTest::SetUp();

		pro_cas_one = ConstructionManager::construct(mod_cas, ltl_one, kin_cas_one);
		pro_cir_cyc = ConstructionManager::construct(mod_cir, ltl_cyc, kin_cir_cyc);
		pro_cir_exp = ConstructionManager::construct(mod_cir, ltl_exp, kin_cir_exp);
		pro_cir_one = ConstructionManager::construct(mod_cir, ltl_one, kin_cir_one);
		pro_com_bst = ConstructionManager::construct(mod_com, ltl_bst, kin_com_bst);
		pro_com_cyc = ConstructionManager::construct(mod_com, ltl_cyc, kin_com_cyc);
		pro_com_sta = ConstructionManager::construct(mod_com, ltl_sta, kin_com_sta);
		pro_com_top = ConstructionManager::construct(mod_com, ltl_top, kin_com_top);
		pro_com_tri = ConstructionManager::construct(mod_com, ltl_tri, kin_com_tri);
		pro_mul_mul = ConstructionManager::construct(mod_mul, ltl_mul, kin_mul_mul);
		pro_mul_cyc = ConstructionManager::construct(mod_mul, ltl_cyc, kin_mul_cyc);
		pro_tri_tri = ConstructionManager::construct(mod_tri, ltl_tri, kin_tri_tri);
	}
};

#endif // TESTING_STRUCTURE_SOURCE_HPP
