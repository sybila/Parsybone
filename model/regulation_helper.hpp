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

class RegulationHelper {
   /**
    * @brief resolveLabel  Return true if the label (edge constrain) of the regulation is satisfied, false otherwise. All labels can be resolved based only on whether mon+ and mon- are true.
    * @param	activating	true if the parametrization satisfies +
    * @param	inhibiting	true if the parametrization satisfies -
    * @param	label	canonical form of edge label given as a string
    * @return	true if the edge constrain is satisfied
    */
   static bool resolveLabel(const bool activating, const bool inhibiting, const string label) {
      // Fill the atomic propositions
      FormulaeResolver::Vals values;
      values.insert(FormulaeResolver::Val("+", activating));
      values.insert(FormulaeResolver::Val("-", inhibiting));

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
         formula = "!(+ | -)";
      else if (label.compare(Label::Free) == 0)
         formula = "tt";
      else
         formula = label;

      return (FormulaeResolver::resolve(values, formula));
   }

public:
   /**
    * @brief getThreshold  For a given regulator, find out what it's threshold in the given context is.
    * @return  threshold value in the given context
    */
   static ActLevel getThreshold(const Model & model, const string & context, const SpecieID t_ID, const string & name, const size_t pos) {
      // Regulator not present.
      if (pos == context.npos)
         return 0;
      const size_t COLON_POS = pos + name.length(); // Where colon is supposed to be.

      // Regulator level not specified.
      if (context[COLON_POS] != ':') {
         // Control if the context is unambiguous.
         auto thresholds = ModelTranslators::getThresholds(model, t_ID);
         if (thresholds.find(ModelTranslators::findID(model,name))->second.size() > 1)
            throw runtime_error ("Ambiguous context \"" + context + "\" - no threshold specified for a regulator " + name + " that has multiple regulations.");
         // If valid, add the threshold 1.
         return thresholds.find(ModelTranslators::findID(model, name))->second[0];
      }

      // There is not a threshold given after double colon.
      if (context[COLON_POS] == ':' && (COLON_POS == (context.npos - 1) || !isdigit(context[COLON_POS+1])))
         throw runtime_error ("No threshold given after colon in the context \"" + context + "\" of the regulator " + name);

      // Add a threshold if uniquely specified.
      string to_return;
      size_t number = 1;
      // Copy all the numbers
      while(isdigit(context[COLON_POS + number])) {
         to_return.push_back(context[COLON_POS + number]);
         number++;
      }
      return (boost::lexical_cast<size_t>(to_return));
   }

   /**
    * @brief fillActivationLevels For each regulation fill the levels of its source in which it is active.
    */
   static void fillActivationLevels(Model & model) {
      // Fill for all the species.
      for (auto ID:range(model.species.size())) {
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

            regul.activity = range(begin, end);
         }
      }
   }

   /**
    * @brief setConditions set conditions on nature of the regulation based on its label.
    */
   static void fillConditions(Model & model) {
      for (const SpecieID ID : range(model.species.size())) {
         Model::Regulations & reguls = model.species[ID].regulations;
         for (Model::Regulation & regul:reguls) {
            regul.satisf.none = resolveLabel(false, false, regul.label);
            regul.satisf.activ = resolveLabel(true, false, regul.label);
            regul.satisf.inhib = resolveLabel(false, true, regul.label);
            regul.satisf.both = resolveLabel(true, true, regul.label);
         }
      }
   }
};

#endif // REGULATION_HELPER_HPP
