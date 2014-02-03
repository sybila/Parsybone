/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warranty. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_CONSTRUCTION_MANAGER_INCLUDED
#define PARSYBONE_CONSTRUCTION_MANAGER_INCLUDED

#include "../model/parameter_reader.hpp"
#include "../model/parametrizations_builder.hpp"
#include "../model/labeling_builder.hpp"
#include "automaton_builder.hpp"
#include "unparametrized_structure_builder.hpp"
#include "product_builder.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief STEP 2 - Builds all the structures and stores them within a ConstructionHolder.
///
/// ConstructionManager overviews the whole process of construction of structures from information contained within a model file.
/// All the objects constructed are stored within a provided CostructionHolder and further acessible only via constant getters.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace ConstructionManager {
   /**
    * @brief computeModelProps
    */
   void computeModelProps(Model & model) {
      // Add levels to the regulations.
      RegulationHelper::fillActivationLevels(model);
      // Compute parameter values.
      ParameterHelper::fillParameters(model);
      // Replace explicitly defined parameters.
      ParameterReader::constrainParameters(model);

	  vector<bool> allowed_states;
      // Compute exact parametrization for the model.
      ParametrizationsBuilder::buildParametrizations(model);
      // Build labels for regulations.
      LabelingBuilder::buildLabeling(model);
   }

   /**
    * Function that constructs all the data in a cascade of temporal builders.
    */
   ProductStructure construct(const Model & model, const PropertyAutomaton & property) {
      // Create the UKS
      UnparametrizedStructureBuilder unparametrized_structure_builder(model, property);
      UnparametrizedStructure unparametrized_structure = unparametrized_structure_builder.buildStructure();

      // Create the Buchi automaton
      AutomatonBuilder automaton_builder(model, property);
      AutomatonStructure automaton = automaton_builder.buildAutomaton();

      // Create the product
      ProductBuilder product_builder;
      ProductStructure product = product_builder.buildProduct(move(unparametrized_structure), move(automaton));
      return product;
   }
}


#endif // PARSYBONE_CONSTRUCTION_MANAGER_INCLUDED
