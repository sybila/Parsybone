#ifndef PARAMETER_READER_HPP
#define PARAMETER_READER_HPP

#include "reading_helper.hpp"
#include "parameter_parser.hpp"

class ParameterReader {
   /**
    * @brief replaceInContext
    * @param parameters
    * @param in_context
    * @param can_context
    * @param targets
    */
   static void replaceInContext(Model::Parameters & parameters, const string & in_context, const string & can_context, const Levels & targets) {
      // List through all parameters of the specie.
      for(auto & param:parameters) {
         // If the context is matched.
         if (param.context.compare(can_context) == 0) {
            param.targets = targets;
            return;
         }
      }

      // If the context was not ever matched.
      throw runtime_error("Given context " + in_context + " not mached, probably incorrect.");
   }

   /**
    * @brief covertToLevels   Take a string of the form (\d,)*\d and transform it into a list of values.
    * @param val_str
    * @return
    */
   static Levels covertToLevels(const Model & model, const string & val_str, const SpecieID t_ID) {
      vector<string> numbers;
      Levels specified;
      split(numbers, val_str, is_any_of(","));

      // Convert the string into list of numbers.
      for (const auto & num:numbers) {
         size_t val;

         // Convert one number.
         try {
            val = lexical_cast<size_t>(num);
         } catch (bad_lexical_cast) {
            throw runtime_error("Specified value " + num + " in the list " + val_str + " is not a number");
         }

         if (val < model.getMin(t_ID) || val > model.getMax(t_ID))
            throw invalid_argument("target value " + val_str + " out of range for specie " + model.getName(t_ID));

         specified.push_back(val);
      }

      return specified;
   }


   /**
    * @brief interpretLevels Obtain values from a value string.
    * @param model
    * @param val_str
    * @param t_ID
    * @return list of possible target values
    */
   static Levels interpretLevels(const Model & model, const string & val_str, const SpecieID t_ID) {
      // ? goes for unspecified.
      if (val_str.compare("?") == 0)
         return model.getBasalTargets(t_ID);
      else
         return covertToLevels(model, val_str, t_ID);
   }

   /**
    * @brief replaceExplicit  Change given precomputed values in given target for explicit list of new ones.
    * @param model   model currently without parameters
    * @param constraints   constraints on parameters created by enumeration
    * @param k_params   specification given by the user
    * @param t_ID
    */
   static void replaceExplicit(const Model & model, Model::Parameters & constraints, const ParameterParser::ParsList & k_params, const SpecieID t_ID) {
      // List through all the PARAM nodes.
      for (const auto & param : k_params) {
         // Obtain context specified.
         string can_context = ReadingHelper::formCanonicContext(model, param.first, t_ID);

         // Get the levels.
         Levels targets = interpretLevels(model, param.second, t_ID);

         // Find the context and replace it's target values.
         replaceInContext(constraints, param.first, can_context, targets);
      }
   }

public:
   /**
    * @brief computeParams Constraints parameter values based on explicit specifications given by the user.
    * @attention  this will not apply edge constraints.
    *
    * @param model   model to fill the data in
    */
   static void computeParams(const ParameterParser::ParameterSpecifications & specs, Model & model) {
      // For each specie create its parameters.
      for (SpecieID ID : range(model.getSpeciesCount())) {
         // Create all contexts with all the possible values.
         auto parameters = model.getParameters(ID);

         // If logic description is given evaluate results.
         if (!specs.param_specs[ID].l_pars.empty())
            throw runtime_error("Logical expression temporarily disabled.");

         // Otherwise replace values.
         replaceExplicit(model, parameters, specs.param_specs[ID].k_pars, ID);

         // Add newly created parameters.
         model.setParameters(ID, parameters);
      }
   }
};

#endif // PARAMETER_READER_HPP
