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
#include "model_test_data.hpp"

class StructureTest : public ModelsTest {
protected:
   ProductStructure o_t_series;
   ProductStructure o_t_cyclic;
   ProductStructure b_k_2_cyclic;
   ProductStructure b_k_2_trivial;
   ProductStructure c_2_set_two_ones;
   ProductStructure c_2_cyclic;
   ProductStructure b_k_2_a_peak;
   ProductStructure trivial;
   ProductStructure b_k_2_stable;
   ProductStructure b_k_2_bistable;
   ProductStructure c_2_experiment;

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
      b_k_2_a_peak = ConstructionManager::construct(bool_k_2, arbitrary_peak_prop);
      b_k_2_stable = ConstructionManager::construct(bool_k_2, stable_prop);
      b_k_2_bistable = ConstructionManager::construct(bool_k_2, bistable_prop);
	  c_2_experiment = ConstructionManager::construct(circuit_2, experiment_series);
   }
};

#endif // TESTING_STRUCTURE_SOURCE_HPP
