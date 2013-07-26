#ifndef READING_HELPER_HPP
#define READING_HELPER_HPP

#include "model.hpp"

class ReadingHelper {
   /**
    * @brief getThreshold  For a given regulator, find out what it's threshold in the given context is.
    * @param context
    * @param target_ID
    * @param name
    * @param pos
    * @return  threshold value in the given context
    */
   static size_t getThreshold(const Model & model, const string & context, const SpecieID t_ID, const string & name, const size_t pos) {
      // Regulator not present.
      if (pos == context.npos)
         return 0;
      const size_t COLON_POS = pos + name.length(); // Where colon is supposed to be.

      // Regulator level not specified.
      if (context[COLON_POS] != ':') {
         // Control if the context is unambiguous.
         auto thresholds = model.getThresholds(t_ID);
         if (thresholds.find(model.findID(name))->second.size() > 1)
            throw runtime_error ("Ambiguous context \"" + context + "\" - no threshold specified for a regulator " + name + " that has multiple regulations.");
         // If valid, add the threshold 1.
         return thresholds.find(model.findID(name))->second[0];
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
    * @brief getSingleParam
    * @param all_thrs
    * @param thrs_comb
    * @param target_ID
    * @return
    */
   static Model::Parameter getSingleParam(const Model & model, const map<SpecieID, Levels> & all_thrs, const Levels thrs_comb, const SpecieID t_ID) {
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

public:
   /**
    * @brief formCanonicContext   Transforms the regulation specification into a canonic form (\forall r \in regulator [r:threshold,...]).
    * @param context
    * @param target_ID
    * @return canonic context form
    */
   static string formCanonicContext(const Model & model, const string & context, const SpecieID t_ID) {
      string new_context; // new canonic form
      const auto names = model.getRegulatorsNames(t_ID);

      // For each of the regulator of the specie.
      for (const auto & name:names) {
         auto pos = context.find(name);
         size_t threshold = getThreshold(model, context, t_ID, name, pos);
         new_context += name + ":" + toString(threshold) + ",";
      }

      // Remove the last comma and return
      return new_context.substr(0, new_context.length() - 1);
   }

   /**
    * @brief fillActivationLevels For each regulation fill the levels of its source in which it is active.
    */
   static void fillActivationLevels(Model & model) {
      // Fill for all the species.
      for (auto ID:range(model.getSpeciesCount())) {
         auto space = model.getThresholds(ID);

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

            model.addActivityLevels(regul.source, ID, range(begin, end));
         }
      }
   }

   /**
    * @brief createParameters Creates a description of kinetic parameters.
    * @param target_ID
    * @param formula
    * @return
    */
   static Model::Parameters createParameters(const Model & model, const SpecieID t_ID) {
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

   static void fillParameters(Model & model) {
      for (const SpecieID ID : range(model.getSpeciesCount())) {
         auto params = createParameters(model, ID);
         model.setParameters(ID, params);
      }
   }
};

#endif // READING_HELPER_HPP
