/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef MODEL_FUNCTIONS_TEST_HPP
#define MODEL_FUNCTIONS_TEST_HPP

#include "model_test_data.hpp"
#include "../model/parameter_reader.hpp"
#include "../model/regulation_helper.hpp"

/// Test various functions the model class posseses.
TEST_F(ModelsTest, ModelFunctions) {
   auto tresholds = ModelTranslators::getThresholds(one_three,0);
   ASSERT_EQ(2u, tresholds.size());
   ASSERT_EQ(3u, tresholds.find(1)->second[1]);

   ASSERT_EQ(2, ModelTranslators::getRegulatorsIDs(one_three, 0).size()) << "There must be only two IDs for regulations, even though there are three incoming interactions.";
}

/// Controls whether explicit parametrizaitions do replace the original values.
TEST_F(ModelsTest, ParametrizationControl) {

   one_three.species[0].params_specs.k_pars.push_back(make_pair("A,B:1","1"));
   one_three.species[0].params_specs.k_pars.push_back(make_pair("B:3","0"));
   one_three.species[1].params_specs.k_pars.push_back(make_pair("A","3"));

   // Transform the description into semantics.
   ASSERT_NO_THROW(ParameterHelper::fillParameters(one_three));
   ASSERT_NO_THROW(ParameterReader::constrainParameters(one_three));

   auto params = one_three.getParameters(0);
   ASSERT_EQ(6, params.size()) << "There should be 6 contexts for A.";
   for (const auto & param:params) {
      if (param.context.compare("A:1,B:1") == 0) {
         ASSERT_EQ(1, param.targets.size()) << "One possible target val in this context.";
         EXPECT_EQ(1, param.targets[0]) << "Target value in given countext should be one.";
      }
      if (param.context.compare("A:0,B:1") == 0) {
         ASSERT_EQ(2, param.targets.size()) << "Target values should not be constrained.";
      }
   }
}

/// This test controls functionality of loop bounding constraint.
TEST_F(ModelsTest, ParametrizationExtremal) {
   circuit_2.addSpecie("C", 1, range<ActLevel>(2u));
   circuit_2.addRegulation(2, 0, 1, "+");
   circuit_2.restrictions.force_extremes = true;
   ConstructionManager::computeModelProps(circuit_2);

   const Model::Parameter & full_act_1 = ModelTranslators::matchContext(circuit_2, "B:0,C:1", 0);
   ASSERT_EQ(1, full_act_1.targets.size()) << "Target is supposed to be 1.";
   EXPECT_EQ(1, full_act_1.targets[0]) << "Target is supposed to be 1.";
   const Model::Parameter & none_act_1 = ModelTranslators::matchContext(circuit_2, "B:1,C:0", 0);
   ASSERT_EQ(1, none_act_1.targets.size()) << "Target is supposed to be 0.";
   EXPECT_EQ(0, none_act_1.targets[0]) << "Target is supposed to be 0.";
   const Model::Parameter & mixed_1 = ModelTranslators::matchContext(circuit_2, "B:1,C:1", 0);
   ASSERT_EQ(2, mixed_1.targets.size()) << "Target is supposed to be 0.";
   const Model::Parameter & mixed_2 = ModelTranslators::matchContext(circuit_2, "B:0,C:0", 0);
   ASSERT_EQ(2, mixed_2.targets.size()) << "Target is supposed to be 0.";

   bool_k_2.restrictions.force_extremes = true;
   ConstructionManager::computeModelProps(bool_k_2);
   const Model::Parameter & fake_full_act = ModelTranslators::matchContext(bool_k_2, "A:0,B:0,C:1", 0);
   EXPECT_EQ(2, fake_full_act.targets.size()) << "Target is supposed to be 1.";
}

/// This test controls functionality of loop bounding constraint.
TEST_F(ModelsTest, ParametrizationLoopBound) {
   Model loop_model;
   loop_model.addSpecie("A", 1, range<ActLevel>(2u));
   loop_model.addSpecie("B", 5, range<ActLevel>(6u));
   loop_model.addRegulation(0, 1, 1, "");
   loop_model.addRegulation(1, 1, 2, "");
   loop_model.addRegulation(1, 1, 4, "");
   loop_model.restrictions.bound_loop = true;
   RegulationHelper::fillActivationLevels(loop_model);
   ParameterHelper::fillParameters(loop_model);

   // Transform the description into semantics.
   ASSERT_NO_THROW(ParameterReader::constrainParameters(loop_model));

   auto params = loop_model.getParameters(1);
   ASSERT_EQ(6, params.size()) << "There should be 6 contexts for B.";
   ASSERT_EQ(3, params[0].targets.size()) << "Targets {0,1,2}";
   EXPECT_EQ(2, params[0].targets[2]) << "Targets {0,1,2}";
   ASSERT_EQ(4, params[3].targets.size()) << "Targets {1,2,3,4}";
   EXPECT_EQ(1, params[3].targets[0]) << "Targets {1,2,3,4}";
}

#endif // MODEL_FUNCTIONS_TEST_HPP
