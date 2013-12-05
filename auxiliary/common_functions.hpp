/*
 * Copyright (C) 2012-2013 - Adam Streck
 * Independent headers for software development.
 */
#ifndef COMMON_FUNCTIONS_HPP
#define COMMON_FUNCTIONS_HPP

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file A handful of handy functions.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

// Header for demangle function
#ifdef __GNUC__
#include <cxxabi.h>
#endif

namespace Common {
   /**
    * @brief Python-like range function - returns a vector of values in the range [begin,end[.
    * @param[in] begin  first number in the range
    * @param[in] end    first number not in the range
    * @return  vector of values from the range [begin,end[
    */
   template<typename IntegralType>
   std::vector<IntegralType> range(const IntegralType begin, const IntegralType end) {
      std::vector<IntegralType> my_arr(end - begin);
      for (IntegralType i = static_cast<IntegralType>(0); i < end - begin; i++) {
         my_arr[i] = begin + i;
      }
      return my_arr;
   }
   template<typename IntegralType>
   inline std::vector<IntegralType> range(const IntegralType end) {
      return range(static_cast<IntegralType>(0), end);
   }

   template<typename IntegralType>
   inline std::vector<IntegralType> range(const std::pair<IntegralType, IntegralType> val_range) {
      return range(val_range.first, val_range.second);
   }

   /**
    * @brief Range adapter foc containers - used to create a vector of values corresponding to indexes of the members of a container.
    * @param[in] container   container whose size is in question.
    * @return  vector of values from the range [0,container.size()[
    */
   template<typename SeqType>
   inline std::vector<std::size_t> scope(const SeqType & container) {
      typedef decltype(container.size()) ContSizeT;
      return range(static_cast<ContSizeT>(0), container.size());
   }

   /**
    * @brief Conversion of basic types to string data type. If an error occurs, displays it and throws an exception.
    * @param[in] data variable to convert
    * @return  converted string
    */
   template<typename basic_T>
   std::string to_string(basic_T data) {
      std::string result;
      try {
         result = std::move(boost::lexical_cast<std::string, basic_T>(data));
      } catch (boost::bad_lexical_cast e) {
         std::cerr << "! Error occured while trying to cast a variable" << data << " to a string: " + std::string(e.what());
         throw std::runtime_error("lexical_cast<string," + std::string(typeid(basic_T).name()) + ">(\"data\")) failed");
      }
      return result;
   }

   /**
    * @brief Increases integral value by 1.
    * @param[in,out] val  reference to value that will be increased
    */
   template<typename IntegralType>
   void increase(typename std::vector<IntegralType>::reference val) {val++;}

   /**
    * @brief Sets boolean value to true.
    * @param[in,out] val  reference to value that will be increased
    */
   template<>
   void increase<bool>(std::vector<bool>::reference val) {val = true;}

   /**
    * @brief Iterates values from left to right if it is possible. If so, return true, otherwise return false.
    * @param[in] top    vector of maximal values each component can reach
    * @param[in] bottom vector   of minimal values each component can reach
    * @param[in,out] iterated    vector of values to iterate
    * @return  true if the iteration was valid, false if it caused overflow (iterated > bottom)
    */
   template<typename IntegralType>
   bool iterate(const std::vector<IntegralType> & top, const std::vector<IntegralType> & bottom, std::vector<IntegralType> & iterated) {
      for (size_t val_num = 0; val_num <= iterated.size(); val_num++) {
         if (val_num == iterated.size())
            return false;
         if (iterated[val_num] == top[val_num]) {
            iterated[val_num] = bottom[val_num];
         }
         else {
            increase<IntegralType>(iterated[val_num]);
            break;
         }
      }
      return true;
   }

   /**
    * @brief getLenght  computes lenght of an interval in a given pair
    * @param range   pair describing the range in the form [first,last)
    * @return  numerical lenght of the given range
    */
   template<typename IntegralType>
   size_t getLenght(const std::pair<IntegralType, IntegralType> range) {
      return (range.second - range.first + 1);
   }

} using namespace Common;

#define NO_COPY(TypeName) \
   TypeName() = default;  \
   TypeName(TypeName && ) = default;  \
   TypeName& operator=(TypeName && ) = default; \
   TypeName(const TypeName & ) = delete; \
   TypeName& operator=(const TypeName & ) = delete;

#define NO_COPY_SHORT(TypeName) \
   TypeName(const TypeName & ) = delete; \
   TypeName& operator=(const TypeName & ) = delete;

#define DEFAULT_MOVE(TypeName) \
   TypeName(TypeName && ) = default;  \
   TypeName& operator=(TypeName && ) = default; \

#endif // COMMON_FUNCTIONS_HPP
