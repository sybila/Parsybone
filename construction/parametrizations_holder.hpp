/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_PARAMETRIZATIONS_HOLDER_INCLUDED
#define PARSYBONE_PARAMETRIZATIONS_HOLDER_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// This class stores feasible subcolors for each specie are stored with that specie ( a vector of all the possibilities for parametrization for this specie).
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ParametrizationsHolder {
	friend class ParametrizationsBuilder;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW TYPES AND DATA:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// Holds all the feasible subcolors for single Specie w.r.t. edge constrains
	struct SpecieColors {
		SpecieID ID; ///< Unique ID of the specie
		std::vector<std::vector<std::size_t> > subcolors; ///< Feasible subcolors of the specie
		std::size_t possible_count; ///< Total number of subcolors possible for the specie(even those unfesible)
		std::size_t acceptable_count; ///< Number of subcolors this state really has (equal to the subcolors.size())

		/// Add as new subcolor (to the end of the vector)
		void push_back (std::vector<std::size_t> subcolor) {
			subcolors.push_back(subcolor);
			acceptable_count = subcolors.size(); // Add a reference value
		}
	};

	/// Storage for all the vectors of subcolors for each specie
	std::vector<SpecieColors> colors;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OTHER FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ParametrizationsHolder(const ParametrizationsHolder & other); ///< Forbidden copy constructor.
	ParametrizationsHolder& operator=(const ParametrizationsHolder & other); ///< Forbidden assignment operator.

public:
	ParametrizationsHolder() {} ///< Default empty constructor, needed to create an empty object that will be filled

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTANT GETTERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @return	total number of species
	 */
	inline const std::size_t getSpecieNum() const {
		return colors.size();
	}

	/**
	 * @param ID	ID of the specie to get the number from
	 *
	 * @return	total number of subcolors this specie could have (all regulatory contexts' combinations)
	 */
	inline const std::size_t getAllColorsNum(const SpecieID ID) const {
		return colors[ID].possible_count;
	}

	/**
	 * @param ID	ID of the specie to get the number from
	 *
	 * @return	total number of subcolors this specie has (allowed regulatory contexts' combinations)
	 */
	inline const std::size_t getColorsNum(const SpecieID ID) const {
		return colors[ID].acceptable_count;
	}

	/**
	 * @return	size of the parameter space used in the computation
	 */
	const std::size_t getSpaceSize() const {
		std::size_t space_size = 1;
		forEach(colors, [&space_size](const SpecieColors & specie_cols) {
				  space_size *=	specie_cols.possible_count;
	});
	return space_size;
}

	/**
	 * @param ID	ID of the specie the requested subcolor belongs to
	 * @param color_num	ordinal number of the requested subcolor
	 *
	 * @return	requested subcolor from the vector of subcolors of given specie
	 */
	inline const std::vector<std::size_t> & getColor(const SpecieID ID, const std::size_t color_num) const {
		return colors[ID].subcolors[color_num];
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// REFORMING GETTERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * This function returns a vector containing target value for a given regulatory contexts for ALL the contexts allowed (in lexicographical order).
	 *
	 * @param ID	ID of the specie that is regulated
	 * @param regul_num	ordinal number of the regulatory context (in a lexicographical order)
	 *
	 * @return	vector with a target value for a given specie and regulatory context for each subcolor (parametrization of the single specie)
	 */
	const std::vector<std::size_t> getTargetVals(const SpecieID ID, const std::size_t regul_num) const {
		//Data to fill
		std::vector<std::size_t> all_target_vals;
		all_target_vals.reserve(colors[ID].subcolors.size());

		// Store values for given regulation
		for (auto color_it = colors[ID].subcolors.begin(); color_it != colors[ID].subcolors.end(); color_it++) {
			all_target_vals.push_back((*color_it)[regul_num]);
		}

		return all_target_vals;
	}

	/**
	 * This function creates a string containing a parametrization from its ordinal number.
	 * The string is in the form [specie_1_context_1, specie_2_context_2,...,specie_m_context_n]
	 *
	 * @param number	ordinal number of the parametrization to be converted
	 *
	 * @return string representation of given parametrisation
	 */
	const std::string createColorString(ColorNum number) const {
		// compute numbers of partial parametrizations for each component
		const std::vector<std::size_t> color_parts = std::move(getSpecieVals(number));

		std::string color_str = "[";
		// cycle through the species
		for (SpecieID ID = 0; ID < getSpecieNum(); ID++) {
			auto color = getColor(ID, color_parts[ID]);
			// fill partial parametrization of the specie
			for (auto it = color.begin(); it != color.end(); it++) {
				color_str += boost::lexical_cast<std::string, std::size_t>(*it);
				color_str += ",";
			}
		}
		// Change the last value
		*(color_str.end() - 1) = ']';

		return color_str;
	}

	/**
	 * This function takes ordinal number of a parametrization and computes ordinal number of partial parametrizations it is built from.
	 *
	 * @param number	ordinal number of the parametrization to be converted
	 *
	 * @return ordinal numbers of partial parametrizations in a vector indexed by IDs of the species
	 */
	const std::vector<std::size_t> getSpecieVals(ColorNum number) const {
		// Prepare storage vector
		std::vector<std::size_t> specie_vals(colors.size());
		auto reverse_val_it = specie_vals.rbegin();

		// Go through colors backwards
		ColorNum divisor = getSpaceSize();
		for (auto specie_it = colors.rbegin(); specie_it != colors.rend(); specie_it++, reverse_val_it++) {
			// lower divisor value
			divisor /= specie_it->possible_count;
			// pick a number for current specie
			*reverse_val_it = (number / divisor);
			// take the rest for next round
			number = number % divisor;
		}

	return specie_vals;
	}
};

#endif // PARSYBONE_PARAMETRIZATIONS_HOLDER_INCLUDED
