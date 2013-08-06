/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_CONSTRUCTION_HOLDER_INCLUDED
#define PARSYBONE_CONSTRUCTION_HOLDER_INCLUDED

#include "automaton_builder.hpp"
#include "basic_structure_builder.hpp"
#include "product_builder.hpp"
#include "unparametrized_structure_builder.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Stores pointers to all data objects created for the purpose of the synthesis.
///
/// Class stores and provides all the objects that are built during construction phase.
/// There are two methods employed:
///	-# set* this method obtains a reference for a dynamic object and assigns it to its unique_ptr,
///   -# get* this method returns a constant reference to a requested object.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ConstructionHolder {
   unique_ptr<AutomatonStructure> automaton;
   unique_ptr<BasicStructure> basic;
   unique_ptr<UnparametrizedStructure> structure;
   unique_ptr<ProductStructure> product;

public:
   void setAutomaton(unique_ptr<AutomatonStructure> _automaton) {
      automaton = move(_automaton);
   }

   void setBasicStructure(unique_ptr<BasicStructure> _basic) {
      basic = move(_basic);
   }

   void setUnparametrizedStructure(unique_ptr<UnparametrizedStructure> _structure) {
      structure = move(_structure);
   }

   void setProduct(unique_ptr<ProductStructure> _product) {
      product = move(_product);
   }

   const AutomatonStructure & getAutomaton() const {
      return *automaton.get();
   }

   const BasicStructure & getBasicStructure() const {
      return *basic.get();
   }


   const UnparametrizedStructure & getUnparametrizedStructure() const {
      return *structure.get();
   }

   const ProductStructure & getProduct() const {
      return *product.get();
   }
};

#endif // PARSYBONE_CONSTRUCTION_HOLDER_INCLUDED
