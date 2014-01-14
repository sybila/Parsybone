/*
* Copyright (C) 2012-2013 - Adam Streck
* This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
* ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
* ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
* For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
*/

/* TODO: Convert in formulae that will be resolved by parser. */

#ifndef PARSYBONE_CONSTRAINT_SPACE_INCLUDED
#define PARSYBONE_CONSTRAINT_SPACE_INCLUDED

#include "../auxiliary/common_functions.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Gecode constraints for partial parametrization of a single specie.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ConstraintSpace : public Space {
	IntVarArray param_vals; ///< Parameter values.
	size_t target_max;

public:
	NO_COPY(ConstraintSpace)

		ConstraintSpace(const size_t param_no, const size_t _target_max)
		: param_vals(*this, param_no, 0, _target_max), target_max(_target_max) {
			branch(*this, param_vals, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		}

	ConstraintSpace(bool share, ConstraintSpace &other_space)
		: Space(share, other_space) {
		param_vals.update(*this, share, other_space.param_vals);
	}

	virtual Space *copy(bool share) {
		return new ConstraintSpace(share, *this);
	}

	/* initial constraining of the values to the predefined ones */
	void remove_targets(const Levels &targets, const size_t param_no) {
		// If a value is not among the possible targets, disable it.
		for (const size_t val : range(target_max))
		if (find(targets.begin(), targets.end(), val) == targets.end())
			rel(*this, param_vals[param_no], IRT_NQ, val);
	}

	/* Create an expression based on the conditions obtained from the edge label */
	BoolExpr resolveLabel(const Model::Regulation & regul, BoolExpr & act, BoolExpr & inh) {
		BoolExpr result = expr(*this, param_vals[0] != param_vals[0]);

		if (regul.edge_const_func.none)
			result = result || (!act && !inh);
		if (regul.edge_const_func.act)
			result = result || (act && !inh);
		if (regul.edge_const_func.inh)
			result = result || (!act && inh);
		if (regul.edge_const_func.both)
			result = result || (act && inh);

		return result;
	}

	/* For each regulation create a constraint corresponding to its label */
	void add_edge_cons(const vector<Model::Regulation> & reguls, const vector<Model::Parameter> & params) {
		if (reguls.empty())
			return;
		for (const Model::Regulation & regul : reguls) {
			// The first statement in the disjunction is set to false the following way (may be done better)
			BoolExpr some_act = expr(*this, param_vals[0] != param_vals[0]);
			BoolExpr some_inh = some_act;

			for (const size_t param_no : scope(params)) {
				if (ParametrizationsHelper::containsRegulation(params[param_no], regul)) {
					for (const size_t compare_no : scope(params)) {
						if (ParametrizationsHelper::isSubordinate(reguls, params[param_no], params[compare_no], regul.source)) {
							some_act = some_act || expr(*this, param_vals[param_no] > param_vals[compare_no]);
							some_inh = some_inh || expr(*this, param_vals[param_no] < param_vals[compare_no]);
						}
					}
				}
			}
			rel(*this, resolveLabel(regul, some_act, some_inh));
		}
	}

	// print solution
	void print(void) const {
		std::cout << param_vals << std::endl;
	}

	Levels getSolution() const {
		Levels result(param_vals.size(), 0u);

		for (const size_t i : range(param_vals.size()))
			result[i] = param_vals[i].val();

		return result;
	}
};

#endif
