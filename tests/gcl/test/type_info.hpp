#pragma once // not using standard header-guard in order to not pollute macro completion on GCL_TEST*

#include <gcl_cpp/type_info.hpp>
#include <gcl_cpp/test.hpp>
#include <iostream>
#include <vector>
#include <functional>

namespace gcl::test
{
	struct type_info
	{
		struct variadic_template
		{
			static void proceed()
			{
				struct Toto {};
				struct Titi {};
				struct Tata {};
				struct Tutu {};
				struct NotInPack {};

				static_assert(std::is_same<gcl::type_info::variadic_template<int>, gcl::type_info::variadic_template<int>>::value, "gcl::test::type_info::variadic_template : variadic_template must be an alias");

				using my_pack_t = typename gcl::type_info::variadic_template<Toto, Titi, Tata, Tutu>;

				GCL_TEST__EXPECT_VALUE(my_pack_t::index_of<Tata>, std::size_t{ 2 });
				GCL_TEST__EXPECT(!my_pack_t::contains<NotInPack>);
			}
		};
		struct id
		{
			static void proceed()
			{
				int i;
				GCL_TEST__EXPECT(gcl::type_info::id::value<int> == gcl::type_info::id::value<decltype(i)>, "gcl::type_info::id : bad value");
				GCL_TEST__EXPECT(gcl::type_info::id::value<int> != gcl::type_info::id::value<std::string>, "gcl::type_info::id : bad value");
			}
		};
		struct experimental
		{
			struct holder
			{
				struct interface_and_cast
				{
					static void proceed()
					{
						struct A {};
						struct B {};

						gcl::type_info::experimental::holder holder_A_1{ A{} };
						gcl::type_info::experimental::holder holder_A_2{ A{} };
						gcl::type_info::experimental::holder holder_B_2{ B{} };

						GCL_TEST__EXPECT_VALUE(holder_A_1.id, holder_A_2.id);
						GCL_TEST__EXPECT_NOT_VALUE(holder_A_1.id, holder_B_2.id);
						GCL_TEST__EXPECT_EXCEPTION(std::bad_cast, [&holder_A_1]() { holder_A_1.cast<B>(); });
						[[maybe_unused]] A & value = holder_A_1.cast<A>();
					}
				};
				struct cleaning
				{
					static void proceed()
					{
						bool is_a_destructor_called_properly = false;
						struct A
						{
							bool & called;
							~A() { called = true; }
						};

						{
							gcl::type_info::experimental::holder holder_A{ A{ is_a_destructor_called_properly } };
						}
						GCL_TEST__EXPECT(is_a_destructor_called_properly);
					}
				};
				using dependencies_t = std::tuple<interface_and_cast, cleaning>;
			};
			class type_name
			{
				template <typename T>
				static constexpr void compare_with_typeid()
				{
					auto gcl_type_name = gcl::type_info::cx::type_name<T>(); // not constexpr since VS 15.8.9
					GCL_TEST__EXPECT_VALUE(typeid(T).name(), gcl_type_name, std::string{ gcl_type_name.data(), gcl_type_name.size() });
				}

			public:
				static void proceed()
				{	// Cannot test cv-qualifier and (l|r)value-references => typeid decay types
					// Cannot test with function => __cdecl

					compare_with_typeid<void>();
					compare_with_typeid<int>();
					compare_with_typeid<std::string>();
					compare_with_typeid<std::vector<std::string>>();
				}
			};

			using dependencies_t = std::tuple<holder, type_name>;
		};

		using dependencies_t = std::tuple<variadic_template, id, experimental>;
	};
}

namespace gcl::test::deprecated
{
	struct type_info
	{
		struct id
		{
			static void proceed()
			{
				int i;
				GCL_TEST__EXPECT(gcl::deprecated::type_info::id<int>::value == gcl::deprecated::type_info::id<decltype(i)>::value, "gcl::type_info::id : bad value");
				GCL_TEST__EXPECT(gcl::deprecated::type_info::id<int>::value != gcl::deprecated::type_info::id<std::string>::value, "gcl::type_info::id : bad value");
			}
		};

		struct holder
		{
			static void proceed()
			{
				struct interface_t {};
				struct concret1_t : interface_t {};
				struct concret1_2 : interface_t {};

				using holder_t = gcl::deprecated::type_info::holder<interface_t>;

				holder_t t1_from_unique_ptr{ std::move(std::unique_ptr<concret1_t>{}) };
				holder_t t1_from_raw_ptr{ new concret1_t{} };
				holder_t t1_from_id_and_interface_ptr{ gcl::deprecated::type_info::id<concret1_t>::value, std::make_unique<interface_t>() };

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

					std::unique_ptr<gcl::deprecated::type_info::experimental::any> any_str_1(new gcl::deprecated::type_info::experimental::any_impl<type_1>("13"));
					std::unique_ptr<gcl::deprecated::type_info::experimental::any> any_str_2(new gcl::deprecated::type_info::experimental::any_impl<type_1>("42"));
					std::unique_ptr<gcl::deprecated::type_info::experimental::any> any_uint32_t_1(new gcl::deprecated::type_info::experimental::any_impl<type_2>(13));
					std::unique_ptr<gcl::deprecated::type_info::experimental::any> any_uint32_t_2(new gcl::deprecated::type_info::experimental::any_impl<type_2>(42));

					GCL_TEST__EXPECT(
						(*any_str_1 == *any_str_1)
						&& !(*any_str_1 == *any_str_2)
						&& (*any_uint32_t_1 == *any_uint32_t_1)
						&& !(*any_uint32_t_1 == *any_uint32_t_2)
						&& !(*any_uint32_t_1 == *any_str_1),
						"gcl::any::operator==(const gcl::any &)");
				}
			};
			using dependencies_t = std::tuple<any>;
		};

		using dependencies_t = std::tuple<id, holder, experimental>;
	};
}