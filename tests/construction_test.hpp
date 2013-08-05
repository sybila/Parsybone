#ifndef CONSTRUCTION_TEST_H
#define CONSTRUCTION_TEST_H

#include "construction_test_data.hpp"

TEST_F(StructureTest, TestCorrectConstruction) {
   ASSERT_EQ(1, series.getAutomaton().getInitialStates().size());
   ASSERT_EQ(1, cyclic.getAutomaton().getInitialStates().size());
}

#endif // CONSTRUCTION_TEST_H
