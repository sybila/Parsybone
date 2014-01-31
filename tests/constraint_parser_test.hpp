/*
* Copyright (C) 2012-2013 - Adam Streck
* This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
* ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
* ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
* For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
*/

#ifndef PARSYBONE_CONSTRAINT_PARSER_TEST_INCLUDED
#define PARSYBONE_CONSTRAINT_PARSER_TEST_INCLUDED

#include "../auxiliary/formulae_resolver.hpp"
#include "../parsing/constraint_parser.hpp"
#include <gtest/gtest.h>

TEST(ConstraintParserTest, TestCopy) {
	ConstraintParser::testCopy();
}

TEST(ConstraintParserTest, ResolveFormulae) {
	// Test true formulae
	std::string true_forms[] = { "tt", "A", "!B", "(ff|A)", "(A|B)", "!(A&B)", "(!(A&A)|!B)", "A|B|A", "((A))" };
	for (auto & formula : true_forms)
		EXPECT_TRUE(ConstraintParser::contains({ "A", "B" }, 1, { 1, 0 }, formula)) << formula;

	// Test false formulae
	std::string false_forms[] = { "ff", "B", "((A|B)&ff)", "(B&!B)", "A&B&A" };
	for (auto & formula : false_forms)
		EXPECT_FALSE(ConstraintParser::contains({ "A", "B" }, 1, { 1, 0 }, formula)) << formula;

	EXPECT_NO_THROW(ConstraintParser::contains({ "a_", "_b9" }, { 1, 1 }, { 1, 1 }, "a_ = _b9"));
}


TEST(ConstraintParserTest, ResolveConstraints) {
	// Test true formulae
	std::string true_forms[] = { "A != B" , "A > B & B <= C & C < 2", "(A=2)&(B=0)&(C=1)" };
	for (auto & formula : true_forms)
		EXPECT_TRUE(ConstraintParser::contains({ "A", "B", "C" }, { 3, 2, 1 }, { 2, 0, 1 }, formula)) << formula;

	// Test false formulae
	std::string false_forms[] = { "A = C | A = B | B = C", "!(A > B)", "B = -1", "A > 2" };
	for (auto & formula : false_forms)
		EXPECT_FALSE(ConstraintParser::contains({ "A", "B", "C" }, { 3, 2, 1 }, { 2, 0, 1 }, formula)) << formula;
}

TEST(ConstraintParserTest, TruncateWhiteSpace) {
	// Test true formulae
	std::string true_forms[] = { " tt ", "  A   ", " ! B ", " ( ff | \n A  ) ", " \r ( A| B) ", "A & B = 0" };
	for (auto & formula : true_forms)
		EXPECT_TRUE(ConstraintParser::contains({ "A", "B" }, 1, { 1, 0 }, formula)) << formula;

	// Test false formulae
	std::string false_forms[] = { " ff ", "  B   ", " \r  ((A |B ) & ff)", "\n\n\n (B&!B)" };
	for (auto & formula : false_forms)
		EXPECT_FALSE(ConstraintParser::contains({ "A", "B" }, 1, { 1, 0 }, formula)) << formula;
}

TEST(ConstraintParserTest, CauseException) {
	EXPECT_THROW(ConstraintParser::contains({ "A", "B" }, 1, { 1, 1 }, "C"), runtime_error); // No C defined
	EXPECT_THROW(ConstraintParser::contains({ "A", "B" }, 1, { 1, 1 }, "A || B"), runtime_error); // Duplicate
	EXPECT_THROW(ConstraintParser::contains({ "A", "B", "C" }, 1, { 1, 1, 1 }, "A | B & C"), runtime_error); // Parenthesis ambiguity
	EXPECT_THROW(ConstraintParser::contains({ "A", "B" }, 1, { 1, 1 }, "(((A | B) & A)"), runtime_error); // Parenthesis mismatch
	EXPECT_THROW(ConstraintParser::contains({ "A" }, 1, { 1 }, ")(A)("), runtime_error); // Parenthesis mismatch
}


#endif // PARSYBONE_CONSTRAINT_PARSER_TEST_INCLUDED
