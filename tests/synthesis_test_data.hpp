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
   SynthesisManager o_t_c_man;
   SynthesisManager o_t_s_man;
   SynthesisManager b_k_c_man;
   SynthesisManager b_k_t_man;
   SynthesisManager t_man;
   SynthesisManager c_2_s_2_o_man;
   SynthesisManager c_2_c_man;
   SynthesisManager b_k_2_a_p_man;
   SynthesisManager b_k_2_s_man;
   SynthesisManager b_k_2_bs_man;
   SynthesisManager c_2_e_man;

   void SetUp() override {
      StructureTest::SetUp();

      o_t_c_man = SynthesisManager(o_t_cyclic);
      o_t_s_man = SynthesisManager(o_t_series);
      b_k_c_man = SynthesisManager(b_k_2_cyclic);
      b_k_t_man = SynthesisManager(b_k_2_trivial);
      t_man = SynthesisManager(trivial);
      c_2_s_2_o_man = SynthesisManager(c_2_set_two_ones);
      c_2_c_man = SynthesisManager(c_2_cyclic);
      b_k_2_a_p_man = SynthesisManager(b_k_2_a_peak);
      b_k_2_s_man = SynthesisManager(b_k_2_stable);
      b_k_2_bs_man = SynthesisManager(b_k_2_bistable);
	  c_2_e_man = SynthesisManager(c_2_experiment);
   }
};

#endif // SYNTHESIS_TEST_DATA_HPP
