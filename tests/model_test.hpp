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

   one_three.species[0].params_specs.k_pars.push_back(make_pair("cA,cB:1","1"));
   one_three.species[0].params_specs.k_pars.push_back(make_pair("cB:3","0"));
   one_three.species[1].params_specs.k_pars.push_back(make_pair("cA","3"));

   // Transform the description into semantics.
   ASSERT_NO_THROW(ParameterHelper::fillParameters(one_three));
   ASSERT_NO_THROW(ParameterReader::constrainParameters(one_three));

   auto params = one_three.getParameters(0);
   ASSERT_EQ(6, params.size()) << "There should be 6 contexts for cA.";
   for (const auto & param:params) {
      if (param.context.compare("cA:1,cB:1") == 0) {
         ASSERT_EQ(1, param.targets.size()) << "One possible target val in this context.";
         EXPECT_EQ(1, param.targets[0]) << "Target value in given countext should be one.";
      }
      if (param.context.compare("cA:0,cB:1") == 0) {
         ASSERT_EQ(2, param.targets.size()) << "Target values should not be constrained.";
      }
   }
}

/// This test controls functionality of loop bounding constraint.
TEST_F(ModelsTest, ParametrizationExtremal) {
   Model extreme_model;
   extreme_model.addSpecie("cA", 1, range(2u));
   extreme_model.addRegulation(0, 0, 1, "");
   extreme_model.restrictions.force_extremes = true;
   RegulationHelper::fillActivationLevels(extreme_model);
   ParameterHelper::fillParameters(extreme_model);

   // Transform the description into semantics.
   ParameterReader::constrainParameters(extreme_model);

   auto params = extreme_model.getParameters(0);
   ASSERT_EQ(2, params.size()) << "There should be 2 contexts for cB.";
   ASSERT_EQ(1, params[0].targets.size()) << "Target is supposed to be 0.";
   EXPECT_EQ(0, params[0].targets[0]) << "Targets {0,1,2}";
   ASSERT_EQ(1, params[1].targets.size()) << "Target is supposed to be 0.";
   EXPECT_EQ(1, params[1].targets[0]) << "Targets {0,1,2}";
}

/// This test controls functionality of loop bounding constraint.
TEST_F(ModelsTest, ParametrizationLoopBound) {
   Model loop_model;
   loop_model.addSpecie("cA", 1, range(2u));
   loop_model.addSpecie("cB", 5, range(6u));
   loop_model.addRegulation(0, 1, 1, "");
   loop_model.addRegulation(1, 1, 2, "");
   loop_model.addRegulation(1, 1, 4, "");
   loop_model.restrictions.bounded_loops = true;
   RegulationHelper::fillActivationLevels(loop_model);
   ParameterHelper::fillParameters(loop_model);

   // Transform the description into semantics.
   ASSERT_NO_THROW(ParameterReader::constrainParameters(loop_model));

   auto params = loop_model.getParameters(1);
   ASSERT_EQ(6, params.size()) << "There should be 6 contexts for cB.";
   ASSERT_EQ(3, params[0].targets.size()) << "Targets {0,1,2}";
   EXPECT_EQ(2, params[0].targets[2]) << "Targets {0,1,2}";
   ASSERT_EQ(4, params[3].targets.size()) << "Targets {1,2,3,4}";
   EXPECT_EQ(1, params[3].targets[0]) << "Targets {1,2,3,4}";
}

#endif // MODEL_FUNCTIONS_TEST_HPP
