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
#include "parametrizations_builder.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ConstructionManager overviews the whole process of construction of structures from information contained within a model file.
/// All the objects constructed are stored within a provided CostructionHolder and further acessible only via constant getters.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ConstructionManager {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   ConstructionHolder & holder; ///< Reference to the object that will finally contain all the constructed objects

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION FUNCTIONS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   void constructBasicStructure() {

   }

	ConstructionManager(const ConstructionManager & other); ///< Forbidden copy constructor.
	ConstructionManager& operator=(const ConstructionManager & other); ///< Forbidden assignment operator.

public:
   /**
    * Constructor
    *
    * @param _holder  object that will hold all the constructed objects
    */
   ConstructionManager(ConstructionHolder & _holder) : holder(_holder) { }

   void construct() {
      output_streamer.output(verbose_str, "Kinetic parameters building started.", OutputStreamer::important);
      // Create possible kinetic parameters
      ConstrainsParser * constrains_parser = new ConstrainsParser(holder.getModel());
      constrains_parser->parseConstrains();
      holder.fillConstrains(std::move(constrains_parser));

      // Create implicit form of the kinetic parameters
      ParametrizationsHolder * parametrizations_holder = new ParametrizationsHolder;
      ParametrizationsBuilder parametrizations_builder(holder.getModel(), holder.getConstrains(), *parametrizations_holder);
      parametrizations_builder.buildParametrizations();
      holder.fillParametrizations(std::move(parametrizations_holder));

		output_streamer.output(verbose_str, "Parametrized Kripke structure building started.", OutputStreamer::important);
		// Create a simple Kripke structure without parametrization
		BasicStructure * basic_structure = new BasicStructure; // Kripke structure built from the network
		BasicStructureBuilder basic_structure_builder(holder.getModel(), *basic_structure);
		basic_structure_builder.buildStructure();
		holder.fillBasicStructure(basic_structure);

		// Create the PKS
		ParametrizedStructure * parametrized_structure = new ParametrizedStructure; // Kripke structure that has transitions labelled with functions
		ParametrizedStructureBuilder parametrized_structure_builder(holder.getBasicStructure(), holder.getParametrizations(), *parametrized_structure);
		parametrized_structure_builder.buildStructure();
		holder.fillParametrizedStructure(parametrized_structure);
   }
};


#endif // PARSYBONE_CONSTRUCTION_MANAGER_INCLUDED
