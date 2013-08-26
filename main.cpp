/*! \mainpage Welcome to the Parsybone code documentation.
 *
 * This is a documentation of code belonging solely to the Parsybone tool, version 1.2.
 * This text is not supposed to be a user manual in any way, but as an reference for further development of this tool.
 * For a description of usage of the tool, please refer to the Manual that is shipped together with the tool.
 *
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

using namespace std;

#include "PunyHeaders/time_manager.hpp"

#include "auxiliary/output_streamer.hpp"
#include "auxiliary/user_options.hpp"
#include "parsing/argument_parser.hpp"
#include "parsing/data_parser.hpp"
#include "parsing/parsing_manager.hpp"
#include "construction/construction_manager.hpp"
#include "construction/product_builder.hpp"
#include "synthesis/synthesis_manager.hpp"

/**
 * Execution of succesive parts of the parameter synthesis.
 */
int main(int argc, char* argv[]) {
   time_manager.startClock("* Runtime", false);
   Model model;
   PropertyAutomaton property;
   ProductStructure product;

   try {
      ParsingManager::parseOptions(argc, argv);
      model = ParsingManager::parseModel(user_options.model_path + user_options.model_name + MODEL_SUFFIX);
      property = ParsingManager::parseProperty(user_options.model_path + user_options.model_name + MODEL_SUFFIX);
   } catch (std::exception & e) {
      output_streamer.output(error_str, string("Error occured while parsing input: \"").append(e.what()).append("\"."));
      return 1;
   }

   try {
      ConstructionManager::computeModelProps(model);
      product = ConstructionManager::construct(model, property);
   } catch (std::exception & e) {
      output_streamer.output(error_str, string("Error occured while building the data structures: \"").append(e.what()).append("\"."));
      return 2;
   }

   try {
      SynthesisManager synthesis_manager(product, model, property);
      synthesis_manager.doSynthesis();
   } catch (std::exception & e) {
      output_streamer.output(error_str, string("Error occured while syntetizing the parameters: \"").append(e.what()).append("\"."));
      return 2;
   }

   if (user_options.be_verbose) {
      time_manager.writeClock("* Runtime");
   }
   return 0;
}
