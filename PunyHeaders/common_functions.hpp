/*
 * Copyright (C) 2012-2013 - Adam Streck
 * Independent headers for software development.
 */
#ifndef COMMON_FUNCTIONS_HPP
#define COMMON_FUNCTIONS_HPP

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file A handful of handy functions.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
using boost::lexical_cast;
using boost::bad_lexical_cast;
using boost::is_any_of;
using boost::split;
using boost::trim;

// Header for demangle function
#ifdef __GNUC__
#include <cxxabi.h>
#endif

namespace Common {
   /**
    * @brief Return string with name of the used type, based on the architecture.
    * @param[in] type_info  a structure holding infromation about the type whose name is requested
    * @return a string with the demangle name of the type
    */
   string demangle(const type_info & type_info) {
#ifdef __GNUC__
      int status;
      return string(abi::__cxa_demangle(type_info.name(), 0, 0, &status));
#else
      return string(type_info.name());
#endif
   }

   /**
    * @brief Python-like range function - returns a vector of values in the range [begin,end[.
    * @param[in] begin  first number in the range
    * @param[in] end    first number not in the range
    * @return  vector of values from the range [begin,end[
    */
   template<typename IntegralType>
   vector<IntegralType> range(const IntegralType begin, const IntegralType end) {
      vector<IntegralType> my_arr(end - begin);
      for (IntegralType i = static_cast<IntegralType>(0); i < end - begin; i++) {
         my_arr[i] = begin + i;
      }
      return my_arr;
   }

   /**
    * @brief Python-like range function - returns a vector of values in the range [0,end[.
    * @param[in] end    first number not in the range
    * @return  vector of values from the range [0,end[
    */
   template<typename IntegralType>
   vector<IntegralType> range(const IntegralType end) {
      return range<IntegralType>(static_cast<IntegralType>(0), end);
   }

   template<typename IntegralType>
   vector<IntegralType> range(const pair<IntegralType, IntegralType> val_range) {
      return range<IntegralType>(val_range.first, val_range.second);
   }

   /**
    * @brief Range adapter foc containers - used to create a vector of values corresponding to indexes of the members of a container.
    * @param[in] container   container whose size is in question.
    * @return  vector of values from the range [0,container.size()[
    */
   template<typename SeqType>
   vector<size_t> scope(const SeqType & container) {
      typedef decltype(container.size()) ContSizeT;
      return range(static_cast<ContSizeT>(0), container.size());
   }

   /**
    * @brief Conversion of basic types to string data type. If an error occurs, displays it and throws an exception.
    * @param[in] data variable to convert
    * @return  converted string
    */
   template<typename basic_T>
   string toString(basic_T data) {
      string result;
      try {
         result = move(lexical_cast<string, basic_T>(data));
      } catch (bad_lexical_cast e) {
         cerr << "! Error occured while trying to cast a variable" << data << " to a string: " + string(e.what());
         throw runtime_error("lexical_cast<string," + string(typeid(basic_T).name()) + ">(\"data\")) failed");
      }
      return result;
   }

   /**
    * @brief Increases integral value by 1.
    * @param[in,out] val  reference to value that will be increased
    */
   template<typename IntegralType>
   void increase(typename vector<IntegralType>::reference val) {val++;}

   /**
    * @brief Sets boolean value to true.
    * @param[in,out] val  reference to value that will be increased
    */
   template<>
   void increase<bool>(vector<bool>::reference val) {val = true;}

   /**
    * @brief Iterates values from left to right if it is possible. If so, return true, otherwise return false.
    * @param[in] top    vector of maximal values each component can reach
    * @param[in] bottom vector   of minimal values each component can reach
    * @param[in,out] iterated    vector of values to iterate
    * @return  true if the iteration was valid, false if it caused overflow (iterated > bottom)
    */
   template<typename IntegralType>
   bool iterate(const vector<IntegralType> & top, const vector<IntegralType> & bottom, vector<IntegralType> & iterated) {
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
   size_t getLenght(const pair<IntegralType, IntegralType> range) {
      return (range.second - range.first + 1);
   }

} using namespace Common;

#endif // COMMON_FUNCTIONS_HPP
