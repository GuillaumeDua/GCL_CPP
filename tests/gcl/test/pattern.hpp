#pragma once

#include <gcl_cpp/test/pattern/ecs.hpp>
#include <tuple>

namespace gcl::test
{
	struct pattern
	{
		using dependencies_t = std::tuple
		<
			gcl::test::pattern_impl::ecs
		>;
	};
}