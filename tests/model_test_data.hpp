/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verifiAtion tool.
 * ParSyBoNe is a free software: you An redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warranty. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#pragma once

#include <gtest/gtest.h>

#include "../model/regulation_helper.hpp"
#include "../construction/construction_manager.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Testing models and properties definitions.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ModelsTest : public ::testing::Test {
protected:
   Model mod_tri; //< Trivial model
   Model mod_mul; //< Multivalue model (1,3)
   Model mod_cir; //< Boolean circuit
   Model mod_com; //< Complete boolean two node graph
   Model mod_cas; //< Three node cascade
   PropertyAutomaton ltl_tri; //< Trivial property
   PropertyAutomaton ltl_mul; //< Multi-valued serise
   PropertyAutomaton ltl_cyc; //< Cyclic property
   PropertyAutomaton ltl_one; //< Set two ones property
   PropertyAutomaton ltl_top; //< Have a peak either on A or on B
   PropertyAutomaton ltl_sta; //< Stable state
   PropertyAutomaton ltl_bst; //< Bistable prop
   PropertyAutomaton ltl_exp; //< Series with experiment

   void setUpModels() {
	  mod_tri.addSpecie("A", 1, Model::Component);
      mod_tri.addRegulation(0,0,1,"Free");

      mod_mul.addSpecie("A", 1,Model::Component);
	  mod_mul.addSpecie("B", 3, Model::Component);
      mod_mul.addRegulation(0, 1, 1, "+");
      mod_mul.addRegulation(0, 0, 1, "-");
      mod_mul.addRegulation(1, 0, 1, "-");
      mod_mul.addRegulation(1, 0, 3, "(-|+)");

	  mod_cir.addSpecie("A", 1, Model::Component);
	  mod_cir.addSpecie("B", 1, Model::Component);
      mod_cir.addRegulation(0, 1, 1, "+");
      mod_cir.addRegulation(1, 0, 1, "-");

	  mod_com.addSpecie("A", 1, Model::Component);
	  mod_com.addSpecie("B", 1, Model::Component);
      mod_com.addRegulation(0, 0, 1, "Free");
      mod_com.addRegulation(0, 1, 1, "Free");
      mod_com.addRegulation(1, 0, 1, "Free");
      mod_com.addRegulation(1, 1, 1, "Free");

	  mod_cas.addSpecie("A", 1, Model::Input);
	  mod_cas.addSpecie("B", 1, Model::Input);
	  mod_cas.addSpecie("C", 1, Model::Component);
	  mod_cas.addRegulation(0, 2, 1, "ActivatingOnly");
	  mod_cas.addRegulation(1, 2, 1, "NotActivating");
   }

   void setUpAutomata() {
      ltl_tri.addState("triv0", true);
      ltl_tri.addEdge(0,0,{"tt"});

      ltl_mul = PropertyAutomaton(TimeSeries);
      ltl_mul.addState("ser0", false);
      ltl_mul.addState("ser1", false);
      ltl_mul.addState("ser2", true);
      ltl_mul.addEdge(0,1,{"(A=0|B=0)"});
      ltl_mul.addEdge(1,1,{"tt"});
      ltl_mul.addEdge(1,2,{"(A=1|B=3)"});
      ltl_mul.addEdge(2,2,{"ff"});

      ltl_cyc = PropertyAutomaton(LTL);
      ltl_cyc.addState("cyc0", false);
      ltl_cyc.addState("cyc1", true);
      ltl_cyc.addState("cyc2", false);
      ltl_cyc.addEdge(0,0,{"A=0"});
      ltl_cyc.addEdge(0,1,{"A=1"});
      ltl_cyc.addEdge(1,0,{"A=0"});
      ltl_cyc.addEdge(1,2,{"A=1"});
      ltl_cyc.addEdge(2,1,{"A=0"});
      ltl_cyc.addEdge(2,2,{"A=1"});

      ltl_one = PropertyAutomaton(TimeSeries);
      ltl_one.addState("ser0", false);
      ltl_one.addState("ser1", false);
      ltl_one.addState("ser2", true);
      ltl_one.addEdge(0,1,{"(A=0&B=1)"});
      ltl_one.addEdge(1,1,{"tt"});
      ltl_one.addEdge(1,2,{"(A=1&B=1)"});
      ltl_one.addEdge(2,2,{"ff"});

      ltl_top = PropertyAutomaton();
      ltl_top.addState("low1",false);
      ltl_top.addState("high",false);
      ltl_top.addState("low2",false);
      ltl_top.addState("final",true);
      ltl_top.addEdge(0,1,{"((A=0&B=1)|(A=1&B=0))"});
      ltl_top.addEdge(1,1,{"tt"});
      ltl_top.addEdge(1,2,{"(A=1&B=1)"});
      ltl_top.addEdge(2,2,{"tt"});
      ltl_top.addEdge(2,3,{"((A=0&B=1)|(A=1&B=0))"});
      ltl_top.addEdge(3,3,{"tt"});

      ltl_sta = PropertyAutomaton(TimeSeries);
      ltl_sta.addState("init",false);
      ltl_sta.addState("stable",false);
      ltl_sta.addState("final",true);
      ltl_sta.addEdge(0,0,{"tt"});
      ltl_sta.addEdge(0,1,{"(A=0&B=0)",true,false});
      ltl_sta.addEdge(1,2,{"tt",false, true});
      ltl_sta.addEdge(2,2,{"ff"});

      ltl_bst = PropertyAutomaton(TimeSeries);
	  ltl_bst.min_acc = 2;
      ltl_bst.addState("init",false);
      ltl_bst.addState("stable",false);
      ltl_bst.addState("final",true);
      ltl_bst.addEdge(0,0,{"tt"});
      ltl_bst.addEdge(0,1,{"(A=0&B=0)",true,false});
      ltl_bst.addEdge(1,2,{"tt",false, true});
      ltl_bst.addEdge(2,2,{"ff"});

	  ltl_exp = PropertyAutomaton(TimeSeries);
	  ltl_exp.addState("ser0", false);
	  ltl_exp.addState("ser1", false);
	  ltl_exp.addState("ser2", true);
	  ltl_exp.addEdge(0, 1, { "(A=0)" });
	  ltl_exp.addEdge(1, 1, { "tt" });
	  ltl_exp.addEdge(1, 2, { "(A=1)" });
	  ltl_exp.addEdge(2, 2, { "ff" });
	  ltl_exp.experiment = "B=1";

   }

   void SetUp() override {
      setUpModels();
      setUpAutomata();
   }
};
