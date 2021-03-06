#include "kinetics_test_data.hpp"

/// This test controls functionality of loop bounding constraint.
TEST_F(KineticsTest, ParametrizationLoopBound) {
	Model mod_loo;
	mod_loo.addSpecie("A", 1, Model::Component);
	mod_loo.addSpecie("B", 5, Model::Component);
	mod_loo.addRegulation(0, 1, 1, "");
	mod_loo.addRegulation(1, 1, 2, "");
	mod_loo.addRegulation(1, 1, 4, "");
	mod_loo.restrictions.bound_loop = true;
	
	auto params = ParameterBuilder::buildParams(mod_loo)[1].params;

	ASSERT_EQ(6, params.size()) << "There should be 6 contexts for B.";
	ASSERT_EQ(3, params[0].targets.size()) << "Targets {0,1,2}";
	EXPECT_EQ(2, params[0].targets[2]) << "Targets {0,1,2}";
	ASSERT_EQ(4, params[1].targets.size()) << "Targets {1,2,3,4}";
	EXPECT_EQ(1, params[1].targets[0]) << "Targets {1,2,3,4}";
}

TEST_F(KineticsTest, CorrectInput) {
	// Inputs have no subparametrizations
	for (const SpecieID ID : cscope(mod_cas.species)) {
		bool is_input = (mod_cas.species[ID].spec_type == Model::Input);
		bool no_cols = (kin_cas_one.species[ID].params.size() == 0);
		ASSERT_TRUE(is_input == no_cols);
	}
}

//
//TEST_F(KineticsTest, NonFunctional) {
//	// B is set to 1 by experiment - others should be non-functional with no parametrizations.
//	for (auto & param_of_A : kin_cir_exp.species[0].params) {
//		bool has_B_1 = (param_of_A.context.find("B:1") != string::npos);
//		ASSERT_TRUE(has_B_1 == param_of_A.functional);
//		ASSERT_TRUE(param_of_A.functional != param_of_A.target_in_subcolor.empty());
//	}
//
//	EXPECT_STREQ("(-1,0,0,1)", KineticsTranslators::createParamString(kin_cir_exp, 0).c_str());
//}