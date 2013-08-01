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
#include "parametrizations_builder.hpp"
#include "labeling_builder.hpp"
#include "unparametrized_structure_builder.hpp"
#include "product_builder.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Stores pointers to all data objects created for the purpose of the synthesis.
///
/// Class stores and provides all the objects that are built during construction phase.
/// There are two methods employed:
///	-# fill* this method obtains a reference for a dynamic object and assigns it to its unique_ptr,
///   -# get* this method returns a constant reference to a requested object.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ConstructionHolder {
    AutomatonStructure automaton;
    unique_ptr<BasicStructure> basic;
    PropertyAutomaton property;
    Model model;
    unique_ptr<UnparametrizedStructure> structure;
    unique_ptr<ProductStructure> product;

public:
    void fillModel(Model _model) {
        model = move(_model);
    }

    void fillProperty(PropertyAutomaton _property) {
        property = move(_property);
    }

    void fillAutomaton(AutomatonStructure _automaton) {
        automaton = move(_automaton);
    }

    void fillBasicStructure(BasicStructure * _basic) {
        basic.reset(_basic);
    }

    void fillUnparametrizedStructure(UnparametrizedStructure * _structure) {
        structure.reset(_structure);
    }

    void fillProduct(ProductStructure * _product) {
        product.reset(_product);
    }


    const AutomatonStructure & getAutomaton() const {
        return automaton;
    }

    const BasicStructure & getBasicStructure() const {
        return *basic.get();
    }

    const Model & getModel() const {
        return model;
    }

    const UnparametrizedStructure & getUnparametrizedStructure() const {
        return *structure.get();
    }

    const ProductStructure & getProduct() const {
        return *product.get();
    }

};

#endif // PARSYBONE_CONSTRUCTION_HOLDER_INCLUDED
