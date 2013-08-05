#ifndef MODEL_TEST_DATA_HPP
#define MODEL_TEST_DATA_HPP

#include <gtest/gtest.h>

#include "../model/parameter_reader.hpp"
#include "../model/regulation_helper.hpp"
#include "../construction/construction_manager.hpp"

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
    PropertyAutomaton series_aut;
    PropertyAutomaton cyclic_aut;

    void SetUp() override {
        basic_model.addSpecie("cA", 1, {0,1});
        basic_model.addSpecie("cB", 3, {0,1,2,3});
        basic_model.addRegulation(0, 1, 1, "+");
        basic_model.addRegulation(0, 0, 1, "-");
        basic_model.addRegulation(1, 0, 1, "-");
        basic_model.addRegulation(1, 0, 3, "(-|+)");
        ConstructionManager::computeModelProps(basic_model);

        series_aut.addState("ser0", false);
        series_aut.addState("ser1", false);
        series_aut.addState("ser2", true);
        series_aut.addConditions(0,0,"tt");
        series_aut.addConditions(0,1,"(cA=0|cB=0)");
        series_aut.addConditions(1,1,"tt");
        series_aut.addConditions(1,2,"(cA=1|cB=3)");
        series_aut.addConditions(2,2,"tt");

        cyclic_aut.addState("cyc0", false);
        cyclic_aut.addState("ser1", true);
        cyclic_aut.addState("cyc2", false);
        cyclic_aut.addConditions(0,0,"cA=0");
        cyclic_aut.addConditions(0,1,"cA=1");
        cyclic_aut.addConditions(1,0,"cA=0");
        cyclic_aut.addConditions(1,2,"cA=1");
        cyclic_aut.addConditions(2,1,"cA=0");
        cyclic_aut.addConditions(2,2,"cA=1");
    }
};

#endif // MODEL_TEST_DATA_HPP
