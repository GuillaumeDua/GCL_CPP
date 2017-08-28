#pragma once // not using standard header-guard in order to not pollute macro completion on GCL_TEST*

#include <gcl_cpp/type_info.hpp>
#include <gcl_cpp/test.hpp>

namespace gcl
{
	namespace test
	{
		struct type_info
		{
			struct tuple
			{
				static void proceed()
				{
					struct Toto{};
					struct Titi{};
					struct Tata{};
					struct Tutu{};
					struct NotInPack {};

					static_assert(std::is_same<gcl::type_info::tuple<int>, gcl::type_info::tuple<int>>::value, "gcl::test::type_info::tuple : tuple must be an alias");

					using my_pack_t = typename gcl::type_info::tuple<Toto, Titi, Tata, Tutu>;

					GCL_TEST__EXPECT_VALUE(my_pack_t::indexOf<Tata>(), std::size_t{ 2 });
					GCL_TEST__EXPECT_EXCEPTION(std::out_of_range, []() { my_pack_t::indexOf<NotInPack>(); });
				}
			};

			struct id
			{
				static void proceed()
				{
					int i;
					GCL_TEST__EXPECT(gcl::type_info::id<int>::value == gcl::type_info::id<decltype(i)>::value, "gcl::type_info::id : bad value");
					GCL_TEST__EXPECT(gcl::type_info::id<int>::value != gcl::type_info::id<std::string>::value, "gcl::type_info::id : bad value");
				}
			};

			struct holder
			{
				static void proceed()
				{
					struct interface_t {};
					struct concret1_t : interface_t {};
					struct concret1_2 : interface_t {};

					using holder_t = gcl::type_info::holder<interface_t>;

					holder_t t1_from_unique_ptr{ std::move(std::unique_ptr<concret1_t>{}) };
					holder_t t1_from_raw_ptr{ new concret1_t{} };
					holder_t t1_from_id_and_interface_ptr{ gcl::type_info::id<concret1_t>::value, std::make_unique<interface_t>() };

					GCL_TEST__EXPECT(
						t1_from_id_and_interface_ptr.id == t1_from_unique_ptr.id &&
						t1_from_unique_ptr.id == t1_from_raw_ptr.id, "gcl::type_info::holder : bad id"
					);
				}
			};

			struct experimental
			{
				struct any
				{
					static void proceed()
					{
						struct type_1 : std::string
						{
							type_1(std::string && str)
								: std::string(str)
							{}
						};
						struct type_2
						{
							type_2(uint32_t && value)
								: value_(value)
							{}

							inline uint32_t get_value(void) const
							{
								return value_;
							}
							inline bool operator==(const type_2 & other) const
							{
								return this->value_ == other.get_value();
							}
						private:
							uint32_t value_;
						};

						std::unique_ptr<gcl::type_info::experimental::any> any_str_1(new gcl::type_info::experimental::any_impl<type_1>("13"));
						std::unique_ptr<gcl::type_info::experimental::any> any_str_2(new gcl::type_info::experimental::any_impl<type_1>("42"));
						std::unique_ptr<gcl::type_info::experimental::any> any_uint32_t_1(new gcl::type_info::experimental::any_impl<type_2>(13));
						std::unique_ptr<gcl::type_info::experimental::any> any_uint32_t_2(new gcl::type_info::experimental::any_impl<type_2>(42));

						GCL_TEST__EXPECT(
							(*any_str_1 == *any_str_1)
							&& !(*any_str_1 == *any_str_2)
							&& (*any_uint32_t_1 == *any_uint32_t_1)
							&& !(*any_uint32_t_1 == *any_uint32_t_2)
							&& !(*any_uint32_t_1 == *any_str_1),
							"gcl::any::operator==(const gcl::any &)");
					}
				};
				using pack_t = std::tuple<any>;
			};

			using pack_t = std::tuple<tuple, holder, experimental>;
		};
	}
}