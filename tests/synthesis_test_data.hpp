/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef SYNTHESIS_TEST_DATA_HPP
#define SYNTHESIS_TEST_DATA_HPP

#include "../synthesis/synthesis_manager.hpp"
#include "construction_test_data.hpp"

class SynthesisTest : public StructureTest {
protected:
   unique_ptr<SynthesisManager> o_t_c_man;
   unique_ptr<SynthesisManager> o_t_s_man;
   unique_ptr<SynthesisManager> b_k_c_man;
   unique_ptr<SynthesisManager> b_k_t_man;
   unique_ptr<SynthesisManager> t_man;
   unique_ptr<SynthesisManager> c_2_s_2_o_man;
   unique_ptr<SynthesisManager> c_2_c_man;
   unique_ptr<SynthesisManager> b_k_2_a_p_man;
   unique_ptr<SynthesisManager> b_k_2_s_man;

   void SetUp() override {
      StructureTest::SetUp();

      o_t_c_man.reset(new SynthesisManager(o_t_cyclic, one_three, A_cyclic_prop));
      o_t_s_man.reset(new SynthesisManager(o_t_series, one_three, o_t_series_prop));
      b_k_c_man.reset(new SynthesisManager(b_k_2_cyclic, bool_k_2, A_cyclic_prop));
      b_k_t_man.reset(new SynthesisManager(b_k_2_trivial, bool_k_2, trivial_prop));
      t_man.reset(new SynthesisManager(trivial, trivial_model, trivial_prop));
      c_2_s_2_o_man.reset(new SynthesisManager(c_2_set_two_ones, circuit_2, set_two_ones_prop));
      c_2_c_man.reset(new SynthesisManager(c_2_cyclic, circuit_2, A_cyclic_prop));
      b_k_2_a_p_man.reset(new SynthesisManager(b_k_2_a_peak, bool_k_2, arbitrary_peak_prop));
      b_k_2_s_man.reset(new SynthesisManager(b_k_2_stable, bool_k_2, stable_prop));
   }
};

#endif // SYNTHESIS_TEST_DATA_HPP
