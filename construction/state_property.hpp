/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_STATE_PROPERTY_INCLUDED
#define PARSYBONE_STATE_PROPERTY_INCLUDED

#include "../auxiliary/data_types.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// This is just a very simple basis for a state of any graph.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename Transition>
struct StateProperty {
   /// unique ID of the state
   StateID ID;
   /// Graph or automaton transitions, basically it is an edge with a label
   std::vector<Transition> transitions;

   /**
    * Basic constructor fills in the ID.
    */
   StateProperty<Transition>(StateID _ID) : ID(_ID) { }
};

#endif // PARSYBONE_STATE_PROPERTY_INCLUDED
