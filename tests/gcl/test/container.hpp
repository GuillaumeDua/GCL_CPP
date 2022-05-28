#pragma once

#include <gcl_cpp/test/container/polymorphic_vector.hpp>
#include <gcl_cpp/test/container/entity_vector.hpp>
#include <gcl_cpp/test/container/polymorphic_reference.hpp>
#include <gcl_cpp/test/container/component_aggregator.hpp>

namespace gcl::test
{
	struct container
	{
		using dependencies_t = std::tuple
		<
			gcl::test::container_impl::polymorphic_vector,
			gcl::test::container_impl::entity_vector,
			gcl::test::container_impl::polymorphic_reference,
			gcl::test::container_impl::component_aggregator
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