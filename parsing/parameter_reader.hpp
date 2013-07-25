#ifndef PARAMETER_READER_HPP
#define PARAMETER_READER_HPP

#include "model.hpp"
#include "parameter_constraints.hpp"
#include "parameter_parser.hpp"

class ParameterReader {

   /**
    * @brief getSingleParam
    * @param all_thrs
    * @param thrs_comb
    * @param target_ID
    * @param formula
    * @return
    */
   ParameterConstraints::Parameter getSingleParam(const Model & model, const map<SpecieID, Levels> & all_thrs, const Levels thrs_comb, const SpecieID t_ID) const {
      // Empty data to fill.
      ParameterConstraints::Parameter parameter = {"", map<StateID, Levels>(), Levels()};

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
   ParameterConstraints::SpecieParameters createParameters(const Model & model, const SpecieID t_ID) const {
      auto all_thrs = model.getThresholds(t_ID);
      Levels bottom, thrs_comb, top;
      ParameterConstraints::SpecieParameters parameters;

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
   ParameterConstraints computeParams(const Model & model, const ParameterParser::ParameterSpecifications & specs) {
      ParameterConstraints constraints;

      // For each specie create its parameters.
      for (SpecieID ID : range(model.getSpeciesCount())) {
         // Create all contexts with all the possible values.
         auto parameters = createParameters(model, ID);

         // If logic description is given evaluate results.
         if (!specs.param_specs[ID].l_pars.empty())
            throw runtime_error("Logical expression temporarily disabled.");

         // Otherwise replace values.
         // replaceExplicit(parameters, specie, ID);
         constraints.parameters.push_back(parameters);
      }


      return constraints;
   }
};

#endif // PARAMETER_READER_HPP
