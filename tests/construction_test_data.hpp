#ifndef TESTING_STRUCTURE_SOURCE_HPP
#define TESTING_STRUCTURE_SOURCE_HPP

#include "../construction/construction_manager.hpp"
#include "model_test_data.hpp"

class StructureTest : public ModelsTest {
public:
   ConstructionHolder series;
   ConstructionHolder cyclic;

   void SetUp() override {
      ModelsTest::SetUp();

      series = ConstructionManager::construct(basic_model, series_aut);
      cyclic = ConstructionManager::construct(basic_model, cyclic_aut);
   }
};

#endif // TESTING_STRUCTURE_SOURCE_HPP
