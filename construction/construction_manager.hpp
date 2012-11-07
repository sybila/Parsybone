/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_CONSTRUCTION_MANAGER_INCLUDED
#define PARSYBONE_CONSTRUCTION_MANAGER_INCLUDED

#include "construction_holder.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief STEP 2 - Builds all the structures and stores them within a ConstructionHolder.
///
/// ConstructionManager overviews the whole process of construction of structures from information contained within a model file.
/// All the objects constructed are stored within a provided CostructionHolder and further acessible only via constant getters.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ConstructionManager {
   ConstructionHolder & holder; ///< Reference to the object that will finally contain all the constructed objects.

	ConstructionManager(const ConstructionManager & other); ///< Forbidden copy constructor.
	ConstructionManager& operator=(const ConstructionManager & other); ///< Forbidden assignment operator.

public:
   /**
    * Constructor, passes the reference.
    *
    * @param _holder  object that will hold all the constructed objects
    */
   ConstructionManager(ConstructionHolder & _holder) : holder(_holder) { }

   /**
    * Function that constructs all the data in a cascade of temporal builders.
    */
   void construct() {
      output_streamer.output(verbose_str, "Kinetic parameters building started.");
      // Create possible kinetic parameters
      ParametrizationsHolder * parametrizations_holder = new ParametrizationsHolder;
      ParametrizationsBuilder parametrizations_builder(holder.getModel(), *parametrizations_holder);
      parametrizations_builder.buildParametrizations();
      holder.fillConstrains(parametrizations_holder);

      // Create implicit form of the kinetic parameters used as edge labels
      LabelingHolder * labeling_holder = new LabelingHolder;
      LabelingBuilder labeling_builder(holder.getModel(), holder.getParametrizations(), *labeling_holder);
      labeling_builder.buildLabeling();
      holder.fillLabeling(labeling_holder);

		output_streamer.output(verbose_str, "Parametrized Kripke structure building started.");
		// Create a simple Kripke structure without parametrization
		BasicStructure * basic_structure = new BasicStructure; // Kripke structure built from the network
		BasicStructureBuilder basic_structure_builder(holder.getModel(), *basic_structure);
		basic_structure_builder.buildStructure();
		holder.fillBasicStructure(basic_structure);

		// Create the UKS
		UnparametrizedStructure * unparametrized_structure = new UnparametrizedStructure; // Kripke structure that has transitions labelled with functions
		UnparametrizedStructureBuilder unparametrized_structure_builder(holder.getBasicStructure(), holder.getLabeling(), *unparametrized_structure);
		unparametrized_structure_builder.buildStructure();
		holder.fillUnparametrizedStructure(unparametrized_structure);

		output_streamer.output(verbose_str, "Buchi automaton building started.");
		// Create the Buchi automaton
		AutomatonStructure * automaton = new AutomatonStructure; // Set of transitions - controlling automaton
		AutomatonBuilder automaton_builder(holder.getModel(), *automaton);
		automaton_builder.buildAutomaton();
		holder.fillAutomaton(automaton);

		output_streamer.output(verbose_str, "Product building started.");
		// Create the product
		ProductStructure * product_structure = new ProductStructure(holder.getUnparametrizedStructure(), holder.getAutomatonStructure());
		ProductBuilder product_builder(holder.getUnparametrizedStructure(), holder.getAutomatonStructure(), *product_structure);
		product_builder.buildProduct();
		holder.fillProduct(product_structure);
   }
};


#endif // PARSYBONE_CONSTRUCTION_MANAGER_INCLUDED
