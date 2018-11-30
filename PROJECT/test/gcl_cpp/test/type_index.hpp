#pragma once // not using standard header-guard in order to not pollute macro completion on GCL_TEST*


#include <gcl_cpp/type_index.hpp>
#include <gcl_cpp/test.hpp>
#include <memory>

namespace gcl::test
{
	struct type_index
	{
		struct interface_t { virtual int get_integer_value() { return 0; }; };
		struct A : interface_t { int get_integer_value() { return 1; } };
		struct B : interface_t { int get_integer_value() { return 2; } };

		struct generate
		{
			static void proceed()
			{
				using type_indexer_type = type_indexer<interface_t, A, B>;
				type_indexer_type type_indexer;

				using ptr_type = std::unique_ptr<interface_t>;

				GCL_TEST__EXPECT_VALUE(ptr_type{ type_indexer.generate<0>() }->get_integer_value(), 1);
				GCL_TEST__EXPECT_VALUE(ptr_type{ type_indexer.generate<1>() }->get_integer_value(), 2);
				GCL_TEST__EXPECT_EXCEPTION(std::out_of_range, [&type_indexer]() {type_indexer.generate(2); });
			}
		};
		using dependencies_t = std::tuple<generate>;
	};
}

namespace gcl::test::deprecated
{
	struct type_index
	{
		struct interface_is
		{
			static void proceed()
			{
				struct Interface { virtual const std::string name() = 0; };
				struct Toto : Interface { const std::string name() { return "Toto"; } };
				struct Titi : Interface { const std::string name() { return "Titi"; } };
				struct Tata : Interface { const std::string name() { return "Tata"; } };
				struct Tutu : Interface { const std::string name() { return "Tutu"; } };

				using mapped_type_t = gcl::deprecated::type_index::interface_is<Interface>::of_types<Toto, Titi, Tata, Tutu>;

				mapped_type_t::indexer index;

				// GCL_TEST__EXPECT_VALUE(&(index[2].default_constructor), &(index.at<Tata>().default_constructor)); // WTF does not compile ?

				auto constructor = index.at<Tata>().default_constructor;
				auto tata_instance = constructor();
				GCL_TEST__EXPECT_VALUE(tata_instance->name(), "Tata");
				delete tata_instance;

				GCL_TEST__EXPECT(index.size() == mapped_type_t::index.size(), "gcl::type_index : static and dynamic size mismatch");
			}
		};

		using dependencies_t = std::tuple<interface_is>;
	};
}