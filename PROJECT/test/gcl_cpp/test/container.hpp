#pragma once

#include <gcl_cpp/test/container/polymorphic_vector.hpp>
#include <gcl_cpp/test/container/entity_vector.hpp>

namespace gcl
{
	namespace test
	{
		struct container
		{
			using polymorphic_vector = gcl::test::container_partial_impl::polymorphic_vector;
			using entity_vector = gcl::test::container_partial_impl::entity_vector;

			using dependencies_t = std::tuple<polymorphic_vector, entity_vector>;
		};
	}
}