#ifndef SYNTHESIS_RESULTS_HPP
#define SYNTHESIS_RESULTS_HPP

#include "../auxiliary/data_types.hpp"

class SynthesisResults {
   vector<size_t> costs; ///< costs of individual parametrizations used this round
   Paramset accepting; ///< a mask for parametrizations accepting in this round

public:
   /**
    * Fills after time series check finished.
    * @param new_costs	a vector of lenght |parameter_set| containing cost values. If the value does not exist (state is not reachable), use INF
    */
   void setResults(const vector<size_t> & new_costs, const Paramset results) {
      costs = new_costs;
      accepting = results;
   }

   /**
    * @return  max finite cost among parametrizations used this round
    */
   size_t getMaxDepth () const {
      size_t max_depth = 0;
      for (const auto depth:costs)
         max_depth = max((depth == INF) ? 0 : depth, max_depth);
      return max_depth;
   }

   /**
    * @return min cost among parametrizations used this round
    */
   size_t getMinDepth() const {
      size_t min_depth = INF;
      for (const size_t depth:costs)
         min_depth = min(depth, min_depth);
      return min_depth;
   }

   /**
    * @param number of the parametrization relative in this round
    * @return  Cost value of a particular parametrization
    */
   size_t getCost(size_t position) const {
      return costs[position];
   }

   /**
    * @return  Cost value of all the parametrizations from this round
    */
   const vector<size_t> & getCost() const {
      return costs;
   }

   /**
    * @return  mask of parametrizations that are computed acceptable in this round
    */
   const Paramset & getAcceptable() const {
      return accepting;
   }
};

#endif // SYNTHESIS_RESULTS_HPP
