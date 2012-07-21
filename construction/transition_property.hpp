/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_TRANSITION_PROPERTY_INCLUDED
#define PARSYBONE_TRANSITION_PROPERTY_INCLUDED

#include "../auxiliary/data_types.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// This is just a very simple basis for a transition in a graph
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct TransitionProperty {
   /// unique ID of the state
   StateID target_ID;

   /**
    * Basic constructor fills in the ID.
    */
   TransitionProperty(StateID _target_ID) : target_ID(_target_ID) { }
};

#endif // PARSYBONE_TRANSITION_PROPERTY_INCLUDED
