#if not defined(GCL_ENABLE_RUNTIME_TESTS)
static_assert(false, "GCL_ENABLE_RUNTIME_TESTS required to build runtime tests");
#endif
#if not defined(GCL_ENABLE_COMPILE_TIME_TESTS)
static_assert(false, "GCL_ENABLE_COMPILE_TIME_TESTS required to build compile-time tests");
#endif

// static tests (GCL_ENABLE_COMPILE_TIME_TESTS)
#include <gcl/mp/mp.hpp>
#include <gcl/cx/cx.hpp>
#include <gcl/compile_time_constant/ctc.hpp>
#include <gcl/mp/preprocessor.hpp>
#include <gcl/concepts.hpp>
#include <gcl/functional.hpp>

// dynamic tests (GCL_ENABLE_RUNTIME_TESTS)
#include <gcl/io/io.hpp>
#include <gcl/container/tuple_view.hpp>

#if defined(__clang__)
# pragma message("[gcl] in " __FILE__ " : clang/clang-cl does not support ranges yet")
#elif not defined(__cpp_lib_ranges)
# pragma message("[gcl] in " __FILE__ " : missing __cpp_lib_ranges")
#else
# include <gcl/container/interval_map.hpp>
# include <gcl/algorithms/algorithms.hpp>
#endif

#include <stdexcept>
#include <iostream>

auto main(int, char const*[]) -> int
{
    try
    {   // GCL_ENABLE_RUNTIME_TESTS
        // gcl::signals::initialize();
        // gcl::test::proceed();
#if defined(__cpp_lib_ranges) and not defined(__clang__)
        gcl::container::test::interval_map::test();
        gcl::algorithms::tests::ranges::test();
#endif
        gcl::io::tests::serialization::test();

        std::cout << "gcl : all runtime tests succeed\n";
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error : " << ex.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Error : unknown" << std::endl;
    }

    return 0;
}

// todo : 1 component -> 1 cmake component
// todo : dependency graph visualization
