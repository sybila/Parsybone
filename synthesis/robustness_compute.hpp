/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_ROBUSTNESS_COMPUTE_INCLUDE
#define PARSYBONE_ROBUSTNESS_COMPUTE_INCLUDE

#include "witness_searcher.hpp"
#include "checker_setting.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Class responsible for computation of robustness values for each acceptable parametrization.
///
/// @attention Now broken due to the fact that initials do not match initial measurement.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class RobustnessCompute {
   const ProductStructure & product; ///< Product reference for state properties.
   const ColorStorage & storage; ///< Constant storage with the actuall data.
   CheckerSettings settings; ///< Setup for the process.

   /// This structure holds values used in the iterative process of robustness computation.
   vector<size_t> exits; ///< A number of transitions this state can be left through under given parametrization.
   vector<double> current_prob; ///< Current probability of reaching.
   vector<double> next_prob; ///< Will store the probability in the next round.

   /**
    * For each state compute how many exists are under each parametrization.
    */
   void computeExits(const vector<StateTransition> & transitions) {
      // If not acceptable, leave zero
      for (const StateTransition & tran : transitions) {
         const vector<StateID> transports = ColoringFunc::broadcastParameters(settings.getParamNo(), product.getStructure(), product.getKSID(tran.first));

         // Consider only the steps that go towards a single state of the BA (picked the highest).
         exits[tran.first] += max(1u, transports.size());
      }
   }

   /**
    * Set probability of each initial state to 1.0 / number of used initial states for this parametrization.
    */
   void initiate() {
      exits.assign(exits.size(), 0);
      current_prob.assign(current_prob.size(), 0.);
      next_prob = current_prob;

      setInitials();
   }

   /**
    * @brief setInitials
    */
   void setInitials() {
      const vector<StateID> & initials = settings.getInitials(product);

      for (const StateID init:initials)
         next_prob[init] = 1.0 / initials.size();
   }

   /**
    * Compute the resulting values as a sum of probabilites of reaching any state.
    */
   void finish() {

   }

public:
   /**
    * Constructor ensures that data objects used within the whole computation process have appropriate size.
    */
   RobustnessCompute(const ProductStructure & _product, const ColorStorage & _storage) : product(_product), storage(_storage) {
      exits.resize(product.getStateCount());
      current_prob.resize(product.getStateCount());
      next_prob.resize(product.getStateCount());
   }

   /**
    * Function that computes robustness values for each parametrization.
    */
   void compute(const SynthesisResults & results, const vector<pair<StateID,StateID> > & transitions, const CheckerSettings & _settings) {
      settings = _settings;
      initiate();
      computeExits(transitions);

      // Assign probabilites for the initial states
      setInitials();

      // Cycle through the levels of the DFS procedure
      for (size_t round_num = 0; round_num < results.lower_bound; round_num++) {
         // Copy the data from the previous round.
         current_prob = next_prob;
         next_prob.assign(next_prob.size(), 0.);

         // For the parametrization cycle through transitions
         for (const auto & trans:transitions) {
            size_t divisor = exits[trans.first]; // Count succesor
            // Add probabilities
            if (divisor)
               next_prob[trans.second] += current_prob[trans.first] / divisor ;
         }
      }

      finish();
   }

   /**
    * @return the current robustness
    */
   double getRobustness() const {
      double robustness = 0.;
      for (const StateID ID:settings.getFinals(product))
         robustness += next_prob[ID];
      return robustness;
   }

   /**
    * @return robustness measure on each final satate
    */
   vector<double> getFinalMarkings() const {
      vector<double> markings;
      markings.reserve(settings.getFinals(product).size());
      for (const StateID ID:settings.getFinals(product))
         markings.push_back(next_prob[ID]);
      return markings;
   }
};

#endif // PARSYBONE_ROBUSTNESS_COMPUTE_INCLUDE
