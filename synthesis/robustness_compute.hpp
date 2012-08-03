#ifndef PARSYBONE_ROBUSTNESS_COMPUTE_INCLUDE
#define PARSYBONE_ROBUSTNESS_COMPUTE_INCLUDE

#include "witness_searcher.hpp"

class RobustnessCompute {
   const ProductStructure & product; ///< Product reference for state properties
   const ColorStorage & storage; ///< Constant storage with the actuall data
   const WitnessSearcher & searcher;

   struct Marking {
      std::vector<unsigned char> exits; ///< For each parametrization stores a number of transitions this state can be left through under given parametrization.
      std::vector<double> current_prob;
   };

   std::vector<Marking> markings;

   void clear() {
      forEach(markings, [](Marking & marking){
         marking.exits.assign(marking.exits.size(), 0);
         marking.current_prob.assign(marking.current_prob.size(), 0.0);
      });
   }

   void computeExits() {
      Paramset current_mask = paramset_helper.getLeftOne();
      for (std::size_t param_num = 0; param_num < paramset_helper.getParamsetSize(); param_num++, current_mask <<= 1) {
         if (!(current_mask & storage.getAcceptable()))
            continue;
         for (StateID ID = 0; ID < product.getStateCount(); ID++) {
            markings[ID].exits[param_num] = storage.getNeighbours(ID, true, current_mask).size();
         }
      }
   }

   RobustnessCompute(const RobustnessCompute & other); ///< Forbidden copy constructor.
   RobustnessCompute& operator=(const RobustnessCompute & other); ///< Forbidden assignment operator.

public:
   /**
    * Constructor, passes the data
    */
   RobustnessCompute(const ConstructionHolder & _holder, const ColorStorage & _storage,  const WitnessSearcher & _searcher)
      : product(_holder.getProduct()), storage(_storage), searcher(_searcher) {
      Marking empty = {std::vector<unsigned char>(paramset_helper.getParamsetSize(), 0), std::vector<double>(paramset_helper.getParamsetSize(), 0)};
      markings.resize(product.getStateCount(), empty);
   }

   void compute() {
      clear();
   }

   const std::vector<std::string> getOutput() const {
      std::vector<std::string> to_return;
      for (std::size_t counter = 0; counter < paramset_helper.count(storage.getAcceptable()); counter++) {
         to_return.push_back(std::string("0.5"));
      }
      return to_return;
   }
};

#endif // PARSYBONE_ROBUSTNESS_COMPUTE_INCLUDE
