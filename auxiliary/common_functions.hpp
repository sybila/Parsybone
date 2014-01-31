/*
* Copyright (C) 2012-2013 - Adam Streck
* Independent headers for software development.
*/
#ifndef COMMON_FUNCTIONS_HPP
#define COMMON_FUNCTIONS_HPP

#include "data_types.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file A handful of handy functions.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Common {
	/**
	* @brief renaming for boost::counting_range
	* @param[in] begin  first number in the range
	* @param[in] end    first number not in the range
	* @return  a counting range for [begin,end[
	*/
	template<typename IntegralType>
	inline auto crange(const IntegralType begin, const IntegralType end) -> decltype(boost::counting_range(begin, end)) {
		return boost::counting_range(begin, end);
	}

	/**
	* @brief an interface to boost::counting_range
	* @param[in] end    first number not in the range
	* @return  a counting range for [0,end[
	*/
	template<typename IntegralType>
	inline auto crange(const IntegralType end) -> decltype(boost::counting_range(end, end))  {
		return boost::counting_range(static_cast<IntegralType>(0), end);
	}

	/**
	* @brief A range adapter for containers with the size() operator.
	* @param[in] container   container whose size is in question.
	* @return  a counting range for [0,container.size()[
	*/
	template<typename SeqType>
	inline auto cscope(const SeqType & container) -> decltype(crange(container.size()))  {
		typedef decltype(container.size()) ContSizeT;
		return crange(static_cast<ContSizeT>(0), container.size());
	}

	/**
	* @brief creates a vector with the given range
	* @param[in] begin    first number in the range
	* @param[in] end    first number not in the range
	* @return  a vector for [begin,end[
	*/
	template<typename IntegralType>
	inline std::vector<IntegralType> vrange(const IntegralType begin, const IntegralType end) {
		std::vector<IntegralType> values(end - begin);

		size_t val = begin;
		generate(values.begin(), values.end(), [&val](){return val++; });

		return values;
	}

	/**
	* @brief creates a vector with the given range
	* @param[in] end    first number not in the range
	* @return  a vector for [0,end[
	*/
	template<typename IntegralType>
	inline std::vector<IntegralType> vrange(const IntegralType end)  {
		return vrange(static_cast<IntegralType>(0), end);
	}


	/**
    * @brief Increases integral value by 1.
 	* @param[in,out] val  reference to value that will be increased
	*/
	template<typename IntegralType>
	void increase(typename std::vector<IntegralType>::reference val) { val++; }

	/**
	* @brief Sets boolean value to true.
	* @param[in,out] val  reference to value that will be increased
	*/
	template<>
	void increase<bool>(std::vector<bool>::reference val) { val = true; }

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
	TypeName& operator=(TypeName && ) = default; 

#endif // COMMON_FUNCTIONS_HPP
