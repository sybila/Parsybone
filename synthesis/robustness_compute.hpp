#ifndef PARSYBONE_ROBUSTNESS_COMPUTE_INCLUDE
#define PARSYBONE_ROBUSTNESS_COMPUTE_INCLUDE

#include "witness_searcher.hpp"
#include <float.h>

class RobustnessCompute {
   const ProductStructure & product; ///< Product reference for state properties
   const ColorStorage & storage; ///< Constant storage with the actuall data
   const WitnessSearcher & searcher;

   struct Marking {
      std::vector<unsigned char> exits; ///< For each parametrization stores a number of transitions this state can be left through under given parametrization.
      std::vector<double> current_prob;
      std::vector<double> next_prob;
   };

   std::vector<Marking> markings;
   std::vector<double> results;

   void clear() {
      forEach(markings, [](Marking & marking){
         marking.exits.assign(marking.exits.size(), 0);
         marking.current_prob.assign(marking.current_prob.size(), 0.0);
         marking.next_prob.assign(marking.next_prob.size(), 0.0);
      });
      results.assign(paramset_helper.getParamsetSize(), 0.0);
   }

   void computeExits() {
      Paramset current_mask = paramset_helper.getLeftOne();
      for (std::size_t param_num = 0; param_num < paramset_helper.getParamsetSize(); param_num++, current_mask >>= 1) {
         if (!(current_mask & storage.getAcceptable()))
            continue;
         for (StateID ID = 0; ID < product.getStateCount(); ID++) {
            markings[ID].exits[param_num] = storage.getNeighbours(ID, true, current_mask).size();
         }
      }
   }

   void initiate() {
      forEach(product.getInitialStates(), [&](StateID ID) {
         forEach(markings[ID].next_prob, [](double & prob) {
            prob = 1.0;
         });
      });
   }

   void finish() {
      forEach(product.getFinalStates(), [&](StateID ID) {
         for (std::size_t param_num = 0; param_num < results.size(); param_num++) {
            results[param_num] += markings[ID].next_prob[param_num];
         }
      });
   }

   RobustnessCompute(const RobustnessCompute & other); ///< Forbidden copy constructor.
   RobustnessCompute& operator=(const RobustnessCompute & other); ///< Forbidden assignment operator.

public:
   /**
    * Constructor, passes the data
    */
   RobustnessCompute(const ConstructionHolder & _holder, const ColorStorage & _storage,  const WitnessSearcher & _searcher)
      : product(_holder.getProduct()), storage(_storage), searcher(_searcher) {
      Marking empty = {std::vector<unsigned char>(paramset_helper.getParamsetSize(), 0),
         std::vector<double>(paramset_helper.getParamsetSize(), 0.0),
         std::vector<double>(paramset_helper.getParamsetSize(), 0.0)};
      markings.resize(product.getStateCount(), empty);
      results.resize(paramset_helper.getParamsetSize(), 0.0);
   }

   void compute() {
      clear();
      computeExits();
      initiate();

      auto transitions = searcher.getTransitions();
      for (std::size_t round_num = 0; round_num < storage.getMaxDepth(); round_num++) {
         forEach(markings, [](Marking & marking) {
            marking.current_prob = marking.next_prob;
            marking.next_prob.assign(marking.next_prob.size(), 0.0);
         });
         initiate();
         for (std::size_t param_num = 0; param_num != paramset_helper.getParamsetSize(); param_num++) {
            forEach(transitions[param_num], [&](std::pair<StateID, StateID> trans) {
               std::size_t divisor = markings[trans.first].exits[param_num];
               if (divisor)
                  markings[trans.second].next_prob[param_num] += markings[trans.first].current_prob[param_num] / divisor ;
            });
         }
      }

      finish();
   }

   const std::vector<std::string> getOutput() const {
      std::vector<std::string> to_return;
      forEach(results, [&to_return](double robust){
         std::string string_val("<");
         if (robust)
            to_return.push_back(string_val.append(toString(robust)).append(">"));
      });

      return to_return;
   }
};

#endif // PARSYBONE_ROBUSTNESS_COMPUTE_INCLUDE
