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
/// Basic model:
/// max(cA)=1, max(cB)=3
/// cA <- -cA:1, -cB:1, +|-cB:3
/// cB <- +cA:1
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ModelsTest : public ::testing::Test {
protected:
    Model trivial_model;
    Model one_three;
    Model circuit_2;
    Model bool_k_2;
    PropertyAutomaton trivial_prop;
    PropertyAutomaton o_t_series_prop;
    PropertyAutomaton cA_cyclic_prop;
    PropertyAutomaton set_two_ones_prop;

    void setUpModels() {
        trivial_model.addSpecie("cA", 1, {0,1});
        trivial_model.addRegulation(0,0,1,"Free");

        one_three.addSpecie("cA", 1, {0,1});
        one_three.addSpecie("cB", 3, {0,1,2,3});
        one_three.addRegulation(0, 1, 1, "+");
        one_three.addRegulation(0, 0, 1, "-");
        one_three.addRegulation(1, 0, 1, "-");
        one_three.addRegulation(1, 0, 3, "(-|+)");

        circuit_2.addSpecie("cA", 1, {0,1});
        circuit_2.addSpecie("cB", 1, {0,1});
        circuit_2.addRegulation(0, 1, 1, "+");
        circuit_2.addRegulation(1, 0, 1, "-");

        bool_k_2.addSpecie("cA", 1, {0,1});
        bool_k_2.addSpecie("cB", 1, {0,1});
        bool_k_2.addRegulation(0, 0, 1, "Free");
        bool_k_2.addRegulation(0, 1, 1, "Free");
        bool_k_2.addRegulation(1, 0, 1, "Free");
        bool_k_2.addRegulation(1, 1, 1, "Free");
    }

    void setUpAutomata() {
        trivial_prop.addState("triv0", true);
        trivial_prop.addEdge(0,0,"tt");

        o_t_series_prop.addState("ser0", false);
        o_t_series_prop.addState("ser1", false);
        o_t_series_prop.addState("ser2", true);
        o_t_series_prop.addEdge(0,0,"tt");
        o_t_series_prop.addEdge(0,1,"(cA=0|cB=0)");
        o_t_series_prop.addEdge(1,1,"tt");
        o_t_series_prop.addEdge(1,2,"(cA=1|cB=3)");
        o_t_series_prop.addEdge(2,2,"tt");

        cA_cyclic_prop.addState("cyc0", false);
        cA_cyclic_prop.addState("cyc1", true);
        cA_cyclic_prop.addState("cyc2", false);
        cA_cyclic_prop.addEdge(0,0,"cA=0");
        cA_cyclic_prop.addEdge(0,1,"cA=1");
        cA_cyclic_prop.addEdge(1,0,"cA=0");
        cA_cyclic_prop.addEdge(1,2,"cA=1");
        cA_cyclic_prop.addEdge(2,1,"cA=0");
        cA_cyclic_prop.addEdge(2,2,"cA=1");

        set_two_ones_prop.addState("ser0", false);
        set_two_ones_prop.addState("ser1", false);
        set_two_ones_prop.addState("ser2", true);
        set_two_ones_prop.addEdge(0,0,"tt");
        set_two_ones_prop.addEdge(0,1,"(cA=0&cB=1)");
        set_two_ones_prop.addEdge(1,1,"tt");
        set_two_ones_prop.addEdge(1,2,"(cA=1&cB=1)");
        set_two_ones_prop.addEdge(2,2,"tt");
    }

    void SetUp() override {
        setUpModels();
        setUpAutomata();
    }
};

#endif // MODEL_TEST_DATA_HPP
