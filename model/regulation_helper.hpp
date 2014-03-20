/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warranty. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
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
};

#endif // REGULATION_HELPER_HPP
