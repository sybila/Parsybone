/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef OUTPUT_TEST_HPP
#define OUTPUT_TEST_HPP

#include "model_test_data.hpp"
#include "../synthesis/output_manager.hpp"

TEST_F(StructureTest, DatabaseTest) {
   UserOptions user_options;
   user_options.use_database = true;
   user_options.database_file = "trivial.sqlite";
   OutputManager output(user_options, trivial_prop, trivial_model);

   EXPECT_NO_THROW(output.outputForm());
   EXPECT_NO_THROW(output.outputRound(0, 3, 0.5, "{(0>0)}"));
   output.outputRound(1, 5, 0.0, "{(1>3)}");
 }

#endif // OUTPUT_TEST_HPP
