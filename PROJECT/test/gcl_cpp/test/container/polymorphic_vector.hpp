#pragma once // not using standard header-guard in order to not pollute macro completion on GCL_TEST*

# include <gcl_cpp/container/polymorphic_vector.hpp>
# include <gcl_cpp/test.hpp>

namespace gcl::test::container_impl
{
	struct polymorphic_vector
	{
		struct A
		{
			A(std::string && s = "")
				: str{ std::forward<std::string>(s) }
			{}
			std::string str;
		};
		struct B {};
		struct C {};

		using type = gcl::container::polymorphic_vector;

		struct create_and_get
		{
			static void proceed()
			{
				type container{ A{}, B{}, A{ "titi" } };

				container.push_back(A{});
				container.push_back(B{});

				container.emplace_back<A>("toto");

				GCL_TEST__EXPECT_VALUE(container.get().size(), 6);
				GCL_TEST__EXPECT_VALUE(container.get<A>().size(), 4);
			}
		};
		struct visit
		{
			static inline type container_value{ A{ "foo" }, B{}, A{ "toto" }, B{}, C{}, A{ "bar" }, A{ "titi" } };

			struct visit_any_const
			{
				static void proceed()
				{
					const auto & container = container_value;

					std::size_t any_visitor_counter{ 0 };
					container.visit([&any_visitor_counter](const std::any &) { ++any_visitor_counter; });

					GCL_TEST__EXPECT_VALUE(any_visitor_counter, container.get().size());
					GCL_TEST__EXPECT_VALUE(any_visitor_counter, 7);
				}
			};
			struct visit_any_with_type_safety
			{
				static void proceed()
				{
					{	// change std::any underlying type
						type container{ A{ "foo" } };

#ifdef _DEBUG // type-consistency check are in debug only
						GCL_TEST__EXPECT_EXCEPTION(std::bad_typeid, [&container]()
						{
							container.visit([](std::any & value)
							{
								value = B{};
							});
						});
#endif
					}
					{	// not changing std::any underlying type
						type container{ A{ "foo" } };

						container.visit([](std::any & value)
						{
							value = A{ "bar" };
						});

						const auto & first_element = container.get<A>().at(0);
						const auto & first_element_as_A = std::any_cast<const A &>(*first_element);
						GCL_TEST__EXPECT_VALUE("bar", first_element_as_A.str);
					}
				}
			};
			struct visit_type_const
			{
				static void proceed()
				{
					const auto & container = container_value;

					std::size_t A_visitor_counter{ 0 };
					std::size_t A_visitor_start_with_t_counter{ 0 };
					container.visit<A>([&A_visitor_counter, &A_visitor_start_with_t_counter](const A & value)
					{
						if (!value.str.empty() && value.str.at(0) == 't')
							++A_visitor_start_with_t_counter;
						++A_visitor_counter;
					});

					GCL_TEST__EXPECT_VALUE(A_visitor_counter, container.get<A>().size());
					GCL_TEST__EXPECT_VALUE(A_visitor_counter, 4);
					GCL_TEST__EXPECT_VALUE(A_visitor_start_with_t_counter, 2);
				}
			};
			struct visit_type
			{
				static void proceed()
				{
					// type cannot be copied (cf std::unique_ptr ctors)
					type container{ A{ "foo" }, B{}, A{ "toto" }, B{}, C{}, A{ "bar" }, A{ "titi" } };

					container.visit<A>([](A & value)
					{
						value.str = "foobar";
					});

					std::size_t A_visitor_counter{ 0 };
					container.visit<A>([&A_visitor_counter](const A & value)
					{
						if (value.str == "foobar")
							++A_visitor_counter;
					});

					GCL_TEST__EXPECT_VALUE(A_visitor_counter, container.get<A>().size());
				}
			};


			using dependencies_t = std::tuple
				<
				visit_any_const,
				visit_any_with_type_safety,
				visit_type_const,
				visit_type
				>;
		};

		using dependencies_t = std::tuple<create_and_get, visit>;
	};
}

namespace gcl::test::deprecated::container_impl
{
	struct polymorphic_vector
	{
		struct A
		{
			virtual ~A() {}
			virtual void do_stuff() = 0;
			bool to_garbage = true;
		};
		struct B : A
		{
			B(int i)
				: _i(i)
			{}
			int _i;
			void do_stuff() {}
		};
		struct C : A
		{
			C(const std::string & str)
				: _str(str)
			{}
			std::string _str;
			void do_stuff() {}
		};

		static void proceed()
		{

			gcl::deprecated::container::polymorphic_vector<A> container;
			using container_elem_t = gcl::deprecated::container::polymorphic_vector<A>::element_t;

			container.emplace_back<B>(1);
			container.emplace_back<C>("one");

			for (auto & elem : container.get())
			{
				elem->do_stuff();
			}
			// B = B._i +1
			container.visit<B>([](A & elem)
			{
				dynamic_cast<B*>(&elem)->_i += 1;
			});
			// C = C._str + 'X'
			container.visit<C>([](A & elem)
			{
				dynamic_cast<C*>(&elem)->_str += 'X';
				elem.to_garbage = true;
			});

			B * b_ptr = dynamic_cast<B*>(container.get<B>().front());
			GCL_TEST__EXPECT_NOT_VALUE(b_ptr, nullptr);
			GCL_TEST__EXPECT_VALUE(b_ptr->_i, 2);

			C * c_ptr = dynamic_cast<C*>(container.get<C>().front());
			GCL_TEST__EXPECT_NOT_VALUE(c_ptr, nullptr);
			GCL_TEST__EXPECT_VALUE(c_ptr->_str.back(), 'X');

			container.remove_if([](const auto & elem) { return elem->to_garbage; });
			GCL_TEST__EXPECT_VALUE(container.get().size(), 0);
			GCL_TEST__EXPECT_VALUE(container.get<B>().size(), 0);
			GCL_TEST__EXPECT_VALUE(container.get<C>().size(), 0);
		}
	};
}
