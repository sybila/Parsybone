/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_FORMULAE_PARSER_TEST_INCLUDED
#define PARSYBONE_FORMULAE_PARSER_TEST_INCLUDED

#include "../auxiliary/formulae_resolver.hpp"
#include "../parsing/constraint_parser.hpp"
#include <gtest/gtest.h>

class FormulaValuation : public ::testing::Test {
protected:
    static map<string, bool> vars;

    void static SetUpTestCase() {
        vars.insert(make_pair("A",true));
        vars.insert(make_pair("B",false));
        vars.insert(make_pair("quite_a_long_true_value_1", true));
    }
};
map<string, bool> FormulaValuation::vars;

TEST_F(FormulaValuation, ValuateFormulas) {
    // Test true formulae
    std::string true_forms [] = {"tt", "A", "!B", "(ff|A)", "(A|B)", "!(A&B)", "(!(A&A)|!B)", "quite_a_long_true_value_1"};
    for (auto & formula:true_forms)
        EXPECT_TRUE(FormulaeResolver::resolve(vars, formula));

    // Test false formulae
    std::string false_forms [] = {"ff", "B", "((A|B)&ff)", "(B&!B)", "!quite_a_long_true_value_1"};
    for (auto & formula:false_forms)
        EXPECT_FALSE(FormulaeResolver::resolve(vars, formula));
}

TEST_F(FormulaValuation, TruncateWhiteSpace) {
    // Test true formulae
    std::string true_forms [] = {" tt ", "  A   ", " ! B ", " ( ff | \n A  ) ", " \r ( A| B) ", "    quite_a_long_true_value_1 "};
    for (auto & formula:true_forms)
        EXPECT_TRUE(FormulaeResolver::resolve(vars, formula));

    // Test false formulae
    std::string false_forms [] = {" ff ", "  B   ", " \r  ((A |B ) & ff)", "\n\n\n (B&!B)"};
    for (auto & formula:false_forms)
        EXPECT_FALSE(FormulaeResolver::resolve(vars, formula));
}

TEST_F(FormulaValuation, CauseException) {
    EXPECT_NO_THROW(FormulaeResolver::resolve(vars, "A")); // No exception should be raised
    EXPECT_THROW(FormulaeResolver::resolve(vars, "C"), runtime_error); // No C defined
    EXPECT_THROW(FormulaeResolver::resolve(vars, "A|B"), runtime_error); // No parenthesis
    EXPECT_THROW(FormulaeResolver::resolve(vars, "(A&&B)"), runtime_error); // Duplicate symbol
}

TEST(FormulaParsing, Basic) {
	ConstraintParser *constraint_parser = new ConstraintParser({ "a", "b" }, { 2, 1 });
	constraint_parser->applyFormula("a > b & b = 1");
	DFS<ConstraintParser> search(constraint_parser);
	delete constraint_parser;
	while (ConstraintParser *space = search.next()) {
		space->print();
		delete space;
	}
}

#endif // PARSYBONE_FORMULAE_PARSER_TEST_INCLUDED
