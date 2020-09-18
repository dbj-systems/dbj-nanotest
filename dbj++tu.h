#pragma once
#ifndef DBJ_TU_INCLUDED
#define DBJ_TU_INCLUDED
/* (c) 2019,2020 by dbj.org   -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ */

#ifdef __clang__
#pragma clang system_header
#endif // __clang__

#include "vt100win10.h"

// always MT
#undef  DBJ_NANO_LIB_MT
#define DBJ_NANO_LIB_MT
#include "../dbj--nanolib/dbj++nanolib.h"
#include "../dbj--nanolib/dbj_nano_synchro.h"
#include "../dbj--nanolib/nonstd/dbj++array.h"
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

/*
now notice this static in here
without it global ad-hoc lambda instances compiled are not guaranteed to be unique
on the level of the app!
*/
#define TU_REGISTER static auto \
_DBJ_CONCATENATE(dbj_unused_tu_function_pointer_, __LINE__) \
= ::dbj::tu::tu_catalog().append_test_function

// requires function pointer
// they are in there but
// for some reason clang loses lambdas somewhere
#define TUF_REG( FP_ ) static auto \
_DBJ_CONCATENATE(dbj_unused_tu_function_pointer_, __LINE__) \
= ::dbj::tu::tu_catalog().append_test_function( FP_ )

#define TU_REGISTER_NOT inline auto \
_DBJ_CONCATENATE(dbj_unused_tu_function_pointer_, __LINE__) = 

#ifdef _DEBUG
#define DBJ_CHECKING_IS_TU_FP_UNIQUE 1
#else
#define DBJ_CHECKING_IS_TU_FP_UNIQUE 0
#endif

namespace dbj::tu
{
	// using namespace std;


	/// ---------------------------------------------------------------
	//inline units_sequence_type& units_()
	//{
	//	static units_sequence_type units_single_instance_{};
	//	return units_single_instance_;
	//};
	/// ---------------------------------------------------------------
	struct testing_system final
	{
	private:
		using tu_function = void (*)();

		/*
		4095 test units_ is a lot of test units_ for any kind of project
		more than 4095 test units_ means something is wrong with
		a project (laugh)
		*/
		constexpr static inline size_t fp_storage_size{ 0xFFF };

		using units_sequence_type
			= DBJ_ARRAY_WITH_PUSH
			<  tu_function, fp_storage_size>;

		units_sequence_type units_{};
	public:
		// method for adding test functions
		// NOTE: __clang__ and __GNUC__  are following the standard
		// so passing lambda/fp by value does the "degrade"
		// to the "function pointer"
		// side effect of that is same adresses for different lambdas
		// meaning: loosing them
		// so be sure to pass lambda/fp by const ref
		volatile auto append_test_function
		(tu_function const & fun_) noexcept
		{
			DBJ_LOCAL_LOCK;

#if DBJ_CHECKING_IS_TU_FP_UNIQUE
			{
				bool test_found_before_registration{ false };

				for (tu_function elem : units_)
				{
					if (elem == fun_)
					{
						test_found_before_registration = true;
						break;
					}
				}

				if (test_found_before_registration)
				{
					// NOTE: this will not link in release builds
					_CrtDbgReportW(_CRT_ERROR, 
						_CRT_WIDE(__FILE__), __LINE__, NULL, L"Test Unit [%p], found already registered.",  fun_
					);
				}
			}
#endif
			auto rezult = units_.push_back(fun_);
			DBJ_ASSERT(rezult != nullptr);
			return fun_;
		}

		// do not warn about address %4X formating char
#pragma warning( push )
#pragma warning( disable : 4477 4313 )

		int execute(bool listing_ = false) noexcept
		{
			DBJ_LOCAL_LOCK;

			unsigned counter_{};
			start();

			for (volatile tu_function tu_ : units_)
			{
				if (tu_ == nullptr) {
					// this is serious and I should work on this not to happen
					// try signaling even in release build
					// stderr should be redirected to a file
					fprintf( stderr, "\nERROR : %s(%d) : %s" ,__FILE__, __LINE__, "This should not happen?");
					break;
				}

				DBJ_PRINT(DBJ_FG_CYAN "\nTest Unit:  " DBJ_FG_RED_BOLD "%d [%4X]" DBJ_RESET,
					counter_++, tu_);
				line_of_hyphens();

				if (listing_)
					continue;

				dbj::nanolib::timer timer_{};

				if (tu_)
					(tu_)();

				DBJ_PRINT(DBJ_FG_CYAN);
				DBJ_PRINT("\nDone in: %s", as_buffer(timer_).data());
				line_of_hyphens();
				DBJ_PRINT(DBJ_RESET);
			}
			if (!listing_)
				after_loop();

			return EXIT_SUCCESS;
		}

#pragma warning( pop )

	private:

		void start(int = 0, char** = nullptr) noexcept
		{
			DBJ_PRINT(DBJ_FG_CYAN);
			DBJ_PRINT("\nDBJ++TESTING ---------------------------------------");
			line_of_hyphens();
#ifdef __clang__
			//__clang__             // set to 1 if compiler is clang
			//	__clang_major__       // integer: major marketing version number of clang
			//	__clang_minor__       // integer: minor marketing version number of clang
			//	__clang_patchlevel__  // integer: marketing patch level of clang
			//	__clang_version__     // string: full version number
			DBJ_PRINT(DBJ_FG_CYAN "\nCLANG: %s" DBJ_RESET, __clang_version__);
#else
			DBJ_PRINT("\n_MSVC_LANG: %lu", _MSVC_LANG);
#endif
#if DBJ_TERMINATE_ON_BAD_ALLOC
			DBJ_PRINT(DBJ_FG_RED_BOLD "\nProgram is configured to terminate on heap memory exhaustion" DBJ_RESET);
#else
			DBJ_PRINT(DBJ_FG_RED_BOLD "\nProgram is configured to throw std::bad_alloc on heap memory exhaustion" DBJ_RESET);
#endif
			DBJ_PRINT("\nCatalogue has %zd test units_", units_.size());
			line_of_hyphens();
			DBJ_PRINT(DBJ_RESET);
		}

		void after_loop() noexcept
		{
			DBJ_PRINT(DBJ_FG_CYAN "\n%s" DBJ_RESET, "All tests done.\n" );
		}

		void line_of_hyphens() noexcept
		{
			DBJ_PRINT("\n----------------------------------------------------------------------\n");
		}

	}; // testing system

	inline testing_system & tu_catalog()
	{
		static testing_system instance_{};
		return instance_;
	}

} // namespace dbj::tu

#include "ostrstream_operators.h"
#include "ostrmng.h"

#endif // DBJ_TU_INCLUDED