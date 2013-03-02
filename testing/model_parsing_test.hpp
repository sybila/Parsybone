#ifndef MODEL_PARSING_TEST_HPP
#define MODEL_PARSING_TEST_HPP

#include "testing_models_source.hpp"

TEST_F(ModelsTest, ModelParsing) {
    NetworkParser parser(basic_model);

    parser.createParameters(0, "");
    parser.createParameters(1, "");
    parser.fillActivationLevels();

    EXPECT_EQ(2u, basic_model.getRegulations(0)[1].activity.size()) << "Wrong number of activity levels.";
    EXPECT_EQ(1u, basic_model.getRegulations(0)[0].activity[0]) << "Wrong activity levels values in singlelevel.";
    EXPECT_EQ(2u, basic_model.getRegulations(0)[1].activity[1]) << "Wrong activity levels values in multilevel.";
}

#endif // MODEL_PARSING_TEST_HPP
