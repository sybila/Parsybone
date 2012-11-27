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
/// @file This file holds specifications of data types (typedefs and 3rd party classes) used by multiple classes throughout the computation.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <regex>
#include <set>
#include <string>
#include <stdexcept>

#include <boost/assert.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INPUT/OUTPUT RELATED DATA
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const float program_version = 1.0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INPUT/OUTPUT RELATED TYPES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// What stream to use in output functions.
enum StreamType {error_str, results_str, stats_str, verbose_str};

/// Symbol that separates values of a single parametrizations on output.
const string separator = ":";

/// Definition of naming abbreviations for edge labels:
namespace Label {
   const string Activating = "Activating";
   const string ActivatingOnly = "ActivatingOnly";
   const string Inhibiting = "Inhibiting";
   const string InhibitingOnly = "InhibitingOnly";
   const string NotActivating = "NotActivating";
   const string NotInhibiting = "NotInhibiting";
   const string Observable = "Observable";
   const string NotObservable = "NotObservable";
   const string Free = "Free";
}

const string MODEL_SUFFIX = ".dbm";
const string DATABASE_SUFFIX = ".sqlite";
const string OUTPUT_SUFFIX = ".out";

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA RELATED TYPES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// If a regulation is missing, what kind of value it should get?
enum UnspecifiedParameters {error_reg, basal_reg, param_reg};

/// To store the way a specie value has changed.
enum Direction {up_dir, stay_dir, down_dir};

/// ID of a graph/automaton state.
typedef size_t StateID; 

/// ID of a specie.
typedef size_t SpecieID; 

/// Vector of activation levels of species used for labelling of states of KS.
typedef vector<size_t> Levels;

/// Transitional values in a state of BA.
typedef vector<Levels > AllowedValues;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTATION RELATED TYPES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Mask of parameters - each bit represents single combination of target values for each function.
typedef unsigned int Paramset;

/// Index of the color - may be a big number.
typedef long long ColorNum;

/// Index of the color - may be a big number as well.
typedef unsigned long long RoundNum;

/// IDs of predecessors of a state.
typedef vector<StateID> Neighbours;

/// State number and its coloring.
typedef pair<StateID, Paramset> Coloring;

/// Storing range of values that will be used in the form [first, last) i.e. first index is used, last is not.
typedef pair<ColorNum, ColorNum> Range;

#endif // PARSYBONE_DATA_TYPES_INCLUDED
