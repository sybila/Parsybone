/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_PARAMETRIZATIONS_HOLDER_INCLUDED
#define PARSYBONE_PARAMETRIZATIONS_HOLDER_INCLUDED

#include "../auxiliary/data_types.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Stores partial parametrizations (functions of each component).
///
/// This class stores feasible subcolors for each specie are stored with that specie ( a vector of all the possibilities for parametrization for this specie).
/// @attention Subcolor means partial parametrizatrization ~ full parametrization of a single specie.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ParametrizationsHolder {
	friend class ParametrizationsBuilder;

	/// Holds all the feasible subcolors for single Specie w.r.t. edge constrains
	struct SpecieColors {
      SpecieID ID; ///< Unique ID of the specie.
      vector<Levels> subcolors; ///< Feasible subcolors of the specie.
      ParamNum possible_count; ///< Total number of subcolors possible for the specie(even those unfesible).
      ParamNum acceptable_count; ///< Number of subcolors this state really has (equal to the subcolors.size()).

      /// Add as new subcolor (to the end of the vector).
		void push_back (vector<size_t> subcolor) {
			subcolors.push_back(subcolor);
			acceptable_count = subcolors.size(); // Add a reference value
		}
	};

    /// Storage for all the vectors of subcolors for each specie.
	vector<SpecieColors> colors;

public:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @return	total number of species
	 */
	inline size_t getSpecieNum() const {
		return colors.size();
	}

	/**
	 * @param ID	ID of the specie to get the number from
	 *
	 * @return	total number of subcolors this specie could have (all regulatory contexts' combinations)
	 */
	inline size_t getAllColorsNum(const SpecieID ID) const {
		return colors[ID].possible_count;
	}

	/**
	 * @param ID	ID of the specie to get the number from
	 *
	 * @return	total number of subcolors this specie has (allowed regulatory contexts' combinations)
	 */
	inline size_t getColorsNum(const SpecieID ID) const {
		return colors[ID].acceptable_count;
	}

	/**
	 * @return	size of the parametrization space used in the computation
	 */
	ParamNum getSpaceSize() const {
		ParamNum space_size = 1;
		for (const auto & color:colors) {
			space_size *= color.acceptable_count;
		}
		return space_size;
	}

	/**
	 * @param ID	ID of the specie the requested subcolor belongs to
	 * @param color_num	ordinal number of the requested subcolor
	 *
	 * @return	requested subcolor from the vector of subcolors of given specie
	 */
	inline const vector<size_t> & getColor(const SpecieID ID, const ParamNum color_num) const {
		return colors[ID].subcolors[static_cast<size_t>(color_num)];
	}

    ParametrizationsHolder(ParametrizationsHolder&& ) = default;
    ParametrizationsHolder& operator=(ParametrizationsHolder&& ) = default;
    ParametrizationsHolder(const ParametrizationsHolder& ) = delete;
    ParametrizationsHolder& operator=(const ParametrizationsHolder& ) = delete;
    ParametrizationsHolder() = default;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// REFORMING GETTERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * This function returns a vector containing target value for a given regulatory contexts for ALL the contexts allowed (in lexicographical order).
	 *
	 * @param ID	ID of the specie that is regulated
	 * @param param_num	ordinal number of the kinetic parameter (in a lexicographical order)
	 *
	 * @return	vector with a target value for a given specie and regulatory context for each subcolor (parametrization of the single specie)
	 */
	const vector<size_t> getTargetVals(const SpecieID ID, const size_t param_num) const {
		//Data to fill
		vector<size_t> all_target_vals;
		all_target_vals.reserve(colors[ID].subcolors.size());

		// Store values for given regulation
		for (auto color_it = colors[ID].subcolors.begin(); color_it != colors[ID].subcolors.end(); color_it++) {
			all_target_vals.push_back((*color_it)[param_num]);
		}

		return all_target_vals;
	}

	/**
	 * This function creates a string containing a parametrization from its ordinal number.
    * The string is in the form [specie_1_context_1, specie_2_context_2,...,specie_m_context_n].
	 *
	 * @param number	ordinal number of the parametrization to be converted
	 *
	 * @return string representation of given parametrisation
	 */
	const string createColorString(ParamNum number) const {
		// compute numbers of partial parametrizations for each component
		const vector<ParamNum> color_parts = move(getSpecieVals(number));

		string color_str = "(";
		// cycle through the species
		for (SpecieID ID = 0; ID < getSpecieNum(); ID++) {
			auto color = getColor(ID, color_parts[ID]);
			// fill partial parametrization of the specie
			for (auto it = color.begin(); it != color.end(); it++) {
				color_str += lexical_cast<string, size_t>(*it);
				color_str += ",";
			}
		}
		// Change the last value
		*(color_str.end() - 1) = ')';

		return color_str;
	}

	/**
	 * This function takes ordinal number of a parametrization and computes ordinal number of partial parametrizations it is built from.
	 *
	 * @param number	ordinal number of the parametrization to be converted
	 *
	 * @return ordinal numbers of partial parametrizations in a vector indexed by IDs of the species
	 */
	const vector<ParamNum> getSpecieVals(ParamNum number) const {
		// Prepare storage vector
		vector<ParamNum> specie_vals(colors.size());
		auto reverse_val_it = specie_vals.rbegin();

		// Go through colors backwards
		ParamNum divisor = getSpaceSize();
		for (auto specie_it = colors.rbegin(); specie_it != colors.rend(); specie_it++, reverse_val_it++) {
			// lower divisor value
			divisor /= specie_it->acceptable_count;
			// pick a number for current specie
			*reverse_val_it = (number / divisor);
			// take the rest for next round
			number = number % divisor;
		}

      return specie_vals;
	}
};

#endif // PARSYBONE_PARAMETRIZATIONS_HOLDER_INCLUDED
