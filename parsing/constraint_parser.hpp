/*
* Copyright (C) 2012-2013 - Adam Streck
* This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
* ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
* ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
* For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
*/

#ifndef PARSYBONE_CONSTRAINT_PARSER_INCLUDED
#define PARSYBONE_CONSTRAINT_PARSER_INCLUDED

#include "../auxiliary/common_functions.hpp"

enum Comparison { comp_lt, comp_le, comp_e, comp_ne, comp_gt, comp_ge };

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ConstraintParser : public Space {
	vector<string> names;
	IntVarArray allowed_vals;
	BoolExpr tt_expr, ff_expr;
	string formula_;

	/* Keep only the values that are actually the values of the species. */
	void bound_values(const Levels & maxes){
		for (const ActLevel i : scope(maxes)) 
			rel(*this, allowed_vals[i] < (maxes[i] + 1));
	}

	bool getNumber(string atom_part, int & value) {
		try {
			value = lexical_cast<int>(atom_part);
		}
		catch (...) {
			return false;
		}
		return true;
	}

	/* Find the number of the integer variable that corresponds to the given name of the specie. */
	size_t findName(string specie_name) {
		for (const size_t name_no : scope(names))
		if (specie_name.compare(names[name_no]) == 0)
			return name_no;
		throw runtime_error("Unrecognized variable name \"" + specie_name + "\" in the fomula \"" + formula_ + "\".");
	}

	/* Convert the atomic expression to the relevant constraint */
	BoolExpr convertAtom(const string & atom) {
		BoolExpr result;
		string left_side, right_side;
		int left_val, righ_val;

		if (atom.compare("tt") == 0)
			return tt_expr;
		else if (atom.compare("ff") == 0)
			return ff_expr;
		else if (atom.find("<=") != atom.npos) {
			left_side = atom.substr(0, atom.find("<="));
			right_side = atom.substr(atom.find("<=") + 2);
			if (getNumber(left_side, left_val))
				result = expr(*this, allowed_vals[findName(right_side)] >= left_val);
			if (getNumber(right_side, righ_val))
				result = expr(*this, allowed_vals[findName(left_side)] <= righ_val);
			else
				result = expr(*this, allowed_vals[findName(left_side)] <= allowed_vals[findName(right_side)]);
		}
		else if (atom.find(">=") != atom.npos) {
			left_side = atom.substr(0, atom.find(">="));
			right_side = atom.substr(atom.find(">=") + 2);
			if (getNumber(left_side, left_val))
				result = expr(*this, allowed_vals[findName(right_side)] <= left_val);
			if (getNumber(right_side, righ_val))
				result = expr(*this, allowed_vals[findName(left_side)] >= righ_val);
			else
				result = expr(*this, allowed_vals[findName(left_side)] >= allowed_vals[findName(right_side)]);
		}
		else if (atom.find("!=") != atom.npos) {
			left_side = atom.substr(0, atom.find("!="));
			right_side = atom.substr(atom.find("!=") + 2);
			if (getNumber(left_side, left_val))
				result = expr(*this, allowed_vals[findName(right_side)] != left_val);
			if (getNumber(right_side, righ_val))
				result = expr(*this, allowed_vals[findName(left_side)] != righ_val);
			else
				result = expr(*this, allowed_vals[findName(left_side)] != allowed_vals[findName(right_side)]);
		}
		else if (atom.find("=") != atom.npos) {
			left_side = atom.substr(0, atom.find("="));
			right_side = atom.substr(atom.find("=") + 1);
			if (getNumber(left_side, left_val))
				result = expr(*this, allowed_vals[findName(right_side)] == left_val);
			if (getNumber(right_side, righ_val))
				result = expr(*this, allowed_vals[findName(left_side)] == righ_val);
			else
				result = expr(*this, allowed_vals[findName(left_side)] == allowed_vals[findName(right_side)]);
		}
		else if (atom.find("<") != atom.npos) {
			left_side = atom.substr(0, atom.find("<"));
			right_side = atom.substr(atom.find("<") + 1);
			if (getNumber(left_side, left_val))
				result = expr(*this, allowed_vals[findName(right_side)] > left_val);
			if (getNumber(right_side, righ_val))
				result = expr(*this, allowed_vals[findName(left_side)] < righ_val);
			else
				result = expr(*this, allowed_vals[findName(left_side)] < allowed_vals[findName(right_side)]);
		}
		else if (atom.find(">") != atom.npos) {
			left_side = atom.substr(0, atom.find(">"));
			right_side = atom.substr(atom.find(">") + 1);
			if (getNumber(left_side, left_val))
				result = expr(*this, allowed_vals[findName(right_side)] < left_val);
			if (getNumber(right_side, righ_val))
				result = expr(*this, allowed_vals[findName(left_side)] > righ_val);
			else
				result = expr(*this, allowed_vals[findName(left_side)] > allowed_vals[findName(right_side)]);
		}
		else {
			result = expr(*this, allowed_vals[findName(atom)] == 1);
		}

		return result;
	}

	/* Split the formula by the specified operator (either | or &). This is an intelligent split - only symbols that are outside parenthesis are considered. */
	vector<string> splitByOperator(const bool is_or, const string & formula) {
		vector<string> result;
		
		int parity = 0;
		size_t last_pos = 0;
		for (const size_t pos : scope(formula)) {
			if (formula[pos] == '(')
				parity++;
			else if (formula[pos] == ')')
				parity--;
			if (parity < 0)
				throw runtime_error("There is a right bracket without matching left bracket in the part \"" + formula + "\" of the fomula \"" + formula_ + "\".");

			if (parity == 0 && ((formula[pos] == '|' && is_or) || (formula[pos] == '&' && !is_or))) {
				result.push_back(formula.substr(last_pos, pos - last_pos));
				last_pos = pos + 1;
			}
		}

		result.push_back(formula.substr(last_pos));

		if (parity > 0)
			throw runtime_error("There is a left bracket without matching right bracket in the part \"" + formula + "\" of the fomula \"" + formula_ + "\".");

		return result;
	}

	/* Trim the current formula and if it is enclosed in parenthesis, remove them. */
	void remove_parenthesis(string & formula) {
		// If there's less than two characters there can't be no parenthesis.
		if (formula.size() < 2)
			return;
		// Parenthesis must be enclosing
		if (*formula.begin() != '(' || *formula.rbegin() != ')')
			return;
		// Only the last parenthesis must be matching
		size_t parity = 1;
		for (const size_t pos : range(1u, formula.size() - 1)) {
			if (formula[pos] == '(')
				parity++;
			else if (formula[pos] == ')')
				parity--;
			if (parity == 0)
				return;
		}
		
		formula = formula.substr(1, formula.size() - 2);
	}

	/*  */
	BoolExpr resolveFormula(string formula) {
		BoolExpr result;

		// Remove outer parenthesis until you reach fixpoint.
		string old_formula;
		do {
			old_formula = formula;
			remove_parenthesis(formula);
		} while (old_formula.compare(formula) != 0);

		// Make divisions of the current expression by operators
		vector<string> div_by_or = splitByOperator(true, formula);
		vector<string> div_by_and = splitByOperator(false, formula);

		// Based on the divisions decide how to deal with the formula
		if (div_by_or.size() == 1 && div_by_and.size() == 1) {
			if (formula[0] == '!')
				return expr(*this, !resolveFormula(formula.substr(1)));
			else
				return convertAtom(formula);
		}
		else if (div_by_or.size() > 1 && div_by_and.size() == 1) {
			result = expr(*this, resolveFormula(div_by_or[0]) || resolveFormula(div_by_or[1]));
			for (const size_t expr_no : range(2u, div_by_or.size())) 
				result = expr(*this, result || resolveFormula(div_by_or[expr_no]));
			return result;
		}
		else if (div_by_or.size() == 1 && div_by_and.size() > 1) {
			result = expr(*this, resolveFormula(div_by_and[0]) && resolveFormula(div_by_and[1]));
			for (const size_t expr_no : range(2u, div_by_and.size()))
				result = expr(*this, result && resolveFormula(div_by_and[expr_no]));
			return result;
		}
		else {
			throw runtime_error("Error when parsing the part \"" + formula + "\" of the fomula \"" + formula_ + 
				"\". Operators | and & are mixed, add parenthesis.");
		}
	}

public:
	NO_COPY(ConstraintParser)

		ConstraintParser(const vector<string> & _names, const Levels & maxes)
		: allowed_vals(*this, maxes.size(), 0, *max_element(maxes.begin(), maxes.end())), names(_names) {
			branch(*this, allowed_vals, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
			bound_values(maxes);
			tt_expr = expr(*this, allowed_vals[0] == allowed_vals[0]);
			ff_expr = expr(*this, allowed_vals[0] != allowed_vals[0]);
		}

	ConstraintParser(bool share, ConstraintParser &other_space)
		: Space(share, other_space) {
		allowed_vals.update(*this, share, other_space.allowed_vals);
	}

	virtual Space *copy(bool share) {
		return new ConstraintParser(share, *this);
	}

	// print solution
	void print(void) const {
		std::cout << allowed_vals << std::endl;
	}

	/* Obtain a vector with the solution */
	Levels getSolution() const {
		Levels result(allowed_vals.size(), 0u);

		for (const size_t i : range(allowed_vals.size()))
			result[i] = allowed_vals[i].val();

		return result;
	}

	/* Take a logical formula and make it into a constraint that gets propagated. */
	void applyFormula(string formula) {
		formula_ = formula;
		formula.erase(remove_if(formula.begin(), formula.end(), isspace), formula.end());
		BoolExpr expr = resolveFormula(formula);
		rel(*this, expr);
	}
};

#endif
