/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_CONSTRUCTION_MANAGER_INCLUDED
#define PARSYBONE_CONSTRUCTION_MANAGER_INCLUDED

#include "construction_holder.hpp"
#include "../model/parameter_reader.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief STEP 2 - Builds all the structures and stores them within a ConstructionHolder.
///
/// ConstructionManager overviews the whole process of construction of structures from information contained within a model file.
/// All the objects constructed are stored within a provided CostructionHolder and further acessible only via constant getters.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace ConstructionManager {
   /**
    * @brief computeModelProps
    */
   void computeModelProps(Model & model) {
      // Add levels to the regulations.
      RegulationHelper::fillActivationLevels(model);
      // Set conditions on the edges.
      RegulationHelper::setConditions(model);
      // Compute parameter values.
      ParameterHelper::fillParameters(model);
      // Replace explicitly defined parameters.
      ParameterReader::computeParams(model);

      ParametrizationsBuilder::buildParametrizations(model);

      LabelingBuilder::buildLabeling(model);
   }

   /**
    * Function that constructs all the data in a cascade of temporal builders.
    */
   ConstructionHolder construct(const Model & model, const vector<PropertyAutomaton> & properties) {
      ConstructionHolder holder;

      // Create a simple Kripke structure without parametrization
      BasicStructure * basic_structure = new BasicStructure; // Kripke structure built from the network
      BasicStructureBuilder basic_structure_builder(model, *basic_structure);
      basic_structure_builder.buildStructure();
      holder.fillBasicStructure(basic_structure);

      // Create the UKS
      UnparametrizedStructure * unparametrized_structure = new UnparametrizedStructure; // Kripke structure that has transitions labelled with functions
      UnparametrizedStructureBuilder unparametrized_structure_builder(model, holder.getBasicStructure(), *unparametrized_structure);
      unparametrized_structure_builder.buildStructure();
      holder.fillUnparametrizedStructure(unparametrized_structure);

      // Create the Buchi automaton
      AutomatonBuilder automaton_builder(model);
      for (size_t i : scope(properties)) {
         holder.fillAutomaton(automaton_builder.buildAutomaton(properties[i]));
      }

      // Create the product
      // WARNING: now takes only a single automaton at a time
      ProductStructure * product_structure = new ProductStructure(holder.getUnparametrizedStructure(), holder.getAutomatonStructure(0));
      ProductBuilder product_builder(holder.getUnparametrizedStructure(), holder.getAutomatonStructure(0), *product_structure);
      product_builder.buildProduct();
      holder.fillProduct(product_structure);

      return holder;
   }
}


#endif // PARSYBONE_CONSTRUCTION_MANAGER_INCLUDED
