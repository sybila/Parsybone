/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef REGULATION_HELPER_HPP
#define REGULATION_HELPER_HPP

#include "model_translators.hpp"
#include "../parsing/constraint_parser.hpp"

namespace RegulationHelper {
   /*    */
   string getLabel(const string & label) {
      string formula;

      // Find the constrain and return its valuation
      if (label.compare(Label::Activating) == 0)
         formula = "+";
      else if (label.compare(Label::ActivatingOnly) == 0)
         formula = "(+ & !-)";
      else if (label.compare(Label::Inhibiting) == 0)
         formula = "-";
      else if (label.compare(Label::InhibitingOnly) == 0)
         formula = "(- & !+)";
      else if (label.compare(Label::NotActivating) == 0)
         formula = "!+";
      else if (label.compare(Label::NotInhibiting) == 0)
         formula = "!-";
      else if (label.compare(Label::Observable) == 0)
         formula = "(+ | -)";
      else if (label.compare(Label::NotObservable) == 0)
         formula = "(!+ & !-)";
      else if (label.compare(Label::Free) == 0)
         formula = "tt";
      else
         formula = label;

	  return formula;
   }

   /**
    * @brief fillActivationLevels For each regulation fill the levels of its source in which it is active.
    */
   void fillActivationLevels(Model & model) {
      // Fill for all the species.
      for (auto ID:crange(model.species.size())) {
         auto space = ModelTranslators::getThresholds(model, ID);

         // List through the regulations of the specie.
         for (auto regul:model.getRegulations((ID))) {
            // Start at the lower threshold
            ActLevel begin = regul.threshold;
            // See if there is an upper threshold
            auto thresholds = space.find(regul.source)->second;
            sort(thresholds.begin(), thresholds.end(), [](unsigned int a, unsigned int b){return a <= b;});
            auto th_it = find(thresholds.begin(), thresholds.end(), begin) + 1;
            // Create the maximum based on whethter this is the last threshold or not.
            ActLevel end = (th_it == thresholds.end()) ? model.getMax(regul.source) + 1 : *th_it;

			regul.activity = vrange(begin, end);
         }
      }
   }
};

#endif // REGULATION_HELPER_HPP
