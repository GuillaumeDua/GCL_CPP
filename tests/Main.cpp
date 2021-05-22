// static tests
#include <gcl/mp/mp.hpp>
#include <gcl/cx/cx.hpp>
#include <gcl/compile_time_constant/ctc.hpp>
#include <gcl/mp/preprocessor.hpp>
#include <gcl/concepts.hpp>
#include <gcl/functional.hpp>

// dynamic tests
#include <gcl/io/io.hpp>
#include <gcl/container/interval_map.hpp>
#include <gcl/algorithms/algorithms.hpp>

#include <stdexcept>
#include <iostream>

auto main(int, char const*[]) -> int
{
    try
    {
        // tmp();
        // gcl::signals::initialize();
        // gcl::test::proceed();
        gcl::container::test::interval_map::test();
        gcl::algorithms::tests::ranges::test();
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
