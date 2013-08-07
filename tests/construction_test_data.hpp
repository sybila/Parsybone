#ifndef TESTING_STRUCTURE_SOURCE_HPP
#define TESTING_STRUCTURE_SOURCE_HPP

#include "../construction/construction_manager.hpp"
#include "model_test_data.hpp"

class StructureTest : public ModelsTest {
protected:
   ProductStructure o_t_series;
   ProductStructure o_t_cyclic;
   ProductStructure b_k_2_cyclic;
   ProductStructure b_k_2_trivial;
   ProductStructure trivial;

   void SetUp() override {
      ModelsTest::SetUp();

      ConstructionManager::computeModelProps(trivial_model);
      ConstructionManager::computeModelProps(one_three);
      ConstructionManager::computeModelProps(bool_k_2);

      o_t_series = ConstructionManager::construct(one_three, o_t_series_prop);
      o_t_cyclic = ConstructionManager::construct(one_three, cA_cyclic_prop);
      b_k_2_cyclic = ConstructionManager::construct(bool_k_2, cA_cyclic_prop);
      b_k_2_trivial = ConstructionManager::construct(bool_k_2, trivial_prop);
      trivial = ConstructionManager::construct(trivial_model, trivial_prop);
   }
};

#endif // TESTING_STRUCTURE_SOURCE_HPP
