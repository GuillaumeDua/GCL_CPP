#pragma once // not using standard header-guard in order to not pollute macro completion on GCL_TEST*

# include "../../container/polymorphic_vector.hpp"
# include <cassert>

namespace gcl
{
	namespace test
	{
		struct container
		{
			static void polymorphic_vector()
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
				assert(b_ptr);
				assert(b_ptr->_i == 2);

				C * c_ptr = dynamic_cast<C*>(container.get<C>().front());
				assert(c_ptr);
				assert(c_ptr->_str.back() == 'X');

				container.remove_if([](const auto & elem) { return elem->to_garbage; });
				assert(container.get().size() == 0);
				assert(container.get<B>().size() == 0);
				assert(container.get<C>().size() == 0);
			}

			static void proceed()
			{
				polymorphic_vector();
			}
		};
	}
}
