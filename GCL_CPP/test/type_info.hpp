#include "../type_info.hpp"

namespace gcl
{
	namespace test
	{
		struct type_info
		{
			struct experimental
			{
				static void any()
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
						uint32_t value_ = 42;
					};

					std::unique_ptr<gcl::type_info::experimental::any> any_str_1(new gcl::type_info::experimental::any_impl<type_1>("hello, world"));
					std::unique_ptr<gcl::type_info::experimental::any> any_str_2(new gcl::type_info::experimental::any_impl<type_1>("hello, world 2"));
					std::unique_ptr<gcl::type_info::experimental::any> any_uint32_t_1(new gcl::type_info::experimental::any_impl<type_2>(13));
					std::unique_ptr<gcl::type_info::experimental::any> any_uint32_t_2(new gcl::type_info::experimental::any_impl<type_2>(42));

					if (
						!(*any_str_1 == *any_str_1)
						|| (*any_str_1 == *any_str_2)
						|| !(*any_uint32_t_1 == *any_uint32_t_1)
						|| (*any_uint32_t_1 == *any_uint32_t_2)
						|| (*any_uint32_t_1 == *any_str_1))
						throw std::logic_error("gcl::any::operator==(const gcl::any &)");
				}
				static void proceed()
				{
					any();
				}
			};
			static void proceed()
			{
			}
		};
	}
}