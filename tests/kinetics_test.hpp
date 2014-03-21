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
	
	auto params = ParameterHelper::buildParams(mod_loo)[1];

	ASSERT_EQ(6, params.size()) << "There should be 6 contexts for B.";
	ASSERT_EQ(3, params[0].targets.size()) << "Targets {0,1,2}";
	EXPECT_EQ(2, params[0].targets[2]) << "Targets {0,1,2}";
	ASSERT_EQ(4, params[3].targets.size()) << "Targets {1,2,3,4}";
	EXPECT_EQ(1, params[3].targets[0]) << "Targets {1,2,3,4}";
}