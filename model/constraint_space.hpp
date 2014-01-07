/*
* Copyright (C) 2012-2013 - Adam Streck
* This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean
* Networks) verification tool.
* ParSyBoNe is a free software: you can redistribute it and/or modify it under
* the terms of the GNU General Public License version 3.
* ParSyBoNe is released without any warrany. See the GNU General Public License
* for more details. <http://www.gnu.org/licenses/>.
* For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and
* <http://sybila.fi.muni.cz/>.
*/

#ifndef PARSYBONE_CONSTRAINT_SPACE_INCLUDED
#define PARSYBONE_CONSTRAINT_SPACE_INCLUDED

#include "../auxiliary/common_functions.hpp"

class ConstraintSpace : public Space {
  IntVarArray arr;
  size_t target_max;

public:
  NO_COPY(ConstraintSpace)
  ConstraintSpace(const size_t param_no, const size_t _target_max)
      : arr(*this, param_no, 0, _target_max), target_max(_target_max) {
    branch(*this, arr, INT_VAL_MIN());
  }

  ConstraintSpace(bool share, ConstraintSpace &other_space)
      : Space(share, other_space) {
    arr.update(*this, share, other_space.arr);
  }

  virtual Space *copy(bool share) { return new ConstraintSpace(share, *this); }

  /*
   *
   */
  void remove_targets(const Levels &targets, const size_t param_no) {
    // If a value is not among the possible targets, disable it.
    for (const size_t val : range(target_max))
      if (find(targets.begin(), targets.end(), val) == targets.end())
        rel(*this, arr[param_no], IRT_NQ, val);
  }

  // print solution
  void print(void) const { std::cout << arr << std::endl; }
};

#endif
