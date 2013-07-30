#ifndef MODEL_FUNCTIONS_TEST_HPP
#define MODEL_FUNCTIONS_TEST_HPP

#include "testing_models_source.hpp"
#include "../parsing/parameter_reader.hpp"

TEST_F(ModelsTest, ModelFunctions) {
   auto tresholds = basic_model.getThresholds(0);
   ASSERT_EQ(2u, tresholds.size());
   ASSERT_EQ(3u, tresholds.find(1)->second[1]);

   ASSERT_EQ(2, basic_model.getRegulatorsIDs(0).size()) << "There must be only two IDs for regulations, even though there are three incoming interactions.";
}

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
   ASSERT_EQ(6, params.size()) << "There should be 6 contexts for A.";
   for (const auto & param:params) {
      if (param.context.compare("cA:1,cB:1") == 0) {
         ASSERT_EQ(1, param.targets.size()) << "One possible target val in this context.";
         EXPECT_EQ(1, param.targets[0]) << "This context should be one.";
      }
   }
}

/// This test controls functionality of loop bounding constraint.
TEST_F(ModelsTest, ParametrizationBounding) {
   Model loop_model;
   loop_model.addSpecie("cA", 1, range(2u));
   loop_model.addSpecie("cB", 5, range(6u));
   loop_model.addRegulation(0, 1, 1, "");
   loop_model.addRegulation(1, 1, 2, "");
   loop_model.addRegulation(1, 1, 4, "");
   loop_model.restrictions.bounded_loops = true;
   ReadingHelper::fillActivationLevels(loop_model);
   ReadingHelper::fillParameters(loop_model);
   ParameterParser::ParameterSpecifications specs;
   specs.param_specs.resize(2);


   // Transform the description into semantics.
   ASSERT_NO_THROW(ParameterReader::computeParams(specs, loop_model));

   auto params = loop_model.getParameters(1);
   ASSERT_EQ(6, params.size()) << "There should be 6 contexts for B.";
}

#endif // MODEL_FUNCTIONS_TEST_HPP
