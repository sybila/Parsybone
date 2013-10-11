/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSING_TEST_HPP
#define PARSING_TEST_HPP

#include "parsing_test_data.hpp"
#include "../parsing/parsing_manager.hpp"

TEST_F(ParsingTest, ParseArguments) {
   UserOptions user_options;
   EXPECT_EQ(1, user_options.processes_count);
   EXPECT_EQ(1, user_options.process_number);
   EXPECT_EQ(INF, user_options.bound_size);
   EXPECT_FALSE(user_options.minimalize_cost);
   EXPECT_TRUE(user_options.filter_databases.empty());
   string model = (source_path + example_model + MODEL_SUFFIX);
   string property = (source_path + example_automaton + PROPERTY_SUFFIX);
   const char * argv [] = {"program_name",
                     model.c_str(),
                     property.c_str(),
                     "-W",
                     "-v"};
   EXPECT_NO_THROW(user_options = ParsingManager::parseOptions(sizeof(argv)/sizeof(char*), argv));
   EXPECT_TRUE(user_options.compute_wintess);
   EXPECT_FALSE(user_options.compute_robustness);
   EXPECT_FALSE(user_options.output_console);
   EXPECT_STREQ(example_model.c_str(), user_options.model_name.c_str());
   EXPECT_STREQ(source_path.c_str(), user_options.property_path.c_str());
}

TEST_F(ParsingTest, ParseExamples) {
   Model example_m;
   EXPECT_NO_THROW(example_m = ParsingManager::parseModel(source_path, example_model));
   PropertyAutomaton example_a;
   EXPECT_NO_THROW(example_a = ParsingManager::parseProperty(source_path, example_automaton));
   PropertyAutomaton example_s;
   EXPECT_NO_THROW(example_s = ParsingManager::parseProperty(source_path,example_series));
}


#endif // PARSING_TEST_HPP
