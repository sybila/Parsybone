/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_WITNESS_SEARCHER_INCLUDED
#define PARSYBONE_WITNESS_SEARCHER_INCLUDED

#include "../construction/construction_holder.hpp"
#include "coloring_analyzer.hpp"
#include "color_storage.hpp"

class WitnessSearcher {
   const ProductStructure & product;
   const ColoringAnalyzer & analyzer;
   const ColorStorage & storage;
   ColorStorage workspace;

   std::vector<std::string> string_paths;

   std::vector<StateID> path;
   std::vector<Paramset> depth_masks;
   std::size_t depth;
   std::size_t fork_depth;
   std::size_t max_depth;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SEARCH FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   void storeTransitions(const Paramset which) {
      std::string path_str;
      for (std::size_t step = my_max(fork_depth,1); step < depth; step++) {
         path_str.append("[").append(toString(path[step])).append("<").append(toString(path[step+1])).append("]");
         workspace.update(path[step], which);
      }

      Paramset marker = paramset_helper.getLeftOne();
      for (std::size_t param = 0; param < paramset_helper.getParamsetSize(); param++) {
         if (which & marker)
            string_paths[param].append(path_str);
         marker >>= 1;
      }
      fork_depth = depth;
   }

   void DFS(const StateID ID, Paramset paramset) {
      path[depth] = ID;
      if (depth > max_depth)
         throw std::runtime_error("Depth boundary overcome.");

      Paramset connected = workspace.getColor(ID) & paramset;
      if (connected)
         storeTransitions(connected);
      paramset &= ~connected;

      if (product.isInitial(ID)) {
         storeTransitions(paramset);
      }

      paramset &= ~depth_masks[depth];
      if (paramset) {
         depth++;
         auto predecessors = workspace.getNeighbours(ID, false, paramset);
         for (auto pred = predecessors.begin(); pred != predecessors.end(); pred++) {
            DFS(*pred, paramset);
         }
         depth--;
      }
      fork_depth = my_min(depth-1, fork_depth);
   }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREATION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   void clearPaths() {
      if (string_paths.size() != paramset_helper.getParamsetSize())
         string_paths.resize(paramset_helper.getParamsetSize(), "");
      else
         forEach(string_paths, [](std::string & single_path){single_path = "";});
      path = std::vector<StateID>(getMaxDepth() + 1, 0);
   }

   void prepareMasks() {
      depth_masks.clear();
      std::vector<std::vector<std::size_t> > members(getMaxDepth() + 1);
      std::size_t param_num = 0;
      forEach(workspace.getCost(), [&members, &param_num](std::size_t current){
         if (current != ~0)
            members[current].push_back(param_num);
         param_num++;
      });
      forEach(members, [&](std::vector<std::size_t> numbers){
          depth_masks.push_back(paramset_helper.getMaskFromNums(numbers));
      });
   }

   WitnessSearcher(const WitnessSearcher & other); ///< Forbidden copy constructor.
   WitnessSearcher& operator=(const WitnessSearcher & other); ///< Forbidden assignment operator.

public:
   /**
    * Constructor, passes the data
    */
   WitnessSearcher(const ConstructionHolder & _holder, const ColoringAnalyzer & _analyzer, const ColorStorage & _storage)
      : product(_holder.getProduct()), analyzer(_analyzer), storage(_storage) {
   }

   const std::size_t getMaxDepth () const {
      std::size_t depth = 0;
      forEach(workspace.getCost(), [&depth](std::size_t current){depth = my_max((current == ~0 ? 0 : current), depth);});
      return depth;
   }

   const std::vector<std::string> getOutput () {
      workspace = storage;
      for (StateID ID = 0; ID < product.getStateCount(); ID++) {
         workspace.remove(ID, ~0);
      }
      clearPaths();
      prepareMasks();
      depth = fork_depth = 0;
      max_depth = getMaxDepth();

      auto finals = product.getFinalStates();
      for (auto final = finals.begin(); final != finals.end(); final++) {
         DFS(*final, storage.getColor(*final));
      }

      std::vector<std::string> acceptable_paths;
      for (auto path_it = string_paths.begin(); path_it != string_paths.end(); path_it++)
         if (!path_it->empty())
            acceptable_paths.push_back(std::move(*path_it));
      return acceptable_paths;
   }
};

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// Class for display of witnesses for all colors in current round.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//#include "../construction/construction_holder.hpp"
//#include "paramset_helper.hpp"
//#include "color_storage.hpp"
//#include "per_color_storage.hpp"
//#include "coloring_analyzer.hpp"

//class WitnessSearcher {
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// DATA
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//   const ProductStructure & product;
//   const ColoringAnalyzer & analyzer;
//   const ColorStorage & storage;
//   std::unique_ptr<PerColorStorage>(per_color_storage);

//	// Witness counting related auxiliary variables:
//	std::vector<StateID> path; // IDs of states alongside the path
//	std::map<StateID, long double> state_robustness;
//	std::set<StateID> used_paths;
//	std::vector<double> probability; // Probability of stepping into each consecutive state
//	std::size_t lenght; // Lenght of the path, non-zero value
//	std::size_t color_num; // Ordinal of the color
//	std::size_t max_path_lenght; // Reference value for output of shortest paths only
//	std::string witness_str; // Variable to store the witness in

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// SEARCH FUNCTIONS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CREATION FUNCTIONS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	WitnessSearcher(const WitnessSearcher & other);            // Forbidden copy constructor.
//	WitnessSearcher& operator=(const WitnessSearcher & other); // Forbidden assignment operator.

//public:
//	/**
//	 * Get reference data and create final states that will hold all the computed data
//	 */
//	 WitnessSearcher(const ConstructionHolder & holder, const ColoringAnalyzer & _analyzer, const ColorStorage & _storage)
//						 : product(holder.getProduct()), analyzer(_analyzer), storage(_storage) {
//		// Resize path for maximal possible lenght
//		path.resize(product.getStateCount() - holder.getAutomatonStructure().getStateCount() * 2 + 2);
//		lenght = color_num = max_path_lenght = 0;
//	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// OUTPUT FUNCTIONS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//private:
//	/**
//	 * Add probability of getting to the requested final state from specific initial state alongside this path
//	 */
//	void countProb() {
//		// Probability of being in the first state
//		long double probability = 1.0;
//		// For each state divide probability by all its successors
//		for (std::size_t state_index = (lenght - 1); state_index > 1; state_index--) {
//			probability /= per_color_storage->getNeighbours(path[state_index], true, color_num).size();
//		}

//		// Check if this initial state is already included, if so, add probability, otherwise create new state with probability of this path
//		auto state_prob = state_robustness.find(path[(lenght - 1)]);
//		if (state_prob == state_robustness.end())
//			state_robustness.insert(std::make_pair(path[(lenght - 1)], probability));
//		else
//			state_prob->second += probability;
//	}

//	/**
//	 * Outputs a witness path as a sequence of states.
//	 */
//	void storeWit() {
//		for (std::size_t state_index = lenght; state_index > 1; state_index--)
//				witness_str.append(product.getString(path[state_index- 1]));
//		  witness_str.append(",");
//	}

//	/**
//	 * Recursive DFS search function for the witness path
//	 */
//	void DFS(const StateID ID) {
//		// Add yourself to the path
//		path[lenght++] = ID;

//		// If you are initial, print this path
//		if (product.isInitial(ID)) {
//			used_paths.insert(path[1]);
//				if (user_options.witnesses())
//				storeWit();
//			if (user_options.robustness())
//				countProb();
//		}
//		else if (lenght < max_path_lenght) { // Continue DFS only if witness has still allowed lenght
//			// Get predecessors
//			const Neighbours & preds = per_color_storage->getNeighbours(ID, false, color_num);

//			// List through predecessors
//			if (preds.size())
//				for (Neighbours::const_iterator pred_it = preds.begin(); pred_it != preds.end(); pred_it++) {
//					// Do not reuse paths
//					if (used_paths.find(*pred_it) != used_paths.end())
//						continue;
//					// If it was not used yet, continue in DFS
//					DFS(*pred_it);
//				}
//		}

//		// Return
//		--lenght;
//	}

//public:
//	const std::vector<std::string> getOutput () {
//		std::vector<std::string> results;

//      // Get synthetized colors
//        auto masks = paramset_helper.getSingleMasks(analyzer.getMask());
//      per_color_storage.reset(new PerColorStorage(analyzer, storage, product));

//		// Go through colors
//		for (color_num = 0; color_num < masks.size(); color_num++) {
//			std::string result;

//         // Get round values
//            std::size_t bit_num = paramset_helper.getBitNum(masks[color_num]);
//            max_path_lenght = storage.getCost(bit_num);

//				witness_str = "{";
//			// Compute witnesses for given color from each final state, they are also displayed, if requested
//			for (auto final_it = product.getFinalStates().begin(); final_it != product.getFinalStates().end(); final_it++) {
//				// Restart values
//				lenght = 0;

//				// Start search
//				DFS(*final_it);
//			}
//				result.append(witness_str);
//				result.back() = '}';

//			// Display robustness if requested
//			if (user_options.robustness()) {
//				double robustness = 0.0;
//				// For each possible initial state add robustness
//				forEach(state_robustness, [&robustness](const std::pair<StateID, double> & start) {
//					robustness += start.second;
//				});
//				// Divide by number of inital states and ouput
//				robustness /= state_robustness.size();
//					 result = toString(robustness) + result;

//				state_robustness.clear();
//			}

//			results.push_back(result);
//			used_paths.clear();
//		}
//		return results;
//	}
//};


// SECOND ATTEMPT
//#include "../construction/construction_holder.hpp"
//#include "coloring_analyzer.hpp"
//#include "color_storage.hpp"

//class WitnessSearcher {
//   const ConstructionHolder & holder;
//   const ColoringAnalyzer & analyzer;
//   const ColorStorage & storage;
//   ColorStorage workspace;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CREATION FUNCTIONS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//   WitnessSearcher(const WitnessSearcher & other);            // Forbidden copy constructor.
//   WitnessSearcher& operator=(const WitnessSearcher & other); // Forbidden assignment operator.

//public:
//   /**
//    * Constructor, passes the data
//    */
//   WitnessSearcher(const ConstructionHolder & _holder, const ColoringAnalyzer & _analyzer, const ColorStorage & _storage)
//      : holder(_holder), analyzer(_analyzer), storage(_storage) {
//   }
//   const std::size_t getMaxBreadth () const {
//      std::size_t breadth = 0;
//      forEach(workspace.getCost(), [&breadth](std::size_t current){breadth = my_max((current == ~0 ? 0 : current), breadth);});
//      return breadth;
//   }

//   const Paramset getUnpassed(const StateID ID) const {
//      auto individuals = workspace.getMarking(ID, true);
//      Paramset passed = 0;
//      forEach(individuals, [&passed](Paramset current){passed |= current;});
//      return (workspace.getColor(ID) & ~passed);
//   }

//   void removeUnpassed(const StateID ID, const Paramset unpassed) {
//      // Remove from self
//      workspace.remove(ID, unpassed);
//      // Remove from my predecessors
//      workspace.remove(ID, unpassed, false);
//      // Remove from mine predecessors'sucessors
//      auto predecessors = workspace.getNeighbours(ID, false, unpassed);
//      for (auto predIDit = predecessors.begin(); predIDit != predecessors.end(); predIDit++) {
//         workspace.remove(*predIDit, ID, unpassed, true);
//      }
//   }

//   void reduceStorage() {
//      auto max_breadth = getMaxBreadth();
//      for (std::size_t round_num = 0; round_num < max_breadth; round_num++) {
//         for(StateID ID = 0; ID < holder.getProduct().getStateCount(); ID++) {
//            if (!holder.getProduct().isFinal(ID))
//               removeUnpassed(ID, getUnpassed(ID));
//         }
//      }
//   }

//   const std::string getTransitions(const Paramset current) {
//      std::string trans_str;
//      for(StateID ID = 0; ID < holder.getProduct().getStateCount(); ID++) {
//         auto predecessors = workspace.getNeighbours(ID, false, current);
//         for (auto ID_it = predecessors.begin(); ID_it != predecessors.end(); ID_it++) {
//            trans_str.append("<").append(holder.getProduct().getString(*ID_it)).append("->")
//                  .append(holder.getProduct().getString(ID)).append(">");
//         }
//      }
//      return trans_str;
//   }

//   const std::vector<std::string> getOutput () {
//      std::vector<std::string> transitions;
//      workspace = storage;
//      reduceStorage();
//      auto masks = paramset_helper.getSingleMasks(workspace.getAcceptable());
//      for (auto mask_it = masks.begin(); mask_it != masks.end(); mask_it++){
//         transitions.push_back(getTransitions(*mask_it));
//      }
//      return transitions;
//   }
//};
