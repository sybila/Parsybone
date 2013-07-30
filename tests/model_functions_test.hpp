#ifndef MODEL_FUNCTIONS_TEST_HPP
#define MODEL_FUNCTIONS_TEST_HPP

#include "testing_models_source.hpp"
#include "../parsing/parameter_reader.hpp"

/// Test various functions the model class posseses.
TEST_F(ModelsTest, ModelFunctions) {
   auto tresholds = basic_model.getThresholds(0);
   ASSERT_EQ(2u, tresholds.size());
   ASSERT_EQ(3u, tresholds.find(1)->second[1]);

   ASSERT_EQ(2, basic_model.getRegulatorsIDs(0).size()) << "There must be only two IDs for regulations, even though there are three incoming interactions.";
}

/// Controls whether explicit parametrizaitions do replace the original values.
TEST_F(ModelsTest, ParametrizationControl) {
   // Create the parameter specification
   ParameterParser::ParameterSpecifications specs;
   specs.param_specs.resize(2);

   specs.param_specs[0].k_pars.push_back(make_pair("cA,cB:1","1"));
   specs.param_specs[0].k_pars.push_back(make_pair("cB:3","0"));
   specs.param_specs[1].k_pars.push_back(make_pair("cA","3"));

   // Transform the description into semantics.
   ASSERT_NO_THROW(ParameterReader::computeParams(specs, basic_model));

   auto params = basic_model.getParameters(0);
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
   ParameterHelper::fillActivationLevels(extreme_model);
   ParameterHelper::fillParameters(extreme_model);
   ParameterParser::ParameterSpecifications specs;
   specs.param_specs.resize(1);

   // Transform the description into semantics.
   ParameterReader::computeParams(specs, extreme_model);

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
   ParameterHelper::fillActivationLevels(loop_model);
   ParameterHelper::fillParameters(loop_model);
   ParameterParser::ParameterSpecifications specs;
   specs.param_specs.resize(2);


   // Transform the description into semantics.
   ASSERT_NO_THROW(ParameterReader::computeParams(specs, loop_model));

   auto params = loop_model.getParameters(1);
   ASSERT_EQ(6, params.size()) << "There should be 6 contexts for cB.";
   ASSERT_EQ(3, params[0].targets.size()) << "Targets {0,1,2}";
   EXPECT_EQ(2, params[0].targets[2]) << "Targets {0,1,2}";
   ASSERT_EQ(4, params[3].targets.size()) << "Targets {1,2,3,4}";
   EXPECT_EQ(1, params[3].targets[0]) << "Targets {1,2,3,4}";
}

#endif // MODEL_FUNCTIONS_TEST_HPP
