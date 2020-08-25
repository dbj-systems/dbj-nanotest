#pragma once
#ifndef DBJ_FORMAT_INC_
#define DBJ_FORMAT_INC_
/* (c) 2019,2020 by dbj.org   -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ */

#ifdef __clang__
#pragma clang system_header
#endif // __clang__

#include "../dbj--nanolib/nonstd/nano_printf.h"
#include "vt100win10.h"

#undef DBJ_LOG_MAX_LINE_LEN
#define DBJ_LOG_MAX_LINE_LEN 1024U
///
/// the most basic output is achieved by redirecting stderr to a file
/// so if you use stdout you will know (if you have a console that is)
#undef  DBJ_DEFAULT_LOG_STD_TARGET
#define DBJ_DEFAULT_LOG_STD_TARGET stderr


// will exit if formating strings have escape chars
// never used in release builds
#ifndef NDEBUG
#define DBJ_NANO_LOG_NO_ESCAPE_CODES_FATAL
#endif // !NDEBUG
//
#undef DBJ_ERR_PROMPT
#undef DBJ_FILE_LINE_TSTAMP

#define DBJ_FILE_LINE_TSTAMP __FILE__ "(" _CRT_STRINGIZE(__LINE__) ")[" __TIMESTAMP__ "] "
/* will not compile if MSG_ is not string literal */
#define DBJ_ERR_PROMPT(MSG_) DBJ_FILE_LINE_TSTAMP MSG_
///------------------------------------------------------------------
#define DBJ_NANO_LOG_FAIL_POLICY(...) perror(DBJ_ERR_PROMPT( # __VA_ARGS__ ) ); exit(1);
//
#define _CRT_SECURE_NO_WARNINGS 1

//
#include <array>
#include <string_view>
//
// here is a lot of virtual tables
// not fast
// TODO: change to std::format usage when time comes
#include <sstream>
#include <tuple>
//
#define __STDC_LIB_EXT1__ 1
#ifndef __STDC_LIB_EXT1__
#error __STDC_LIB_EXT1__ needs to be defined (as 1) before using gmtime_s
#endif
#include <ctime>

namespace dbj::nanolib::ostrmng
{
		inline int default_sink_function(std::string_view log_line_) 
		{ 
			return fprintf(DBJ_DEFAULT_LOG_STD_TARGET, "\n%s", log_line_.data()); 
		}
#pragma region tuple prinf
	// for ADL to work (https://en.cppreference.com/w/cpp/language/adl)
	// this operator has to be in the same namespace as prinf() and logf()

	// https://stackoverflow.com/a/54383242/10870835
	// currently (2020 Q1) we base output processing
	// on ostringstream
	// when C++20 compilers stabilize we will switch to std::format

	namespace detail
	{
		template <class TupType, size_t... I>
		inline std::ostream& tuple_print(std::ostream& os,
			const TupType& _tup, std::index_sequence<I...>)
		{
			os << "(";
			(..., (os << (I == 0 ? "" : ", ") << std::get<I>(_tup)));
			os << ")";
			return os;
		}
	} // namespace detail

	template <class... T>
	inline std::ostream& operator<<(std::ostream& os, const std::tuple<T...>& _tup)
	{
		return detail::tuple_print(os, _tup, std::make_index_sequence<sizeof...(T)>());
	}

#pragma endregion tuple prinf

	using namespace std;

	// ----------------------------------------------------------------------------------------------------
	// general ostrmng function
	template <
		typename T1, typename... T2>
		inline void prinf(const T1& first_param, const T2 &... params)
	{
		using namespace std;

		char buff_[DBJ_LOG_MAX_LINE_LEN]{/*zero it the buff_*/ };
		ostringstream os_(buff_);

		auto out = [&](auto const& obj_) {
			os_ << obj_;
		};

		out(first_param);
		(..., (out(params))); // the rest

		os_.flush();

		default_sink_function(os_.str());
	}

	///------------------------------------------------------------
	/// formatted prinf
	template <typename... Args>
	inline void prinfmt(const char* format_, Args... args) noexcept
	{
		DBJ_NANO_LIB_SYNC_ENTER;

#ifdef DBJ_NANO_LOG_NO_ESCAPE_CODES_FATAL
		if (
			strchr(format_, '\n') ||
			strchr(format_, '\r') ||
			strchr(format_, '\t')
			)
		{
#ifdef NDEBUG
			perror("\n\nFATAL ERROR\n\n" DBJ_ERR_PROMPT("\n\nDo not use escape codes in dbj nano ostrmng formatting\n"));
			exit(1);
#else
			printf("\n\nFATAL ERROR IN RELEASE BUILDS!\n\n" DBJ_ERR_PROMPT("\n\nDo not use escape codes in dbj nano ostrmng formatting\n"));
#endif // DEBUG
		}
#endif // NDEBUG

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-security"
#endif // __clang__

		auto buffy = v_buffer::format(format_, args...);

		//size_t sz = std::snprintf(nullptr, 0, format_, args...);
		//std::vector<char> buffer_(sz + 1); // +1 for null terminator
		//std::snprintf(&buffer_[0], buffer_.size(), format_, args...);

		default_sink_function(buffy.data());
#ifdef __clang__
#pragma clang diagnostic pop
#endif // __clang__
		DBJ_NANO_LIB_SYNC_LEAVE;
		}


#pragma region test macros

#undef TU_CHECK
#define TU_CHECK(x) do {   \
if (false == (x))      \
{                      \
::dbj::nanolib::ostrmng::prinf(DBJ_FG_YELLOW, #x, DBJ_RESET, DBJ_FG_RED_BOLD, " -- Failed! ", DBJ_RESET); \
}                      \
} while (0)

/*
	TX stands for Test eXpression
	I know my core principle is not to use iostringstreams, but I am not a zealot
	I am an pragmatist. For simple and usefull test displays one can use iostreams,
	like in this macro bellow.

	Usage:

	DBJ_TX( 4 + 2 );
	*/
#undef DBJ_TX 

#define DBJ_TX(x) do {\
dbj::nanolib::ostrmng::prinf("\n\nExpression: '", DBJ_FG_YELLOW, #x, \
DBJ_RESET, "'\nResult: ", DBJ_FG_YELLOW_BOLD, (x), \
DBJ_RESET, " -- Type: ", typeid(x).name()); \
} while (0)

#pragma endregion

	} // namespace dbj::nanolib::ostrmng

#endif // !DBJ_FORMAT_INC_
