#ifndef TESTING_MODELS_SOURCE_HPP
#define TESTING_MODELS_SOURCE_HPP

#include <gtest/gtest.h>

#include "PunyHeaders/common_functions.hpp"

#include "../auxiliary/data_types.hpp"
#include "../parsing/reading_helper.hpp"
#include "../parsing/model.hpp"
#include "../parsing/parameter_parser.hpp"

using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Class holds an artificial model of the form:
///
///
///Basic model:
/// max(cA)=1, max(cB)=3
/// cA <- -cA:1, -cB:1, +|-cB:3
/// cB <- +cA:1
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class ModelsTest : public ::testing::Test {
protected:
    Model basic_model;

    void SetUp() override {
        basic_model.addSpecie("cA", 1, {0,1});
        basic_model.addSpecie("cB", 3, {0,1,2,3});
        basic_model.addRegulation(0, 1, 1, "+");
        basic_model.addRegulation(0, 0, 1, "-");
        basic_model.addRegulation(1, 0, 1, "-");
        basic_model.addRegulation(1, 0, 3, "- | +");
        ReadingHelper::fillActivationLevels(basic_model);
        ReadingHelper::fillParameters(basic_model);
    }
};

#endif // TESTING_MODELS_SOURCE_HPP
