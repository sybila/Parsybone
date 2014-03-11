/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warranty. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef MODEL_FUNCTIONS_TEST_HPP
#define MODEL_FUNCTIONS_TEST_HPP

#include "model_test_data.hpp"
#include "../model/parameter_reader.hpp"
#include "../model/regulation_helper.hpp"
#include "../model/constraint_reader.hpp"

/// Test various functions the model class posseses.
TEST_F(ModelsTest, ModelFunctions) {
	auto tresholds = ModelTranslators::getThresholds(one_three, 0);
	ASSERT_EQ(2u, tresholds.size());
	ASSERT_EQ(3u, tresholds.find(1)->second[1]);

	ASSERT_EQ(2, ModelTranslators::getRegulatorsIDs(one_three, 0).size()) << "There must be only two IDs for regulations, even though there are three incoming interactions.";
}

/// Controls whether explicit parametrizaitions do replace the original values.
TEST_F(ModelsTest, ParametrizationControl) {

	one_three.species[0].par_kin.push_back(make_pair("A,B:1", "1"));
	one_three.species[0].par_kin.push_back(make_pair("B:3", "0"));
	one_three.species[1].par_kin.push_back(make_pair("A", "3"));

	// Transform the description into semantics.
	ASSERT_NO_THROW(ParameterHelper::fillParameters(one_three));
	ASSERT_NO_THROW(ParameterReader::constrainParameters(one_three));

	auto params = one_three.getParameters(0);
	ASSERT_EQ(6, params.size()) << "There should be 6 contexts for A.";
	for (const auto & param : params) {
		if (param.context.compare("A:1,B:1") == 0) {
			ASSERT_EQ(1, param.targets.size()) << "One possible target val in this context.";
			EXPECT_EQ(1, param.targets[0]) << "Target value in given countext should be one.";
		}
		if (param.context.compare("A:0,B:1") == 0) {
			ASSERT_EQ(2, param.targets.size()) << "Target values should not be constrained.";
		}
	}
}

/// This test controls functionality of loop bounding constraint.
TEST_F(ModelsTest, ParametrizationLoopBound) {
	Model loop_model;
	loop_model.addSpecie("A", 1, { 0u, 1u }, Model::Component);
	loop_model.addSpecie("B", 5, { 0u, 1u, 2u, 3u, 4u, 5u }, Model::Component);
	loop_model.addRegulation(0, 1, 1, "");
	loop_model.addRegulation(1, 1, 2, "");
	loop_model.addRegulation(1, 1, 4, "");
	loop_model.restrictions.bound_loop = true;
	RegulationHelper::fillActivationLevels(loop_model);
	ParameterHelper::fillParameters(loop_model);

	// Transform the description into semantics.
	ASSERT_NO_THROW(ParameterReader::constrainParameters(loop_model));

	auto params = loop_model.getParameters(1);
	ASSERT_EQ(6, params.size()) << "There should be 6 contexts for B.";
	ASSERT_EQ(3, params[0].targets.size()) << "Targets {0,1,2}";
	EXPECT_EQ(2, params[0].targets[2]) << "Targets {0,1,2}";
	ASSERT_EQ(4, params[3].targets.size()) << "Targets {1,2,3,4}";
	EXPECT_EQ(1, params[3].targets[0]) << "Targets {1,2,3,4}";
}

TEST_F(ModelsTest, ReadConstraints) {
	one_three.species[0].par_cons.clear();
	one_three.species[0].par_cons.push_back("A:1,B:3");
	EXPECT_STREQ("(tt & (A:1,B:3))", ConstraintReader::consToFormula(one_three, 0).c_str());

	one_three.species[0].par_cons.clear();
	one_three.species[0].par_cons.push_back("A,B:3");
	EXPECT_STREQ("(tt & (A:1,B:3))", ConstraintReader::consToFormula(one_three, 0).c_str());

	one_three.species[0].par_cons.clear();
	one_three.species[0].par_cons.push_back("B:3");
	EXPECT_STREQ("(tt & (A:0,B:3))", ConstraintReader::consToFormula(one_three, 0).c_str());

	one_three.species[0].par_cons.clear();
	one_three.species[0].par_cons.push_back("B");
	EXPECT_THROW(ConstraintReader::consToFormula(one_three, 0), runtime_error);

	one_three.species[0].par_cons.clear();
	one_three.species[0].par_cons.push_back("ax");
	EXPECT_THROW(ConstraintReader::consToFormula(one_three, 0), runtime_error);

	one_three.species[0].par_cons.clear();
	one_three.species[0].par_cons.push_back("A:2");
	EXPECT_THROW(ConstraintReader::consToFormula(one_three, 0), runtime_error);

	one_three.species[0].par_cons.clear();
	one_three.species[0].par_cons.push_back("A,1");
	EXPECT_THROW(ConstraintReader::consToFormula(one_three, 0), runtime_error);
}

#endif // MODEL_FUNCTIONS_TEST_HPP
