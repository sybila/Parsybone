/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warranty. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#pragma once

#include "model_test_data.hpp"
#include "../model/model_helper.hpp"
#include "../kinetics/constraint_reader.hpp"

/// Test various functions the model class posseses.
TEST_F(ModelsTest, ModelFunctions) {
	auto tresholds = ModelTranslators::getThresholds(mod_mul, 0);
	ASSERT_EQ(2u, tresholds.size());
	ASSERT_EQ(3u, tresholds.find(1)->second[1]);

	ASSERT_EQ(2, ModelTranslators::getRegulatorsIDs(mod_mul, 0).size()) << "There must be only two IDs for regulations, even though there are three incoming interactions.";
}


TEST_F(ModelsTest, ReadConstraints) {
	mod_mul.species[0].par_cons.clear();
	mod_mul.species[0].par_cons.push_back("A:1,B:3");
	EXPECT_STREQ("(tt & (A:1,B:3))", ConstraintReader::consToFormula(mod_mul, 0).c_str());

	mod_mul.species[0].par_cons.clear();
	mod_mul.species[0].par_cons.push_back("A,B:3");
	EXPECT_STREQ("(tt & (A:1,B:3))", ConstraintReader::consToFormula(mod_mul, 0).c_str());

	mod_mul.species[0].par_cons.clear();
	mod_mul.species[0].par_cons.push_back("B:3");
	EXPECT_STREQ("(tt & (A:0,B:3))", ConstraintReader::consToFormula(mod_mul, 0).c_str());

	mod_mul.species[0].par_cons.clear();
	mod_mul.species[0].par_cons.push_back("B");
	EXPECT_THROW(ConstraintReader::consToFormula(mod_mul, 0), runtime_error);

	mod_mul.species[0].par_cons.clear();
	mod_mul.species[0].par_cons.push_back("ax");
	EXPECT_THROW(ConstraintReader::consToFormula(mod_mul, 0), runtime_error);

	mod_mul.species[0].par_cons.clear();
	mod_mul.species[0].par_cons.push_back("A:2");
	EXPECT_THROW(ConstraintReader::consToFormula(mod_mul, 0), runtime_error);

	mod_mul.species[0].par_cons.clear();
	mod_mul.species[0].par_cons.push_back("A,1");
	EXPECT_THROW(ConstraintReader::consToFormula(mod_mul, 0), runtime_error);
}
