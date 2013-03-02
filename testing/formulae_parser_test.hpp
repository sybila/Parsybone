#ifndef PARSYBONE_FORMULAE_PARSER_TEST_INCLUDED
#define PARSYBONE_FORMULAE_PARSER_TEST_INCLUDED

#include "../parsing/formulae_parser.hpp"
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
        EXPECT_TRUE(FormulaeParser::resolve(vars, formula));

    // Test false formulae
    std::string false_forms [] = {"ff", "B", "((A|B)&ff)", "(B&!B)", "!quite_a_long_true_value_1"};
    for (auto & formula:false_forms)
        EXPECT_FALSE(FormulaeParser::resolve(vars, formula));
}

TEST_F(FormulaValuation, TruncateWhiteSpace) {
    // Test true formulae
    std::string true_forms [] = {" tt ", "  A   ", " ! B ", " ( ff | \n A  ) ", " \r ( A| B) ", "    quite_a_long_true_value_1 "};
    for (auto & formula:true_forms)
        EXPECT_TRUE(FormulaeParser::resolve(vars, formula));

    // Test false formulae
    std::string false_forms [] = {" ff ", "  B   ", " \r  ((A |B ) & ff)", "\n\n\n (B&!B)"};
    for (auto & formula:false_forms)
        EXPECT_FALSE(FormulaeParser::resolve(vars, formula));
}

TEST_F(FormulaValuation, CauseException) {
    EXPECT_NO_THROW(FormulaeParser::resolve(vars, "A")); // No exception should be raised
    EXPECT_THROW(FormulaeParser::resolve(vars, "C"), runtime_error); // No C defined
    EXPECT_THROW(FormulaeParser::resolve(vars, "A|B"), runtime_error); // No parenthesis
    EXPECT_THROW(FormulaeParser::resolve(vars, "(A&&B)"), runtime_error); // Duplicate symbol
}

#endif // PARSYBONE_FORMULAE_PARSER_TEST_INCLUDED
