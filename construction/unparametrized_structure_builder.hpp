/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_UNPARAMETRIZED_STRUCTURE_BUILDER_INCLUDED
#define PARSYBONE_UNPARAMETRIZED_STRUCTURE_BUILDER_INCLUDED

#include "unparametrized_structure.hpp"
#include "../model/model_translators.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Creates a UnparametrizedStructure as a composition of a BasicStructure and ParametrizationsHolder.
///
/// UnparametrizedStructureBuilder creates the UnparametrizedStructure from the model data.
/// States are read from the basic structure and passed to the unparametrized structure, then the transitions are added.
/// Each transition is supplemented with a label - mask of transitive values and the its function ID.
/// This expects semantically correct data from BasicStructure and FunctionsStructure.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class UnparametrizedStructureBuilder {
   const Model & model;
   const PropertyAutomaton & property;

   vector<size_t> index_jumps; ///< Holds index differences between two neighbour states in each direction for each specie.
   vector<bool> allowed_states;

   /**
    * @return Returns true if the transition may be ever feasible from this state.
    */
   bool isFeasible(const Levels & parameter_vals, const bool direction, const ActLevel level) {
      for (const ActLevel val : parameter_vals) {
         if (direction) {
            if (val > level) {
               return true;
            }
         } else {
            if (val < level) {
               return true;
            }
         }
      }
      return false;
   }

   /**
    * @brief addTransition add a transition if feasible
    */
   void addTransition(const StateID ID, const StateID target, const SpecieID specie, const bool direction, const Levels & state_levels, UnparametrizedStructure & structure) {
      // Find out which function is currently active
      const size_t fun_no = getActiveFunction(specie, state_levels);
      // Fill the step size
      const ParamNo step_size = model.species[specie].parameters[fun_no].step_size;
      // Reference target values
      const Levels & parameter_vals = model.species[specie].parameters[fun_no].parameter_vals;

      if (isFeasible(parameter_vals, direction, state_levels[specie]))
         structure.addTransition(ID, target, step_size, direction, state_levels[specie], parameter_vals);
   }

   /**
    * @brief addTransitions   add all transitions for the given specie
    */
   void addTransitions(const StateID ID, const Levels & state_levels, UnparametrizedStructure & structure) {
      for (size_t specie = 0; specie < model.species.size(); specie++) {
         // If this value is not the lowest one, add neighbour with lower
         if (state_levels[specie] > 0) {
            const StateID target_ID = ID - index_jumps[specie];
            addTransition(ID, target_ID, specie, false, state_levels, structure);
         }
         // If this value is not the highest one, add neighbour with higher
		 if (state_levels[specie] < structure.maxes[specie]) {
            const StateID target_ID = ID + index_jumps[specie];
            addTransition(ID, target_ID, specie, true, state_levels, structure);
         }
      }
   }

   /**
    * Test wheather the current state corresponds to the requirements put on values of the specified species.
    */
   bool testRegulators(const map<StateID, Levels> requirements, const Levels & state_levels) {
      // List throught regulating species of the function
      for (auto regul:requirements)
         if (count(regul.second.begin(), regul.second.end(), state_levels[regul.first]) == 0)
            return false;

      // Return true if all species passed.
      return true;
   }

   /**
    * Obtain index of the function that might lead to the specified state based on current activation levels of the species and target state.
    */
   size_t getActiveFunction(const SpecieID ID, const Levels & state_levels) {
      // Cycle until the function is found
      bool found = false;
      for (size_t param_no = 0; param_no < model.getParameters(ID).size(); param_no++) {
         found = testRegulators(model.getParameters(ID)[param_no].requirements, state_levels);

         if (found)
            return param_no;
      }
      throw runtime_error("Active function in some state not found.");
   }

   /**
    * Compute a vector of maximal levels and store information about states
    */
   size_t computeBoundaries(UnparametrizedStructure & structure) {
      size_t states_count = 1;

      for(size_t specie_num = 0; specie_num < model.species.size(); specie_num++) {
         // Maximal values of species
         structure.maxes.emplace_back(model.getMax(specie_num));
		 structure.mins.emplace_back(model.getMin(specie_num));
		 const size_t diff = model.getMax(specie_num) - model.getMin(specie_num);
		 structure.diffs.emplace_back(diff);
         // How many states
		 states_count *= (diff + 1);
      }

	  return states_count;
   }

   /**
    * Creates a vector of index differences between neighbour states in each direction.
    * Value is difference of the indexes between two states that were generated as a cartesian product.
    * Differences are caused by the way the states are generated.
    */
   void computeJumps() {
      index_jumps.resize(model.species.size());
      // How many far away are two neighbour in the vector
      size_t jump_lenght = 1;
      // Species with higher index cause bigger differences
      for (size_t specie_num = 0; specie_num < model.species.size(); specie_num++) {
         index_jumps[specie_num] = jump_lenght;
         jump_lenght *= (model.getMax(specie_num) + 1);
      }
   }

   void prepareAllowed(const UnparametrizedStructure & structure, const size_t state_count) {
	   if (state_count * property.getStatesCount() > structure.states.max_size())
		   throw runtime_error("The number of states of the product (" + to_string(state_count * property.getStatesCount()) +
		   " is bigger than the maximum of " + to_string(structure.states.max_size()));
	   allowed_states.resize(state_count);
   }

   void solveConstrains(UnparametrizedStructure & structure) {
	   ConstraintParser * cons_pars = new ConstraintParser(structure.maxes.size(), *max_element(structure.maxes.begin(), structure.maxes.end()));

	   // Impose constraints
	   cons_pars->addBoundaries(structure.maxes, true);
	   cons_pars->applyFormula(ModelTranslators::getAllNames(model), property.getExperiment());

	   // Conduct search
	   DFS<ConstraintParser> search(cons_pars);
	   delete cons_pars;
	   while (ConstraintParser *result = search.next()) {
		   allowed_states[structure.getID(result->getSolution())] = true;
		   delete result;
	   }
   }

public:
   /**
    * Constructor just attaches the references to data holders.
    */
	UnparametrizedStructureBuilder(const Model & _model, const PropertyAutomaton & _property) : model(_model), property(_property) {}

   /**
    * Create the states from the model and fill the structure with them.
    */
   UnparametrizedStructure buildStructure() {
      UnparametrizedStructure structure;
	  // Compute species-related values
	  const size_t state_count = computeBoundaries(structure);
	  // Compute transitions-related values
	  computeJumps();
	  // Solve constraints
	  prepareAllowed(structure, state_count);
	  solveConstrains(structure);

      size_t state_no = 0;

      // Create states
      StateID ID = 0;
      Levels levels(model.species.size(), 0);
      do {
         output_streamer.output(verbose_str, "Creating transitions for state: " + to_string(++state_no) + "/" + to_string(state_count) + ".", 
			 OutputStreamer::no_newl | OutputStreamer::rewrite_ln);
         // Fill the structure with the state
         structure.addState(ID, levels);
         addTransitions(ID, levels, structure);
         ID++;
	  } while (iterate(structure.maxes, structure.mins, levels));

      output_streamer.clear_line(verbose_str);

      return structure;
   }
};

#endif // PARSYBONE_UNPARAMETRIZED_STRUCTURE_BUILDER_INCLUDED
