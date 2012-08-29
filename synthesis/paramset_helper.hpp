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
/// Here are definitions that provide help when working with subsets of parametrization space.
/// These functions work only for parameters that are basic data types (namely integer).
/// @attention Parameters in an Paramset are ordered in an ascending order.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ParamsetHelper {
   const static std::size_t subset_size = sizeof(Paramset) * 8; ///< size in bits of a single subset of parametrization space
   const static Paramset all = static_cast<Paramset>(~0); ///< parametrization set with all the bits set to 1

public:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// VALUE GETTERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   /**
    * @return number of parameters in a single round
    */
   inline const static std::size_t getParamsetSize() {
      return subset_size;
   }
   /**
    * @return a parameter set with everything set to 1
    */
   inline Paramset getAll() const {
      return all;
   }

   /**
    * @return mask that holds value of the binary form 10...0 .
    */
   Paramset getLeftOne(ColorNum size = subset_size ) const {
      return (1 << (size - 1));
   }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// TRANSFORMERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   /**
    * Computer a vector of masks of single parametrizations - i.e. 10010 would give {10000,00010}
    */
   std::vector<Paramset> getSingleMasks(Paramset parameters) const {
      std::vector<Paramset> masks;
      Paramset mask = getLeftOne();
      for (std::size_t counter = 0; counter < getParamsetSize(); counter++) {
         if (mask & parameters)
            masks.push_back(mask);
         mask >>= 1;
      }
      return masks;
   }

   /**
    * Return a paramset with on bits corresponding to requested numbers - i.e. for {1,3} we would get 1010...0
    *
    * @param vector of number in range [1,|paramset|]
    *
    * @return Paramset mask
    */
   const Paramset getMaskFromNums(std::vector<std::size_t> numbers) {
      Paramset full_mask = 0;
      for (auto num = numbers.begin(); num != numbers.end(); num++) {
         if (*num > getParamsetSize())
            throw std::runtime_error("Mask asked for number bigger that is the size of Paramset mask.");
         full_mask |= 1 << (getParamsetSize() - (*num + 1));
      }
      return full_mask;
   }

   /**
    * Flips every bit
    *
    * @return copy of input with swapped bits.
    */
   inline Paramset flip(const Paramset parameters)  const {
      return ~parameters;
   }

   /**
    * Swaps parameters within a variable - last become first etc.
    *
    * @param parameters	parameters to swap
    *
    * @return copy of input with descending order of parameters
    */
   inline Paramset swap(Paramset parameters) const {
      Paramset new_params = 0;
      for (std::size_t param_num = 0; param_num < getParamsetSize(); param_num++) {
         new_params <<= 1;
         if (parameters % 2)
            new_params |= 1;
         parameters >>= 1;
      }
      return new_params;
   }

   /**
    * Swaps parameters within a variable - last become first etc.
    *
    * @param parameters	parameters to swap
    * @param shift	if there are not all the parameters used, shift back after swapping
    *
    * @return copy of input with descending order of parameters
    */
   inline Paramset swap(Paramset parameters, std::size_t shift) const {
      parameters = swap(parameters);
      parameters >>= shift;
      return parameters;
   }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ANALYZERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   /**
    * Count bits
    * From The Art of Computer Programming Vol IV, p 11
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
    * @return true if none of the paremters is set
    */
   inline const bool none(Paramset parameters) const {
      return (parameters == 0);
   }

   /**
    * Get number of the on bit.
    *
    * @param color	bitmask that is required to have just one bit on
    *
    * @return position of the bit in the mask (from the left)
    */
   inline const std::size_t getBitNum(Paramset color) const {
      if (count(color) != 1)
         throw std::runtime_error("getBitNum called on colorset with different number of on bits than one.");

      // Shift bits until you reach the one that is on
      std::size_t bit_num = 0;
      for (; bit_num < getParamsetSize(); bit_num++, color >>= 1) {
         if (color % 2)
            break;
      }

      // Return the bit position
      return ((getParamsetSize() - 1) - bit_num);
   }
} paramset_helper;

#endif // PARSYBONE_PARAMSET_HELPER_INCLUDED
