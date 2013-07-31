#ifndef PARAMETRIZATIONS_HELPER_HPP
#define PARAMETRIZATIONS_HELPER_HPP

#include "../parsing/model.hpp"
#include "../auxiliary/data_types.hpp"
#include <PunyHeaders/formulae_resolver.hpp>

class ParametrizationsHelper {

public:
   /**
    * @brief isSubordinate returns true if the current context is the same as the compared context only with a lower activity value in specificed regulator.
    */
   static bool isSubordinate(const Model & model, const Model::Parameter & current, const Model::Parameter & compare, const SpecieID target_ID, const SpecieID source_ID) {
      for (auto regul_ID:model.getRegulatorsIDs(target_ID)) {
         // All the regulations must have the same requirements.
         if (regul_ID != source_ID) {
            if (current.requirements.find(regul_ID)->second != compare.requirements.find(regul_ID)->second) {
               return false;
            }
         }
         // Except for the one with the specified source.
         else if (current.requirements.find(regul_ID)->second.front() != compare.requirements.find(regul_ID)->second.back() + 1) {
            return false;
         }
      }
      return true;
   }


   /**
    * @brief resolveLabel  Return true if the label (edge constrain) of the regulation is satisfied, false otherwise. All labels can be resolved based only on whether mon+ and mon- are true.
    * @param	activating	true if the parametrization satisfies +
    * @param	inhibiting	true if the parametrization satisfies -
    * @param	label	canonical form of edge label given as a string
    * @return	true if the edge constrain is satisfied
    */
   static bool resolveLabel(const bool & activating, const bool & inhibiting, const string label) {
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
      else
         formula = label;

      return (FormulaeResolver::resolve(values, formula));
   }

   /**
    * Compute and store boundaries on possible context values - used for iterations.
    */
   static void getBoundaries(const Model::Parameters & params, vector<size_t> & bottom_color, vector<size_t> & top_color) {
      for (auto param:params) {
         bottom_color.push_back(param.targets.front());
         top_color.push_back(param.targets.back());
      }
   }

   /**
    * @brief getPossibleCount compute the number of possible colors
    */
   static ParamNum getPossibleCount(const Model::Parameters & params) {
      ParamNum colors_num = 1;
      for (auto param:params)
         colors_num *= param.targets.size();
      return colors_num;
   }
};

#endif // PARAMETRIZATIONS_HELPER_HPP
