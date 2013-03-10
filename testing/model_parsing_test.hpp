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

TEST_F(ModelsTest, CanonicTranslator) {
   NetworkParser parser(basic_model);

   EXPECT_STREQ("A:0", parser.getCanonic("", 1).c_str());
   EXPECT_STREQ("A:1", parser.getCanonic("A", 1).c_str());
   // EXPECT_THROW(parser.getCanonic("C:", 1), runtime_error);
   EXPECT_THROW(parser.getCanonic("A:", 1), runtime_error);
   // EXPECT_THROW(parser.getCanonic("A:2", 1), runtime_error);
   EXPECT_THROW(parser.getCanonic("A:-1", 1), runtime_error);
   EXPECT_THROW(parser.getCanonic("A:a", 1), runtime_error);

   EXPECT_STREQ("A:1,B:0", parser.getCanonic("A:1", 0).c_str());
   EXPECT_STREQ("A:1,B:1", parser.getCanonic("A,B:1", 0).c_str());
   EXPECT_STREQ("A:0,B:3", parser.getCanonic("B:3", 0).c_str());

}

#endif // MODEL_PARSING_TEST_HPP
