#pragma once // not using standard header-guard in order to not pollute macro completion on GCL_TEST*

# include <gcl_cpp/container/polymorphic_vector.hpp>
# include <gcl_cpp/test.hpp>

namespace gcl
{
	namespace test
	{
		namespace container_partial_impl
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

					gcl::container::polymorphic_vector<A> container;
					using container_elem_t = gcl::container::polymorphic_vector<A>::element_t;

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
		};
	}
}
