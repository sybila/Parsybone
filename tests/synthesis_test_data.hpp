/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef SYNTHESIS_TEST_DATA_HPP
#define SYNTHESIS_TEST_DATA_HPP

#include "../synthesis/synthesis_manager.hpp"
#include "construction_test_data.hpp"

class SynthesisTest : public StructureTest {
protected:

   void SetUp() override {
      StructureTest::SetUp();

   }
};

#endif // SYNTHESIS_TEST_DATA_HPP
