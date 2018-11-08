#pragma once

#include <gcl_cpp/test/container/polymorphic_vector.hpp>
#include <gcl_cpp/test/container/entity_vector.hpp>
#include <gcl_cpp/test/container/polymorphic_reference.hpp>

namespace gcl::test
{
	struct container
	{
		using dependencies_t = std::tuple
			<
			gcl::test::container_impl::polymorphic_vector,
			gcl::test::container_impl::entity_vector,
			gcl::test::container_impl::polymorphic_reference
			>;
	};
}

namespace gcl::test::deprecated
{
	struct container
	{
		using dependencies_t = std::tuple
		<
			gcl::test::deprecated::container_impl::polymorphic_vector,
			gcl::test::deprecated::container_impl::entity_vector
		>;
	};
}