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

class ConstraintParserTest : public ::testing::Test {
protected:
	// True if the two vectors are equal
	bool isEqual(const Levels & sat, const Levels & req) {
		if (sat.size() != req.size())
			return false;
		return equal(sat.begin(), sat.end(), req.begin());
	}

	// True if the required result is in between solution of the formula
	bool contains(const vector<string> names, const Levels maxes, const Levels required, const string formula) {
		ConstraintParser *constraint_parser = new ConstraintParser(names, maxes);
		constraint_parser->applyFormula(formula);
		DFS<ConstraintParser> search(constraint_parser);
		delete constraint_parser;
		while (ConstraintParser *space = search.next()) {
			if (isEqual(space->getSolution(), required))
				return true;
			delete space;
		}
		return false;
	}
};

TEST_F(ConstraintParserTest, ResolveFormulae) {
	// Test true formulae
	std::string true_forms[] = { "tt", "A", "!B", "(ff|A)", "(A|B)", "!(A&B)", "(!(A&A)|!B)", "A | B | A" };
	for (auto & formula : true_forms)
		EXPECT_TRUE(contains({ "A", "B" }, { 1, 1 }, { 1, 0 }, formula)) << formula;

	// Test false formulae
	std::string false_forms[] = { "ff", "B", "((A|B)&ff)", "(B&!B)", "A &B&A" };
	for (auto & formula : false_forms)
		EXPECT_FALSE(contains({ "A", "B" }, { 1, 1 }, { 1, 0 }, formula)) << formula;
}


TEST_F(ConstraintParserTest, ResolveConstraints) {
	// Test true formulae
	std::string true_forms[] = { "A != B" , "A > B & B <= C & C < 2", "(A=2)&(B=0)&(C=1)" };
	for (auto & formula : true_forms)
		EXPECT_TRUE(contains({ "A", "B", "C" }, { 3, 2, 1 }, { 2, 0, 1 }, formula)) << formula;

	// Test false formulae
	std::string false_forms[] = { "A = C | A = B | B = C", "!(A > B)" };
	for (auto & formula : false_forms)
		EXPECT_FALSE(contains({ "A", "B", "C" }, { 3, 2, 1 }, { 2, 0, 1 }, formula)) << formula;
}

TEST_F(ConstraintParserTest, TruncateWhiteSpace) {
	// Test true formulae
	std::string true_forms[] = { " tt ", "  A   ", " ! B ", " ( ff | \n A  ) ", " \r ( A| B) ", "A & B = 0" };
	for (auto & formula : true_forms)
		EXPECT_TRUE(contains({ "A", "B" }, { 1, 1 }, { 1, 0 }, formula)) << formula;

	// Test false formulae
	std::string false_forms[] = { " ff ", "  B   ", " \r  ((A |B ) & ff)", "\n\n\n (B&!B)" };
	for (auto & formula : false_forms)
		EXPECT_FALSE(contains({ "A", "B" }, { 1, 1 }, { 1, 0 }, formula)) << formula;
}

TEST_F(ConstraintParserTest, CauseException) {
	EXPECT_THROW(contains({ "A", "B" }, { 1, 1 }, { 1, 1 }, "C"), runtime_error); // No C defined
	EXPECT_THROW(contains({ "A", "B" }, { 1, 1 }, { 1, 1 }, "A || B"), runtime_error); // Duplicate
	EXPECT_THROW(contains({ "A", "B", "C" }, { 1, 1, 1 }, { 1, 1, 1 }, "A | B & C"), runtime_error); // Parenthesis ambiguity
}


#endif // PARSYBONE_CONSTRAINT_PARSER_TEST_INCLUDED
