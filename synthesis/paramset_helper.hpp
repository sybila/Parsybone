/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
 */


#ifndef PARSYBONE_PARAMSET_HELPER_INCLUDED
#define PARSYBONE_PARAMSET_HELPER_INCLUDED

#include "../auxiliary/output_streamer.hpp"
#include "../auxiliary/data_types.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Here are methods that provide help when working with subsets of parametrization space.
/// @attention Parametrizations in an Paramset are ordered in an ascending order.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ParamsetHelper {
   const static std::size_t subset_size = sizeof(Paramset) * 8; ///< Size in bits of a single subset of parametrization space.
   const static Paramset all = static_cast<Paramset>(~0); ///< Parametrization set with all the bits set to 1.

public:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// VALUE GETTERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   /**
    * @return  number of parametrizations in a single round
    */
   inline const static std::size_t getParamsetSize() {
      return subset_size;
   }
   /**
    * @return  paramset with everything set to 1
    */
   inline Paramset getAll() const {
      return all;
   }

   /**
    * @return  paramset that holds value of the binary form 10...0 (leftmost parametrization)
    */
   Paramset getLeftOne(ColorNum size = subset_size ) const {
      return (1 << (size - 1));
   }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// TRANSFORMERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   /**
    * Computer a vector of masks of single parametrizations - i.e. 10010 would give {10000,00010}.
    *
    * @param[in] paramset	paramset to disassamble
    *
    * @return vector containing a paramset with a single parametrization for each parametrization in input paramset
    */
   std::vector<Paramset> getSingleMasks(Paramset paramset) const {
      std::vector<Paramset> masks;
      Paramset mask = getLeftOne();
      for (std::size_t counter = 0; counter < getParamsetSize(); counter++) {
         if (mask & paramset)
            masks.push_back(mask);
         mask >>= 1;
      }
      return masks;
   }

   /**
    * Return a paramset with on bits corresponding to requested numbers - i.e. for {1,3} we would get 1010...0.
    *
    * @param[in] vector of number in range [1,|paramset|]
    *
    * @return  Paramset mask
    */
   const Paramset getMaskFromNums(const std::vector<std::size_t> numbers) const {
      Paramset full_mask = 0;
      for (auto num = numbers.begin(); num != numbers.end(); num++) {
         if (*num > getParamsetSize())
            throw std::runtime_error("Mask asked for number bigger that is the size of Paramset mask.");
         full_mask |= 1 << (getParamsetSize() - (*num + 1));
      }
      return full_mask;
   }

   /**
    * Flips every bit.
    *
    * @param[in] paramset	paramset to flip bits in
    *
    * @return  copy of input with swapped bits.
    */
   inline Paramset flip(const Paramset paramset)  const {
      return ~paramset;
   }

   /**
    * Swaps paramset within a variable - last become first etc.
    *
    * @param[in] paramset	paramset to swap bits in
    *
    * @return  copy of input with descending order of paramset
    */
   inline Paramset swap(Paramset paramset) const {
      Paramset new_params = 0;
      for (std::size_t param_num = 0; param_num < getParamsetSize(); param_num++) {
         new_params <<= 1;
         if (paramset % 2)
            new_params |= 1;
         paramset >>= 1;
      }
      return new_params;
   }

   /**
    * Swaps paramset within a variable - last become first etc.
    *
    * @param[in] paramset	paramset to swap bits in
    * @param[in] shift	if there are not all the paramset used, shift back after swapping
    *
    * @return  copy of input with descending order of paramset
    */
   inline Paramset swap(Paramset paramset, const std::size_t shift) const {
      paramset = swap(paramset);
      paramset >>= shift;
      return paramset;
   }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ANALYZERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   /**
    * Count bits. From The Art of Computer Programming Vol IV, p 11.
    *
    * @param[in] paramset	paramset to count bits in
    *
    * @return  number of parametrizations in the set
    */
   #define MASK_01010101 (((unsigned int)(-1))/3)
   #define MASK_00110011 (((unsigned int)(-1))/5)
   #define MASK_00001111 (((unsigned int)(-1))/17)
   int count (Paramset n) const {
      n = (n & MASK_01010101) + ((n >> 1) & MASK_01010101) ;
      n = (n & MASK_00110011) + ((n >> 2) & MASK_00110011) ;
      n = (n & MASK_00001111) + ((n >> 4) & MASK_00001111) ;
      return n % 255 ;
   }

   /**
    * Test if none of the parametrizations is present.
    *
    * @param[in] paramset	paramset to count bits in
    *
    * @return true if none of the paremters is set
    */
   inline const bool none(Paramset paramset) const {
      return (paramset == 0);
   }

   /**
    * Get a number of the first on bit.
    *
    * @param[in] paramset	bitmask that is required to have just one bit on
    *
    * @return  position of the bit in the mask (from the left)
    */
   inline const std::size_t getBitNum(Paramset paramset) const {
      if (count(paramset) != 1)
         throw std::runtime_error("getBitNum called on colorset with different number of on bits than one.");

      // Shift bits until you reach the one that is on
      std::size_t bit_num = 0;
      for (; bit_num < getParamsetSize(); bit_num++, paramset >>= 1) {
         if (paramset % 2)
            break;
      }

      // Return the bit position
      return ((getParamsetSize() - 1) - bit_num);
   }
} paramset_helper; ///< Single program-shared paramset helper object.

#endif // PARSYBONE_PARAMSET_HELPER_INCLUDED
