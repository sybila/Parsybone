/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */


#ifndef PARSYBONE_PARAMSET_HELPER_INCLUDED
#define PARSYBONE_PARAMSET_HELPER_INCLUDED

#include "PunyHeaders/common_functions.hpp"

#include "../auxiliary/data_types.hpp"
#include "../auxiliary/output_streamer.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Class with mainly static functions for paramset (integer) handling.
///
/// Here are methods that provide help when working with subsets of parametrization space.
/// @attention Parametrizations in an Paramset are ordered in an ascending order.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace ParamsetHelper {
   const size_t subset_size = sizeof(Paramset) * 8; ///< Size in bits of a single subset of parametrization space.
   const Paramset none = static_cast<Paramset>(0u); ///< Parametrization set with all the bits set to 0.
   const Paramset one = static_cast<Paramset>(1u); ///< Parametrization with the right bit set to 1.
   const Paramset all = ~none; ///< Parametrization set with all the bits set to 1.

   /**
    * @brief testParametrizations   conduct a test on correctness of the parametrization representations.
    */
   void testParametrizations() {
      for (size_t i = 0; i < subset_size; i++)
         if ((none >> i) % 2)
            throw runtime_error("Parametrization description malformed. Parsybone cannot run on this platform.");
      for (size_t i = 1; i < subset_size; i++)
         if ((one >> i) % 2)
            throw runtime_error("Parametrization description malformed. Parsybone cannot run on this platform.");
      if (!(one % 2))
         throw runtime_error("Parametrization description malformed. Parsybone cannot run on this platform.");
      for (size_t i = 0; i < subset_size; i++)
         if (!((all >> i) % 2))
            throw runtime_error("Parametrization description malformed. Parsybone cannot run on this platform.");
   }

   /**
    * @return  the number of parametrizations in a single round
    */
   inline size_t getSetSize() {
      return subset_size;
   }

   /**
    * @return  paramset with everything set to 0
    */
   inline Paramset getNone() {
      return none;
   }

   /**
    * @return  paramset with everything set to 1
    */
   inline Paramset getAll() {
      return all;
   }

   /**
    * @brief getLeftOne get paramset that holds value of the binary form 10...0 (leftmost parametrization)
    * @param size number of bits the parametrization has (delimits how much to the left the one will be moved).
    */
   Paramset getLeftOne(ParamNum size = subset_size)  {
      return (one << (size - 1));
   }

   /**
    * Computer a vector of masks of single parametrizations - i.e. 10010 would give {10000,00010}.
    * @param[in] paramset	paramset to disassamble
    * @return vector containing a paramset with a single parametrization for each parametrization in input paramset
    */
   vector<Paramset> getSingleMasks(Paramset paramset)  {
      vector<Paramset> masks;
      Paramset mask = getLeftOne();
      for (size_t counter = 0; counter < getSetSize(); counter++) {
         if (mask & paramset)
            masks.push_back(mask);
         mask >>= 1;
      }
      return masks;
   }

   /**
    * Return a paramset with on bits corresponding to requested numbers - i.e. for {1,3} we would get 1010...0.
    * @param[in] vector of number in range [1,|paramset|]
    * @return  Paramset mask
    */
   Paramset getMaskFromNums(const vector<size_t> numbers)  {
      Paramset full_mask = 0;
      for (auto num = numbers.begin(); num != numbers.end(); num++) {
         if (*num > getSetSize())
            throw runtime_error("Mask asked for number bigger that is the size of Paramset mask.");
         full_mask |= 1 << (getSetSize() - (*num + 1));
      }
      return full_mask;
   }

   /**
    * Flips every bit.
    * @param[in] paramset	paramset to flip bits in
    * @return  copy of input with swapped bits.
    */
   inline Paramset flip(const Paramset paramset)   {
      return ~paramset;
   }

   /**
    * Swaps paramset within a variable - last become first etc.
    * @param[in] paramset	paramset to swap bits in
    * @return  copy of input with descending order of paramset
    */
   inline Paramset swap(Paramset paramset)  {
      Paramset new_params = 0;
      for (size_t param_num = 0; param_num < getSetSize(); param_num++) {
         new_params <<= 1;
         if (paramset % 2)
            new_params |= 1;
         paramset >>= 1;
      }
      return new_params;
   }

   /**
    * Swaps paramset within a variable - last become first etc.
    * @param[in] paramset	paramset to swap bits in
    * @param[in] shift	if there are not all the paramset used, shift back after swapping
    * @return  copy of input with descending order of paramset
    */
   inline Paramset swap(Paramset paramset, const size_t shift)  {
      paramset = swap(paramset);
      paramset >>= shift;
      return paramset;
   }
   /**
    * Count bits.
    * @return  number of parametrizations in the set
    */
   int count (Paramset mask)  {
      size_t result = 0;
      for (size_t i = 0; i < subset_size; i++)
         result += (mask >> i) % 2;
      return result;
   }

   /**
    * Test if none of the parametrizations is present.
    * @param[in] paramset	paramset to count bits in
    * @return true if none of the paremters is set
    */
   inline bool hasNone(Paramset paramset)  {
      return (paramset == none);
   }

   /**
    * Get a number of the first on bit.
    * @param[in] paramset	bitmask that is required to have just one bit on
    * @return  position of the bit in the mask (from the left)
    */
   inline size_t getBitNum(Paramset paramset)  {
      if (count(paramset) != 1)
         throw runtime_error("getBitNum called on colorset with different number of on bits than one.");

      // Shift bits until you reach the one that is on
      size_t bit_num = 0;
      for (; bit_num < getSetSize(); bit_num++, paramset >>= 1) {
         if (paramset % 2)
            break;
      }

      // Return the bit position
      return ((getSetSize() - 1) - bit_num);
   }
}

#endif // PARSYBONE_PARAMSET_HELPER_INCLUDED
