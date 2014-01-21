/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verifiAtion tool.
 * ParSyBoNe is a free software: you An redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef MODEL_TEST_DATA_HPP
#define MODEL_TEST_DATA_HPP

#include <gtest/gtest.h>

#include "../model/parameter_reader.hpp"
#include "../model/regulation_helper.hpp"
#include "../construction/construction_manager.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Testing models and properties definitions.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ModelsTest : public ::testing::Test {
protected:
   Model trivial_model;
   Model one_three;
   Model circuit_2;
   Model bool_k_2;
   PropertyAutomaton trivial_prop;
   PropertyAutomaton o_t_series_prop;
   PropertyAutomaton A_cyclic_prop;
   PropertyAutomaton set_two_ones_prop;
   PropertyAutomaton arbitrary_peak_prop;
   PropertyAutomaton stable_prop;
   PropertyAutomaton bistable_prop;
   PropertyAutomaton experiment_series;

   void setUpModels() {
      trivial_model.addSpecie("A", 1, {0,1});
      trivial_model.addRegulation(0,0,1,"Free");

      one_three.addSpecie("A", 1, {0,1});
      one_three.addSpecie("B", 3, {0,1,2,3});
      one_three.addRegulation(0, 1, 1, "+");
      one_three.addRegulation(0, 0, 1, "-");
      one_three.addRegulation(1, 0, 1, "-");
      one_three.addRegulation(1, 0, 3, "(-|+)");

      circuit_2.addSpecie("A", 1, {0,1});
      circuit_2.addSpecie("B", 1, {0,1});
      circuit_2.addRegulation(0, 1, 1, "+");
      circuit_2.addRegulation(1, 0, 1, "-");

      bool_k_2.addSpecie("A", 1, {0,1});
      bool_k_2.addSpecie("B", 1, {0,1});
      bool_k_2.addRegulation(0, 0, 1, "Free");
      bool_k_2.addRegulation(0, 1, 1, "Free");
      bool_k_2.addRegulation(1, 0, 1, "Free");
      bool_k_2.addRegulation(1, 1, 1, "Free");
   }

   void setUpAutomata() {
      trivial_prop.addState("triv0", true);
      trivial_prop.addEdge(0,0,{"tt"});

      o_t_series_prop = PropertyAutomaton(TimeSeries);
      o_t_series_prop.addState("ser0", false);
      o_t_series_prop.addState("ser1", false);
      o_t_series_prop.addState("ser2", true);
      o_t_series_prop.addEdge(0,1,{"(A=0|B=0)"});
      o_t_series_prop.addEdge(1,1,{"tt"});
      o_t_series_prop.addEdge(1,2,{"(A=1|B=3)"});
      o_t_series_prop.addEdge(2,2,{"ff"});

      A_cyclic_prop = PropertyAutomaton(LTL);
      A_cyclic_prop.addState("cyc0", false);
      A_cyclic_prop.addState("cyc1", true);
      A_cyclic_prop.addState("cyc2", false);
      A_cyclic_prop.addEdge(0,0,{"A=0"});
      A_cyclic_prop.addEdge(0,1,{"A=1"});
      A_cyclic_prop.addEdge(1,0,{"A=0"});
      A_cyclic_prop.addEdge(1,2,{"A=1"});
      A_cyclic_prop.addEdge(2,1,{"A=0"});
      A_cyclic_prop.addEdge(2,2,{"A=1"});

      set_two_ones_prop = PropertyAutomaton(TimeSeries);
      set_two_ones_prop.addState("ser0", false);
      set_two_ones_prop.addState("ser1", false);
      set_two_ones_prop.addState("ser2", true);
      set_two_ones_prop.addEdge(0,1,{"(A=0&B=1)"});
      set_two_ones_prop.addEdge(1,1,{"tt"});
      set_two_ones_prop.addEdge(1,2,{"(A=1&B=1)"});
      set_two_ones_prop.addEdge(2,2,{"ff"});

      arbitrary_peak_prop = PropertyAutomaton();
      arbitrary_peak_prop.addState("low1",false);
      arbitrary_peak_prop.addState("high",false);
      arbitrary_peak_prop.addState("low2",false);
      arbitrary_peak_prop.addState("final",true);
      arbitrary_peak_prop.addEdge(0,1,{"((A=0&B=1)|(A=1&B=0))"});
      arbitrary_peak_prop.addEdge(1,1,{"tt"});
      arbitrary_peak_prop.addEdge(1,2,{"(A=1&B=1)"});
      arbitrary_peak_prop.addEdge(2,2,{"tt"});
      arbitrary_peak_prop.addEdge(2,3,{"((A=0&B=1)|(A=1&B=0))"});
      arbitrary_peak_prop.addEdge(3,3,{"tt"});

      stable_prop = PropertyAutomaton(TimeSeries);
      stable_prop.addState("init",false);
      stable_prop.addState("stable",false);
      stable_prop.addState("final",true);
      stable_prop.addEdge(0,0,{"tt"});
      stable_prop.addEdge(0,1,{"(A=0&B=0)",true,false});
      stable_prop.addEdge(1,2,{"tt",false, true});
      stable_prop.addEdge(2,2,{"ff"});

      bistable_prop = PropertyAutomaton(TimeSeries);
	  bistable_prop.min_acc = 2;
      bistable_prop.addState("init",false);
      bistable_prop.addState("stable",false);
      bistable_prop.addState("final",true);
      bistable_prop.addEdge(0,0,{"tt"});
      bistable_prop.addEdge(0,1,{"(A=0&B=0)",true,false});
      bistable_prop.addEdge(1,2,{"tt",false, true});
      bistable_prop.addEdge(2,2,{"ff"});

	  experiment_series = PropertyAutomaton(TimeSeries);
	  experiment_series.addState("ser0", false);
	  experiment_series.addState("ser1", false);
	  experiment_series.addState("ser2", true);
	  experiment_series.addEdge(0, 1, { "(A=0)" });
	  experiment_series.addEdge(1, 1, { "tt" });
	  experiment_series.addEdge(1, 2, { "(A=1)" });
	  experiment_series.addEdge(2, 2, { "ff" });
	  experiment_series.experiment = "B=1";

   }

   void SetUp() override {
      setUpModels();
      setUpAutomata();
   }
};

#endif // MODEL_TEST_DATA_HPP
