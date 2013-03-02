#ifndef PARSYBONE_TESTER_INCLUDED
#define PARSYBONE_TESTER_INCLUDED

#include "../auxiliary/common_functions.hpp"
#include "../auxiliary/data_types.hpp"
#include "../parsing/formulae_parser.hpp"
#include "../parsing/model_parser.hpp"
#include "../parsing/model.hpp"
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
    ASSERT_NO_THROW(FormulaeParser::resolve(vars, "A")); // No exception should be raised
    ASSERT_THROW(FormulaeParser::resolve(vars, "C"), runtime_error); // No C defined
    ASSERT_THROW(FormulaeParser::resolve(vars, "A|B"), runtime_error); // No parenthesis
    ASSERT_THROW(FormulaeParser::resolve(vars, "(A&&B)"), runtime_error); // Duplicate symbol
}

enum Tests {
    begin_test, model_func, network_parser, end_test
};

class Tester {
    void fillModel(Model & model) {
        model.addSpecie("A", 1, {0,1});
        model.addSpecie("B", 3, {0,1});
        model.addRegulation(0, 1, 1, "+");
        model.addRegulation(0, 0, 1, "-");
        model.addRegulation(1, 0, 1, "-");
        model.addRegulation(1, 0, 3, "- | +");
    }

    bool testModelFunctions() {
        Model model;
        fillModel(model);
        auto tresholds = model.getThresholds(0);
        if (tresholds.size() != 2) {
            cout << "Wrong number of regulators.";
            return false;
        } else if (tresholds.find(1)->second[1] != 3) {
            cout << "Wrong threshold values.";
            return false;
        }
        return true;
    }

    bool testNetworkParser() {
        Model model;
        fillModel(model);
        NetworkParser parser(model);

        parser.createParameters(0, "");
        parser.createParameters(1, "");

        string reform = parser.getCanonic("B", 0);
        cout << "Reform " << reform << endl;
        reform = parser.getCanonic("", 1);
        cout << "Reform " << reform << endl;
        reform = parser.getCanonic("A", 1);
        cout << "Reform " << reform << endl;

        parser.fillActivationLevels();
        if (model.getRegulations(0)[1].activity.size() != 2) {
            cout << "Wrong number of activity levels.";
            return false;
        } else if (model.getRegulations(0)[0].activity[0] != 1) {
            cout << "Wrong activity levels values in singlelevel.";
            return false;
        } else if (model.getRegulations(0)[1].activity[1] != 2) {
            cout << "Wrong activity levels values in multilevel.";
            return false;
        }
        return true;
    }

public:
    bool test(const int test_type) {
        bool result = true;
        cout << "# Executing test: ";
        switch (test_type) {
        case model_func:
            cout << "Model functions." << endl;
            result = testModelFunctions();
            break;
        case network_parser:
            cout << "NetworkParser." << endl;
            result = testNetworkParser();
            break;
        }

        if (!result)
            cout << endl << "Test failed." << endl;

        return result;
    }

    bool testAll() {
        for (int test_name = begin_test + 1; test_name != end_test; test_name++) {
            test(test_name);
        }
    }
};

#endif // PARSYBONE_TESTER_INCLUDED
