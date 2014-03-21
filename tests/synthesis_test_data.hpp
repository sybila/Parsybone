/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warranty. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef SYNTHESIS_TEST_DATA_HPP
#define SYNTHESIS_TEST_DATA_HPP

#include "../synthesis/synthesis_manager.hpp"
#include "construction_test_data.hpp"

class SynthesisTest : public StructureTest {
protected:
	SynthesisManager sym_cir_one;
	SynthesisManager sym_cir_cyc;
	SynthesisManager sym_cir_exp;
	SynthesisManager sym_com_bst;
	SynthesisManager sym_com_cyc;
	SynthesisManager sym_com_sta;
	SynthesisManager sym_com_top;
	SynthesisManager sym_com_tri;
	SynthesisManager sym_mul_cyc;
	SynthesisManager sym_mul_mul;
	SynthesisManager sym_tri_tri;

	void SetUp() override {
		StructureTest::SetUp();

		sym_mul_cyc = SynthesisManager(pro_mul_cyc);
		sym_mul_mul = SynthesisManager(pro_mul_mul);
		sym_com_cyc = SynthesisManager(pro_com_cyc);
		sym_com_tri = SynthesisManager(pro_com_tri);
		sym_tri_tri = SynthesisManager(pro_tri_tri);
		sym_cir_one = SynthesisManager(pro_cir_one);
		sym_cir_cyc = SynthesisManager(pro_cir_cyc);
		sym_com_top = SynthesisManager(pro_com_top);
		sym_com_sta = SynthesisManager(pro_com_sta);
		sym_com_bst = SynthesisManager(pro_com_bst);
		sym_cir_exp = SynthesisManager(pro_cir_exp);
	}
};

#endif // SYNTHESIS_TEST_DATA_HPP
