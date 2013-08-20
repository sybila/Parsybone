/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_UNPARAMETRIZED_STRUCTURE_BUILDER_INCLUDED
#define PARSYBONE_UNPARAMETRIZED_STRUCTURE_BUILDER_INCLUDED

#include "basic_structure.hpp"
#include "unparametrized_structure.hpp"
#include "../model/model_translators.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Creates a UnparametrizedStructure as a composition of a BasicStructure and ParametrizationsHolder.
///
/// UnparametrizedStructureBuilder creates the UnparametrizedStructure from the model data.
/// States are read from the basic structure and passed to the unparametrized structure, then the transitions are added.
/// Each transition is supplemented with a label - mask of transitive values and the its function ID.
/// This expects semantically correct data from BasicStructure and FunctionsStructure.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class UnparametrizedStructureBuilder {
   // Provided with the constructor
   const Model & model;
   const BasicStructure & basic_structure; ///< Provider of basic KS data.

   /**
    * Test wheather the current state corresponds to the requirements put on values of the specified species.
    * @param source_species	species that can possibly regulate the target
    * @param source_values	in which levels the species have to be for regulation to be active
    * @param state_levels	activation levels of the current state
    * @return true it the state satisfy the requirements
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
    * @param neighbour_num	index of the neighbour state. Specie that change is used to determine wich function to use.
    * @param state_levels	species level of the state we are currently at
    * @return function that might lead to the next state
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
    * For each existing neighbour add a transition to the newly created state
    * @param ID	state from which the transitions should lead
    * @param state_levels	activation levels in this state
    */
   void addTransitions(const StateID ID, const Levels & state_levels, UnparametrizedStructure & structure) {
      // Go through all the original transitions
      for (size_t trans_num = 0; trans_num < basic_structure.getTransitionCount(ID); trans_num++) {
         // Data to fill
         const StateID target_ID = basic_structure.getTargetID(ID, trans_num); // ID of the state the transition leads to

         const SpecieID spec_ID = basic_structure.getSpecieID(ID, trans_num);
         // Find out which function is currently active
         const size_t fun_no = getActiveFunction(spec_ID, state_levels);
         // Fill the step size
         const size_t step_size = model.species[spec_ID].parameters[fun_no].step_size;
         // Fill data about transitivity using provided values
         const ActLevel level = state_levels[spec_ID];

         const bool dir = basic_structure.getDirection(ID, trans_num);

         // Add the transition
         structure.addTransition(ID, target_ID, step_size, dir, level, model.species[spec_ID].parameters[fun_no].possible_values);
      }
   }
public:
   /**
    * Constructor just attaches the references to data holders.
    */
   UnparametrizedStructureBuilder(const Model & _model, const BasicStructure & _basic_structure)
      : model(_model), basic_structure(_basic_structure) {
   }

   /**
    * Create the states from the model and fill the structure with them.
    */
   UnparametrizedStructure buildStructure() {
      UnparametrizedStructure structure;
      const size_t state_count = basic_structure.getStateCount();
      size_t state_no = 0;

      // Recreate all the states of the simple structure
      for(StateID ID = 0; ID < basic_structure.getStateCount(); ID++) {
         output_streamer.output(verbose_str, "Creating transitions for state: " + toString(++state_no) + "/" + toString(state_count) + ".", OutputStreamer::no_newl | OutputStreamer::rewrite_ln);

         // Create a new state from the known data
         const Levels & state_levels = basic_structure.getStateLevels(ID);
         structure.addState(ID, state_levels);

         // Add all the transitions
         addTransitions(ID, state_levels, structure);
      }

      output_streamer.clear_line(verbose_str);

      return structure;
   }
};

#endif // PARSYBONE_UNPARAMETRIZED_STRUCTURE_BUILDER_INCLUDED
