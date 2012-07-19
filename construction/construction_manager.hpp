/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_CONSTRUCTION_MANAGER_INCLUDED
#define PARSYBONE_CONSTRUCTION_MANAGER_INCLUDED

#include "parametrizations_builder.hpp".hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ConstructionManager overviews the whole process of construction of structures from information contained within a model file.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ConstructionManager {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   const Model & model; ///< Model object that will contain all the parsed information from the .dbm file

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTION FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   void constructBasicStructure() {

   }

public:
   /**
    * Constructor
    *
    * @param _model  model object to read the data from
    */
   ConstructionManager(const Model & _model) : model(_model) {

   }

   void construct() {
      ParametrizationsHolder parametrizations_holder;
      ConstrainsParser constrains_parser(model);

      output_streamer.output(verbose_str, "Functions building started.", OutputStreamer::important);

      constrains_parser.parseConstrains();

      ParametrizationsBuilder parametrizations_builder(model, constrains_parser, parametrizations_holder);
      parametrizations_builder.buildFunctions();


   }
};


#endif // PARSYBONE_CONSTRUCTION_MANAGER_INCLUDED
