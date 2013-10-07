/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef CORE_LEVEL_TESTS_HPP
#define CORE_LEVEL_TESTS_HPP

#include <gtest/gtest.h>
#include "../synthesis/split_manager.hpp"

TEST(CoreLevelTest, SplitTest) {
   SplitManager manager(3,1,10);
   manager.computeSubspace();
   size_t param_no = 0;
   do {
      EXPECT_EQ(param_no, manager.getParamNo());
      ASSERT_GT(10u, param_no);
      param_no += 3;
   } while(manager.increaseRound());
}

#endif // CORE_LEVEL_TESTS_HPP
