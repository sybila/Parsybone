#ifndef MODEL_FUNCTIONS_TEST_HPP
#define MODEL_FUNCTIONS_TEST_HPP

#include "testing_models_source.hpp"

TEST_F(ModelsTest, ModelFunctions) {
    auto tresholds = basic_model.getThresholds(0);
    ASSERT_EQ(2u, tresholds.size());
    ASSERT_EQ(3u, tresholds.find(1)->second[1]);
}

#endif // MODEL_FUNCTIONS_TEST_HPP
