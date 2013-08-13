/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

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
   ProductStructure c_2_set_two_ones;
   ProductStructure c_2_cyclic;
   ProductStructure trivial;

   void SetUp() override {
      ModelsTest::SetUp();

      ConstructionManager::computeModelProps(trivial_model);
      ConstructionManager::computeModelProps(one_three);
      ConstructionManager::computeModelProps(bool_k_2);
      ConstructionManager::computeModelProps(circuit_2);

      o_t_series = ConstructionManager::construct(one_three, o_t_series_prop);
      o_t_cyclic = ConstructionManager::construct(one_three, A_cyclic_prop);
      b_k_2_cyclic = ConstructionManager::construct(bool_k_2, A_cyclic_prop);
      b_k_2_trivial = ConstructionManager::construct(bool_k_2, trivial_prop);
      trivial = ConstructionManager::construct(trivial_model, trivial_prop);
      c_2_set_two_ones = ConstructionManager::construct(circuit_2, set_two_ones_prop);
      c_2_cyclic = ConstructionManager::construct(circuit_2, A_cyclic_prop);
   }
};

#endif // TESTING_STRUCTURE_SOURCE_HPP
