#pragma once
#ifndef DBJ_TU_INCLUDED
#define DBJ_TU_INCLUDED
/* (c) 2019,2020 by dbj.org   -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ */

#ifdef __clang__
#pragma clang system_header
#endif // __clang__

#include "vt100win10.h"

#include <functional>

#ifndef DBJ_NANOLIB_INCLUDED
#include "../dbj--nanolib/dbj++nanolib.h"
#endif

#ifndef DBJ_ARRAY_INCLUDED_
#include "../dbj--nanolib/nonstd/dbj++array.h"
#endif

#include "../dbj--nanolib/nonstd/dbj_timer.h"

// please see this if wondering why do we use array not vector
// #error https://stackoverflow.com/q/58569773/10870835

/*
testing nano-lib

Usage is as bellow.
Testing unit is a lambda with no arguments.

TU_REGISTER(
	[]{ / * to be used testing code * / }
);

TU_REGISTER_NOT (
	[]{ / * unused testing code * / }
);


void main() {
	// execute all registered tests
	dbj::tu::catalog.execute();
}

*/
#define TU_REGISTER inline auto \
_DBJ_CONCATENATE(dbj_unused_tu_function_pointer_, __LINE__) \
= ::dbj::tu::testing_system::append_test_function

// requires function pointer
// they are in there but
// for some reason clang loses lambdas somewhere
#define TUF_REG( FP_ ) inline auto \
_DBJ_CONCATENATE(dbj_unused_tu_function_pointer_, __LINE__) \
= ::dbj::tu::testing_system::append_test_function( FP_ )

#define TU_REGISTER_NOT inline auto \
_DBJ_CONCATENATE(dbj_unused_tu_function_pointer_, __LINE__) = 

#define DBJ_CHECKING_IS_TU_FP_UNIQUE 0

namespace dbj::tu
{
	using namespace std;

	using tu_function = void (*)();
	using tu_fun_obj  = function< void() > ;

	/*
	4095 test units is a lot of test units for any kind of project
	more than 4095 test units means something is wrong with
	a project logic
	*/
	constexpr size_t fp_storage_size{ 0xFFF };

	using units_sequence_type
		= DBJ_ARRAY_STORAGE
		<  tu_fun_obj, fp_storage_size>;

	inline void line() noexcept
	{
		DBJ_PRINT("----------------------------------------------------------------------");
	}
	/// ---------------------------------------------------------------
	inline units_sequence_type& units()
	{
		static units_sequence_type units_single_instance_{};
		return units_single_instance_;
	};
	/// ---------------------------------------------------------------
	struct testing_system final
	{

		// method for adding test functions
		// NOTE: __clang__ and __GNUC__  are following the standard
		// so passing lambda/fp by value does the "degrade"
		// to the "function pointer"
		// side effect of that is same adresses for different lambdas
		// meaning: loosing them
		// so be sure to pass lambda/fp by const ref
		static volatile auto append_test_function
		(tu_function const & fun_) noexcept
		{
#if DBJ_CHECKING_IS_TU_FP_UNIQUE
			{
				bool test_found_before_registration{ false };
				for (auto& elem : units)
				{
					if (elem == fun_)
					{
						test_found_before_registration = true;
						break;
					}
				}
				if (test_found_before_registration)
				{
					using dbj::nanolib::v_buffer;
					v_buffer::buffer_type report = v_buffer::format("Test Unit %s [%p], found before registration", typeid(fun_).name(), fun_);
					wstring final(report.begin(), report.end());
					_ASSERT_EXPR(false == test_found_before_registration, final.data());
				}
			}
#endif
			auto rezult = units().push_back(fun_);
			DBJ_ASSERT(rezult != nullptr);
			return fun_;
		}

		static void start(int = 0, char** = nullptr) noexcept
		{
			DBJ_PRINT(DBJ_FG_CYAN);
			DBJ_PRINT("DBJ++TESTING ---------------------------------------");
			line();
#ifdef __clang__
			//__clang__             // set to 1 if compiler is clang
			//	__clang_major__       // integer: major marketing version number of clang
			//	__clang_minor__       // integer: minor marketing version number of clang
			//	__clang_patchlevel__  // integer: marketing patch level of clang
			//	__clang_version__     // string: full version number
			DBJ_PRINT(DBJ_FG_CYAN "CLANG: %s" DBJ_RESET, __clang_version__);
#else
			DBJ_PRINT("_MSVC_LANG: %lu", _MSVC_LANG);
#endif
#if DBJ_TERMINATE_ON_BAD_ALLOC
			DBJ_PRINT(DBJ_FG_RED_BOLD "Program is configured to terminate on heap memory exhaustion" DBJ_RESET);
#else
			DBJ_PRINT(DBJ_FG_RED_BOLD "\nProgram is configured to throw std::bad_alloc on heap memory exhaustion" DBJ_RESET);
#endif
			DBJ_PRINT("Catalogue has %zd test units", units().size());
			line();
			DBJ_PRINT(DBJ_RESET);
		}

		static void end() noexcept
		{
			DBJ_PRINT(DBJ_FG_CYAN "%s" DBJ_RESET, "All tests done." );
		}

		// do not warn about address %4X formating char
#pragma warning( push )
#pragma warning( disable : 4477 4313 )

		static int execute(bool listing_ = false) noexcept
		{
			unsigned counter_{};
			start();

			for (const auto & tu_ : units())
			{
				DBJ_ASSERT(tu_);

				DBJ_PRINT(DBJ_FG_CYAN "Test Unit:  " DBJ_FG_RED_BOLD "%d [%4X]" DBJ_RESET,
					counter_++, &(tu_) );
				line();

				if (listing_)
					continue;

				dbj::nanolib::timer timer_{};

				if (tu_)
					(tu_)();

				DBJ_PRINT(DBJ_FG_CYAN);
				DBJ_PRINT("Done in: %s", as_buffer(timer_).data());
				line();
				DBJ_PRINT(DBJ_RESET);
			}
			if (!listing_)
				end();

			return EXIT_SUCCESS;
		}
	}; // testing system
#pragma warning( pop )

	/// constexpr /*inline*/ testing_system catalog;



} // namespace dbj::tu

#include "ostrstream_operators.h"
#include "ostrmng.h"

#endif // DBJ_TU_INCLUDED