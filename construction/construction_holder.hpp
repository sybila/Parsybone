/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_CONSTRUCTION_HOLDER_INCLUDED
#define PARSYBONE_CONSTRUCTION_HOLDER_INCLUDED

#include "../parsing/model.hpp"
#include "automaton_builder.hpp"
#include "basic_structure_builder.hpp"
#include "parametrizations_builder.hpp"
#include "labeling_builder.hpp"
#include "parametrized_structure_builder.hpp"
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
	friend class ConstructionManager;

	std::unique_ptr<AutomatonStructure> automaton;
	std::unique_ptr<BasicStructure> basic;
	std::unique_ptr<ParametrizationsHolder> parametrizations;
	std::unique_ptr<Model> model;
	std::unique_ptr<LabelingHolder> labeling;
	std::unique_ptr<ParametrizedStructure> structure;
	std::unique_ptr<ProductStructure> product;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FILLERS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	void fillModel(Model * _model) {
		model.reset(_model);
	}

private:
	void fillAutomaton(AutomatonStructure * _automaton) {
		automaton.reset(_automaton);
	}

	void fillBasicStructure(BasicStructure * _basic) {
		basic.reset(_basic);
	}

	void fillConstrains(ParametrizationsHolder * _parametrizations) {
		parametrizations.reset(_parametrizations);
	}

	void fillLabeling(LabelingHolder * _labeling) {
		labeling.reset(_labeling);
	}

	void fillParametrizedStructure(ParametrizedStructure * _structure) {
		structure.reset(_structure);
	}

	void fillProduct(ProductStructure * _product) {
		product.reset(_product);
	}

	ConstructionHolder(const ConstructionHolder & other); ///< Forbidden copy constructor.
	ConstructionHolder& operator=(const ConstructionHolder & other); ///< Forbidden assignment operator.

public:
   ConstructionHolder() {} ///< Default (empty) constructor.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const AutomatonStructure & getAutomatonStructure() const {
		return *automaton.get();
	}

	const BasicStructure & getBasicStructure() const {
		return *basic.get();
	}

	const ParametrizationsHolder & getParametrizations() const {
		return *parametrizations.get();
	}

	const Model & getModel() const {
		return *model.get();
	}

	const LabelingHolder & getLabeling() const {
		return *labeling.get();
	}

	const ParametrizedStructure & getParametrizedStructure() const {
		return *structure.get();
	}

	const ProductStructure & getProduct() const {
		return *product.get();
	}
};

#endif // PARSYBONE_CONSTRUCTION_HOLDER_INCLUDED
