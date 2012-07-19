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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Class stores and provides all the objects that are built during construction phase.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ConstructionHolder {
	Model model;

	ConstructionHolder(const ConstructionHolder & other); ///< Forbidden copy constructor.
	ConstructionHolder& operator=(const ConstructionHolder & other); ///< Forbidden assignment operator.

public:
	ConstructionHolder() {} ///< Default (empty) constructor

	void setModel(Model && _model) {
		model = std::move(_model);
	}

	const Model & getModel() const {
		return model;
	}
};

#endif // PARSYBONE_CONSTRUCTION_HOLDER_INCLUDED
