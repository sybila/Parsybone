#ifndef SYNTHESIS_TEST_DATA_HPP
#define SYNTHESIS_TEST_DATA_HPP

#include "../synthesis/synthesis_manager.hpp"
#include "construction_test_data.hpp"

class SynthesisTest : public StructureTest {
   unique_ptr<SynthesisManager> o_t_s_man;
   unique_ptr<SynthesisManager> o_t_c_man;
   unique_ptr<SynthesisManager> b_k_c_man;
   unique_ptr<SynthesisManager> b_k_t_man;
   unique_ptr<SynthesisManager> t_man;

   void SetUp() override {
      StructureTest::SetUp();

      o_t_s_man.reset(new SynthesisManager(o_t_cyclic, one_three, cA_cyclic_aut));
      o_t_c_man.reset(new SynthesisManager(o_t_series, one_three, o_t_series_aut));
      b_k_c_man.reset(new SynthesisManager(b_k_2_cyclic, bool_k_2, cA_cyclic_aut));
      b_k_t_man.reset(new SynthesisManager(b_k_2_trivial, bool_k_2, trivial_aut));
      t_man.reset(new SynthesisManager(trivial, trivial_model, trivial_aut));
   }
};

#endif // SYNTHESIS_TEST_DATA_HPP
