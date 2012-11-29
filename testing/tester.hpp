#ifndef PARSYBONE_TESTER_INCLUDED
#define PARSYBONE_TESTER_INCLUDED

#include "includes.hpp"

enum Tests {
    begin_test, formulas, model_func, end_test
};

class Tester {
    bool testModelFunctions() {
        Model model;
        model.addSpecie("A", 1, 1);
        model.addSpecie("B", 3, 1);
        model.addRegulation(0, 1, 1, "+");
        model.addRegulation(0, 0, 1, "-");
        model.addRegulation(1, 0, 1, "-");
        model.addRegulation(1, 0, 3, "- | +");
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

    bool testFormulaeParser() {
        bool correct = true;
        map<string, bool> vars;
        vars.insert(make_pair("A",true));
        vars.insert(make_pair("B",false));

        vector<string> formulas;
        formulas.push_back("tt");
        formulas.push_back("A");
        formulas.push_back("!B");
        formulas.push_back("(ff|A)");
        formulas.push_back("(A|B)");
        formulas.push_back("!(A&B)");
        formulas.push_back("(!(A&A)|!B)");

        for (auto formula_it = formulas.begin(); formula_it != formulas.end(); formula_it++) {
            if (!FormulaeParser::resolve(vars, *formula_it)) {
                cout << "formula " << *formula_it << " is false" << endl;
                correct = false;
            }
        }

        if (FormulaeParser::resolve(map<string, bool>(), "ff")) {
            cout << "formula ff is true" << endl;
            correct = false;
        }

        return correct;
    }

public:
    bool test(const int test_type) {
        bool result = true;
        cout << "# Executing test: ";
        switch (test_type) {
        case begin_test:
            cout << "Initiation of full test." << endl;
            break;
        case formulas:
            cout << "Propositional formula resolving." << endl;
            result = testFormulaeParser();
            break;
        case model_func:
            cout << "Model functions." << endl;
            result = testModelFunctions();
            break;
        case end_test:
            cout << "Finishi of full test." << endl;
            break;
        }

        if (!result)
            cout << endl << "Test failed." << endl;

        return result;
    }

    bool testAll() {
        for (int test_name = begin_test; test_name != end_test; ++test_name) {
            test(test_name);
        }
    }
};

#endif // PARSYBONE_TESTER_INCLUDED
