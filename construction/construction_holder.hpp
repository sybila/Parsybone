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
#include "constrains_parser.hpp"
#include "parametrizations_holder.hpp"
#include "basic_structure_builder.hpp"
#include "parametrized_structure_builder.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Class stores and provides all the objects that are built during construction phase.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ConstructionHolder {
	friend class ConstructionManager;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::unique_ptr<ConstrainsParser> constrains;
	std::unique_ptr<Model> model;
	std::unique_ptr<ParametrizationsHolder> parametrizations;
	std::unique_ptr<BasicStructure> basic;
	std::unique_ptr<ParametrizedStructure> structure;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FILLERS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

public:
	void fillModel(Model * _model) {
		model.reset(_model);
	}

private:
	void fillBasicStructure(BasicStructure * _basic) {
		basic.reset(_basic);
	}

	void fillConstrains(ConstrainsParser * _constrains) {
		constrains.reset(_constrains);
	}

	void fillParametrizations(ParametrizationsHolder * _parametrizations) {
		parametrizations.reset(_parametrizations);
	}

	void fillParametrizedStructure(ParametrizedStructure * _structure) {
		structure.reset(_structure);
	}

	ConstructionHolder(const ConstructionHolder & other); ///< Forbidden copy constructor.
	ConstructionHolder& operator=(const ConstructionHolder & other); ///< Forbidden assignment operator.

public:
	ConstructionHolder() {} ///< Default (empty) constructor

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const BasicStructure & getBasicStructure() const {
		return *basic.get();
	}

	const ConstrainsParser & getConstrains() const {
		return *constrains.get();
	}

	const Model & getModel() const {
		return *model.get();
	}

	const ParametrizationsHolder & getParametrizations() const {
		return *parametrizations.get();
	}

	const ParametrizedStructure & getParametrizedStructure() const {
		return *structure.get();
	}
};

#endif // PARSYBONE_CONSTRUCTION_HOLDER_INCLUDED
