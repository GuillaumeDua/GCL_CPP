// #define _GCL_DEBUG

// static tests

#include <gcl/mp/mp.hpp>
#include <gcl/cx/cx.hpp>
#include <gcl/compile_time_constant/ctc.hpp>

// dynamic tests

#include <gcl/serialization.hpp>

void tmp()
{	// only for gcl_v2 WIP
    gcl::serialization::p1::test();
    gcl::serialization::p5::test();
}

#include <stdexcept>
#include <iostream>

auto main(int, char const*[]) -> int
{
    try
    {
        tmp();
        // gcl::signals::initialize();
        // gcl::test::proceed();
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