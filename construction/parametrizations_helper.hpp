#ifndef PARAMETRIZATIONS_HELPER_HPP
#define PARAMETRIZATIONS_HELPER_HPP

#include "../parsing/model_translators.hpp"
#include "../auxiliary/data_types.hpp"
#include <PunyHeaders/formulae_resolver.hpp>

class ParametrizationsHelper {

public:
   /**
    * @brief isSubordinate returns true if the current context is the same as the compared context only with a lower activity value in specificed regulator.
    */
   static bool isSubordinate(const Model & model, const Model::Parameter & current, const Model::Parameter & compare, const SpecieID target_ID, const SpecieID source_ID) {
      for (auto regul_ID:ModelTranslators::getRegulatorsIDs(model, target_ID)) {
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

   /**
    * @brief fitsConditions
    * @param sat conditions set by a regulation.
    */
   static bool fitsConditions(const Model::Satisfaction & sat, const bool activating, const bool inhibiting) {
      size_t comb_type = static_cast<size_t>(activating) + 2*static_cast<size_t>(inhibiting);
      switch (comb_type) {
      case 0:
         return sat.none;
      case 1:
         return sat.activ;
      case 2:
         return sat.inhib;
      case 3:
         return sat.both;
      }
      return false;
   }
};

#endif // PARAMETRIZATIONS_HELPER_HPP
