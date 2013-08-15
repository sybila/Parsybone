/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_PARAMETRIZATIONS_BUILDER_INCLUDED
#define PARSYBONE_PARAMETRIZATIONS_BUILDER_INCLUDED

#include "PunyHeaders/common_functions.hpp"
#include "PunyHeaders/formulae_resolver.hpp"

#include "../auxiliary/data_types.hpp"
#include "parametrizations_helper.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Class that computes feasible parametrizations for each specie from the edge constrains and stores them in a ParametrizationHolder object.
///
/// This construction may be optimized by including the warm-start constraint satisfaction.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ParametrizationsBuilder {
   static ParamNum color_no; ///< Just for output, the total amount of sub-colors.
   static ParamNum color_tested; ///< Just for output, the current amount of sub-colors tested.

   /**
    * Test specific constrain on given color - this function checks both observability and the edge constrain.
    * @param activating	a reference to the variable storing true iff the regulation has observable activating effect
    * @param inhibiting a reference to the variable storing true iff the regulation has observable inhibiting effect
    * @param ID	ID of the specie that undergoes the test
    * @param param_num	index of tested parameter
    * @param regul_num	index of regulation whose constrains are tested
    * @param subcolor	coloring for this specie that is tested
    * @return	true if constrains are satisfied
    */
   static void testConstrains(const Model & model, const SpecieID target_ID, const size_t & param_num, const Model::Regulation & regul,
                              const Levels & subparam, bool & activating, bool & inhibiting ) {
      // Get reference data
      const auto & parameters = model.getParameters(target_ID);
      const StateID source_ID = regul.source;
      ActLevel threshold = parameters[param_num].requirements.find(source_ID)->second.front();
      if (threshold == 0)
         return;
      else
         threshold--;

      size_t compare_num = 0;
      while(compare_num < parameters.size()) {
         auto compare = parameters[compare_num];
         if (ParametrizationsHelper::isSubordinate(model, parameters[param_num], compare, target_ID, source_ID))
            break;
         else
            compare_num++;
      }

      // Assign regulation aspects
      activating |= subparam[param_num] > subparam[compare_num];
      inhibiting |= subparam[param_num] < subparam[compare_num];
   }

   /**
    * Tests if given subparametrization on a given specie can satisfy given requirements.
    * @param ID	ID of the specie to test contexts in
    * @param subcolor	unique valuation of all regulatory contexts
    * @return	true if the subparametrization is feasible
    */
   static bool testSubparametrization (const Model & model, const SpecieID ID, const Levels & subparam) {
      // get referecnces to Specie data
      const auto & regulations = model.getRegulations(ID);
      const auto & parameters = model.getParameters(ID);

      // Cycle through all species's regulators
      for (const Model::Regulation & regul:regulations) {
         // Skip if there are no requirements (free label)
         if (regul.label.compare(Label::Free) == 0)
            continue;

         // Prepare variables storing info about observable effects of this component
         bool activating = false, inhibiting = false;
         // For each parameter containing the reugulator in parametrization control its satisfaction
         for (auto param_num:range(parameters.size())) {
            testConstrains(model, ID, param_num, regul, subparam, activating, inhibiting);
         }

         // Test obtained knowledge agains the label itself - return false if the label is not satisfied
         if (!ParametrizationsHelper::fitsConditions(regul.satisf, activating, inhibiting))
            return false;
      }

      // If everything has passed, return true
      return true;
   }

   /**
    * Test all possible subcolors and saves valid.
    * @param valid	data storage to save in
    * @param ID	ID of currently used specie
    * @param colors_num	how many colors are there together
    * @param bottom_color	low bound on possible contexts
    * @param top_color	top bound on possible contexts
    */
   static void testColors(Model & model, const SpecieID ID, const Levels & bottom_color, const Levels & top_color) {
      // Cycle through all possible subcolors for this specie
      Levels subcolor(bottom_color);

      // Cycle through all colors
      do {
         outputProgress();
         // Test if the parametrization satisfies constraints.
         if (!testSubparametrization(model, ID, subcolor))
            continue;
         model.species[ID].subcolors.push_back(subcolor);
      } while (iterate(top_color, bottom_color, subcolor));
      output_streamer.clear_line(verbose_str);

      if (model.species[ID].subcolors.empty())
         throw runtime_error(string("No valid parametrization found for the specie ").append(toString(ID)));
   }

   /**
    * @brief outputProgress
    */
   static inline void outputProgress() {
      output_streamer.output(verbose_str, "Testing edge constraints on partiall parametrizations: " + toString(++color_tested) + "/" + toString(color_no) + ".", OutputStreamer::no_newl | OutputStreamer::rewrite_ln);
   }

   /**
    * For this specie, test all possible subcolors (all valuations of this specie contexts) and store those that satisfy edge labels.
    * @param ID specie used in this round
    */
   static void createKinetics(Model & model, const SpecieID ID) {
      // Data to fill

      // Create boundaries for iteration
      Levels bottom_color, top_color;
      ParametrizationsHelper::getBoundaries(model.getParameters(ID), bottom_color, top_color);
      // model.species[ID].possible_count = ParametrizationsHelper::getPossibleCount(model.getParameters(ID));

      // Test all the subcolors and save feasible
      testColors(model, ID, bottom_color, top_color);
   }

public:
   /**
    * Entry function of parsing, tests and stores subcolors for all the species.
    */
   static void buildParametrizations(Model & model) {
      color_tested = 0;
      color_no = 0;
      for (SpecieID ID = 0; ID < model.species.size(); ID++)
         color_no += ParametrizationsHelper::getPossibleCount(model.getParameters(ID));

      // Cycle through species
      for (SpecieID ID = 0; ID < model.species.size(); ID++)
         createKinetics(model, ID);

      output_streamer.output(verbose_str, "", OutputStreamer::no_out | OutputStreamer::rewrite_ln | OutputStreamer::no_newl);
   }
};
ParamNum ParametrizationsBuilder::color_tested;
ParamNum ParametrizationsBuilder::color_no;

#endif // PARSYBONE_PARAMETRIZATIONS_BUILDER_INCLUDED
