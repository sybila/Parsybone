/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_DATA_TYPES_INCLUDED
#define PARSYBONE_DATA_TYPES_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This file holds specifications of data types (typedefs and 3rd party classes) used by multiple classes throughout the computation.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <string>
#include <stdexcept>

#include <boost/assert.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INPUT/OUTPUT RELATED TYPES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// What stream to use in all-streams functions
enum StreamType {input_str, error_str, results_str, stats_str, verbose_str};

// Named pointer - just in case of necessity to use a file
std::istream * const input_stream = &std::cin;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA RELATED TYPES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// If a regulation is missing, what kind of value it should get?
enum UnspecifiedRegulations {error_reg, basal_reg, param_reg};

// To store the way a specie value has changed
enum Direction {up_dir, stay_dir, down_dir};

// ID of a graph/automaton state
typedef std::size_t StateID; 

// ID of a specie
typedef std::size_t SpecieID; 

// vector of activation levels of species used for labelling of states of KS
typedef std::vector<std::size_t> Levels;

// Transitible values in a state of BA
typedef std::vector<std::set<std::size_t> > AllowedValues;

// Label of the interaction
enum EdgeConstrain {pos_cons, neg_cons, none_cons};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTATION RELATED TYPES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// mask of parameters - each bit represents single combination of target values for each function
typedef unsigned int Parameters;

// Index of the color - may be a big number
typedef long long ColorNum;

// IDs of predecessors of a state
typedef std::vector<StateID> Neighbours;

// State number and its coloring
typedef std::pair<StateID, Parameters> Coloring;

// Storing range of values that will be used in the form [first, last) i.e. first index is used, last is not
typedef std::pair<ColorNum, ColorNum> Range;

// How to deal with witnesses
enum WitnessUse {none_wit, short_wit, all_wit};

#endif
