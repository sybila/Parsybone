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
/// @attention  Parameters in an Paramset are ordered in an ascending order.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ParamsetHelper {
   static std::size_t subset_size;

public:
   /**
    * @return number of parameters in a single round
    */
   inline static const std::size_t getParamsetSize() {
      return sizeof(Parameters) * 8;
   }

   /**
    * Count bits
    * From The Art of Computer Programming Vol IV, p 11
    */
   #define MASK_01010101 (((unsigned int)(-1))/3)
   #define MASK_00110011 (((unsigned int)(-1))/5)
   #define MASK_00001111 (((unsigned int)(-1))/17)
   int count (Parameters n) {
      n = (n & MASK_01010101) + ((n >> 1) & MASK_01010101) ;
      n = (n & MASK_00110011) + ((n >> 2) & MASK_00110011) ;
      n = (n & MASK_00001111) + ((n >> 4) & MASK_00001111) ;
      return n % 255 ;
   }

   /**
    * @return a parameter set with everything set to 1
    */
   inline Parameters getAll() {
      return ~0;
   }

   /**
    * @return mask that holds value of the binary form 10...0 .
    */
   Parameters getLeftOne(ColorNum size = (getParamsetSize() - 1) ) {
      return (1 << size);
   }

   /**
    * Computer a vector of masks of single parametrizations - i.e. 10010 would give {10000,00010}
    */
   std::vector<Parameters> getSingleMasks(Parameters parameters) {
      std::vector<Parameters> masks;
      Parameters mask = getLeftOne();
      for (std::size_t counter = 0; counter < getParamsetSize(); counter++) {
         if (mask & parameters)
            masks.push_back(mask);
         mask >>= 1;
      }
      return masks;
   }

   /**
    * @return true if none of the paremters is set
    */
   inline const bool none(Parameters parameters) {
      return (parameters == 0);
   }

   /**
    * Flips every bit
    *
    * @return copy of input with swapped bits.
    */
   inline Parameters flip(const Parameters parameters) {
      return ~parameters;
   }

   /**
    * Swaps parameters within a variable - last become first etc.
    *
    * @param parameters	parameters to swap
    *
    * @return copy of input with descending order of parameters
    */
   inline Parameters swap(Parameters parameters) {
      Parameters new_params = 0;
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
   inline Parameters swap(Parameters parameters, std::size_t shift) {
      parameters = swap(parameters);
      parameters >>= shift;
      return parameters;
   }

   /**
    * Get number of the on bit.
    *
    * @param color	bitmask that is required to have just one bit on
    *
    * @return position of the bit in the mask (from the left)
    */
   inline const std::size_t getBitNum(Parameters color) {
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
