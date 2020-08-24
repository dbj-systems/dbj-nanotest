#pragma once

#include <sstream>

/*
primary runtime buffer is vector of a char_type
primary compile time buffer is array of a char_type
thus I will put required operators in here
to print them as strings
*/

//inline std::ostringstream& operator<<(std::ostringstream& os_, DBJ_VECTOR<char> buff_)
//{
//	if (os_.good())
//	{
//		os_ << buff_.data();
//	}
//	return os_;
//}
//
//inline std::wostringstream& operator<<(std::wostringstream& os_, DBJ_VECTOR<wchar_t> buff_)
//{
//	if (os_.good())
//	{
//		os_ << buff_.data();
//	}
//	return os_;
//}

//template <size_t N>
//inline std::ostringstream& operator<<(std::ostringstream& os_, std::array<char, N> buff_)
//{
//	if (os_.good())
//	{
//		os_ << buff_.data();
//	}
//	return os_;
//}
//
//template <size_t N>
//inline std::ostringstream& operator<<(std::ostringstream& os_, std::array<wchar_t, N> buff_)
//{
//	if (os_.good())
//	{
//		os_ << buff_.data();
//	}
//	return os_;
//}

#include <optional>

inline std::ostringstream& operator<<(std::ostringstream& os_, std::nullopt_t const&)
{
	if (os_.good())
	{
		os_ << "nullopt";
	}
	return os_;
}

/*
no this does not help
https://www.boost.org/doc/libs/1_34_0/boost/optional/optional_io.hpp
*/
template <typename T1>
inline std::ostringstream& operator<<(std::ostringstream& os_, std::optional<T1> const& opt_)
{
	if (os_.good())
	{
		if (opt_)
			return os_ << std::boolalpha << "{ " << *opt_ << " }";

		return os_ << "{ empty }";
	}
	return os_;
}

#include <functional>

/*
std::pair pair **was** in the core of valstat_1
*/
template <typename T1, typename T2>
inline std::ostringstream& operator<<(std::ostringstream& os_, std::pair<T1, T2> pair_)
{
	if (os_.good())
	{
		os_ << "{ " << pair_.first << " , " << pair_.second << " }";
	}
	return os_;
}
