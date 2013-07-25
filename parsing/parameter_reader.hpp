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
   void replaceInContext(Model::Parameters & parameters, const string & in_context, const string & can_context, const Levels & targets) const {
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
   Levels covertToLevels(const Model & model, const string & val_str, const SpecieID t_ID) const {
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
   Levels interpretLevels(const Model & model, const string & val_str, const SpecieID t_ID) const {
      // ? goes for unspecified.
      if (val_str.compare("?") == 0)
         return model.getBasalTargets(t_ID);
      else
         return covertToLevels(model, val_str, t_ID);
   }

   /**
    * @brief replaceExplicit  change given precomputed values in given target for explicit list of new ones.
    * @param model   model currently without parameters
    * @param constraints   constraints on parameters created by enumeration
    * @param k_params   specification given by the user
    * @param t_ID
    */
   void replaceExplicit(const Model & model, Model::Parameters & constraints, const ParameterParser::ParsList & k_params, const SpecieID t_ID) const {
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

   /**
    * @brief getSingleParam
    * @param all_thrs
    * @param thrs_comb
    * @param target_ID
    * @return
    */
   Model::Parameter getSingleParam(const Model & model, const map<SpecieID, Levels> & all_thrs, const Levels thrs_comb, const SpecieID t_ID) const {
      // Empty data to fill.
      Model::Parameter parameter = {"", map<StateID, Levels>(), Levels()};

      // Loop over all the sources.
      for (auto source_num:range(thrs_comb.size())) {
         // Find the source details and its current threshold
         string source_name = model.getRegulatorsNames(t_ID)[source_num];
         StateID source_ID = model.getRegulatorsIDs(t_ID)[source_num];
         auto thresholds = all_thrs.find(source_ID)->second;

         // Find activity level of the current threshold.
         ActLevel threshold = (thrs_comb[source_num] == 0) ? 0 : thresholds[thrs_comb[source_num] - 1];

         // Add current regulation as present.
         string regulation_name = source_name + ":" + toString(threshold);

         // Add the regulation to the source
         parameter.context += regulation_name + ",";

         // Find in which levels the specie must be for the regulation to occur.
         ActLevel next_th = (thrs_comb[source_num] == thresholds.size()) ? model.getMax(source_ID) + 1 : thresholds[thrs_comb[source_num]];
         Levels activity_levels = range(threshold, next_th);
         parameter.requirements.insert(make_pair(source_ID, activity_levels));
      }

      parameter.targets = model.getBasalTargets(t_ID);

      // Remove the last comma and return.
      parameter.context = parameter.context.substr(0, parameter.context.length() - 1);
      return parameter;
   }

   /**
    * @brief createParameters Creates a description of kinetic parameters.
    * @param target_ID
    * @param formula
    * @return
    */
   Model::Parameters createParameters(const Model & model, const SpecieID t_ID) const {
      auto all_thrs = model.getThresholds(t_ID);
      Levels bottom, thrs_comb, top;
      Model::Parameters parameters;

      // These containers hold number of thresholds per regulator.
      for (auto & source_thresholds:all_thrs) {
         bottom.push_back(0);
         thrs_comb.push_back(0);
         top.push_back(source_thresholds.second.size());
      }

      // Loop over all the contexts.
      do {
         parameters.push_back(getSingleParam(model, all_thrs, thrs_comb, t_ID));
      } while(iterate(top, bottom, thrs_comb));

      return parameters;
   }

public:
   /**
    * @brief computeParams
    * @param model
    * @param specs
    * @return
    */
   void computeParams(const ParameterParser::ParameterSpecifications & specs, Model & model) {
      // For each specie create its parameters.
      for (SpecieID ID : range(model.getSpeciesCount())) {
         // Create all contexts with all the possible values.
         auto parameters = createParameters(model, ID);

         // If logic description is given evaluate results.
         if (!specs.param_specs[ID].l_pars.empty())
            throw runtime_error("Logical expression temporarily disabled.");

         // Otherwise replace values.
         replaceExplicit(model, parameters, specs.param_specs[ID].k_pars, ID);

         // Add newly created parameters.
         model.addParameters(ID, parameters);
      }
   }
};

#endif // PARAMETER_READER_HPP
