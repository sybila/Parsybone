#pragma once

#include "model_test_data.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Creatin of kinetics for the analysis
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class KineticsTest : public ModelsTest {
protected:
	Kinetics kin_cas_one;
	Kinetics kin_cir_one;
	Kinetics kin_cir_cyc;
	Kinetics kin_cir_exp;
	Kinetics kin_com_bst;
	Kinetics kin_com_cyc;
	Kinetics kin_com_sta;
	Kinetics kin_com_top;
	Kinetics kin_com_tri;
	Kinetics kin_mul_cyc;
	Kinetics kin_mul_mul;
	Kinetics kin_tri_tri;

	void SetUp() override {
		ModelsTest::SetUp();

		kin_cas_one = ConstructionManager::computeKinetics(mod_cas, ltl_one);
		kin_cir_one = ConstructionManager::computeKinetics(mod_cir, ltl_one);
		kin_cir_cyc = ConstructionManager::computeKinetics(mod_cir, ltl_cyc);
		kin_cir_exp = ConstructionManager::computeKinetics(mod_cir, ltl_exp);
		kin_com_bst = ConstructionManager::computeKinetics(mod_com, ltl_bst);
		kin_com_cyc = ConstructionManager::computeKinetics(mod_com, ltl_cyc);
		kin_com_sta = ConstructionManager::computeKinetics(mod_com, ltl_sta);
		kin_com_top = ConstructionManager::computeKinetics(mod_com, ltl_top);
		kin_com_tri = ConstructionManager::computeKinetics(mod_com, ltl_tri);
		kin_mul_cyc = ConstructionManager::computeKinetics(mod_mul, ltl_cyc);
		kin_mul_mul = ConstructionManager::computeKinetics(mod_mul, ltl_mul);
		kin_tri_tri = ConstructionManager::computeKinetics(mod_tri, ltl_tri);
	}
};