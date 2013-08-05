#ifndef CONSTRUCTION_TEST_H
#define CONSTRUCTION_TEST_H

#include "construction_test_data.hpp"

TEST_F(StructureTest, TestCorrectConstruction) {
   ASSERT_EQ(1, o_t_series.getAutomaton().getInitialStates().size());
   ASSERT_EQ(1, o_t_cyclic.getAutomaton().getInitialStates().size());
}

#endif // CONSTRUCTION_TEST_H
