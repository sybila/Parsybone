#ifndef TESTING_STRUCTURE_SOURCE_HPP
#define TESTING_STRUCTURE_SOURCE_HPP

#include "../construction/construction_manager.hpp"
#include "model_test_data.hpp"

class StructureTest : public ModelsTest {
protected:
   ConstructionHolder series;
   ConstructionHolder cyclic;
   ConstructionHolder trivial;

   void SetUp() override {
      ModelsTest::SetUp();

      series = ConstructionManager::construct(basic_model, series_aut);
      cyclic = ConstructionManager::construct(basic_model, cyclic_aut);
      trivial = ConstructionManager::construct(trivial_model, trivial_prop);
   }
};

#endif // TESTING_STRUCTURE_SOURCE_HPP
