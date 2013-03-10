#ifndef TESTING_MODELS_SOURCE_HPP
#define TESTING_MODELS_SOURCE_HPP

#include <gtest/gtest.h>

#include "../auxiliary/common_functions.hpp"
#include "../auxiliary/data_types.hpp"
#include "../parsing/model_parser.hpp"
#include "../parsing/model.hpp"

using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Class holds an artificial model of the form:
///
///      A,1  B,3
/// A,1  1    1
/// B,3  1,3
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class ModelsTest : public ::testing::Test {
protected:
    Model basic_model;

    void SetUp() override {
        basic_model.addSpecie("A", 1, {0,1});
        basic_model.addSpecie("B", 3, {0,1});
        basic_model.addRegulation(0, 1, 1, "+");
        basic_model.addRegulation(0, 0, 1, "-");
        basic_model.addRegulation(1, 0, 1, "-");
        basic_model.addRegulation(1, 0, 3, "- | +");
    }
};

#endif // TESTING_MODELS_SOURCE_HPP
