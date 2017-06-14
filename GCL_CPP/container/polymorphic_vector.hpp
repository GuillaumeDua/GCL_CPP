#ifndef GCL_CONTAINER__POLYMORPHIC_VECTOR_HPP__
# define GCL_CONTAINER__POLYMORPHIC_VECTOR_HPP__

#include "../type_info.hpp"

#include <unordered_map>
#include <vector>
#include <functional>

namespace gcl
{
	namespace container
	{
		template <typename interface_t>
		struct polymorphic_vector
		{
			using value_type = interface_t;
			using value_holder_t = gcl::type_info::holder<value_type>;
			using element_t = std::unique_ptr<value_type>;

			template <typename T>
			void push_back(std::unique_ptr<T> && obj)
			{
				static_assert(std::is_base_of<value_type, T>::value, "T does not derive from value_type");
				content_sorted_accessor[gcl::type_info::id<T>::value].emplace_back(obj.get());
				content.push_back(std::forward<std::unique_ptr<T>>(obj));
			}
			void push_back(value_holder_t && holder)
			{
				content_sorted_accessor[holder.id].emplace_back(holder.value.get());
				content.push_back(std::move(holder.value));
			}
			template <typename T, typename ... Args>
			T & emplace_back(Args&&... args)
			{
				static_assert(std::is_base_of<value_type, T>::value, "T does not derive from value_type");
				auto elem{std::make_unique<T>(std::forward<Args>(args)...)};
				push_back(std::move(elem));
				return *elem;
			}

			template <typename T>
			inline auto const & get()
			{
				static_assert(std::is_base_of<value_type, T>::value, "T does not derive from value_type");
				return content_sorted_accessor.at(gcl::type_info::id<T>::value);
			}
			inline auto const & get(gcl::type_info::id_type id)
			{
				return content_sorted_accessor.at(id);
			}
			inline auto const & get()
			{
				return content;
			}

			void visit(std::function<void(const element_t &)> func) const
			{
				for (auto & elem : content)
				{
					func(elem);
				}
			}
			void visit(std::function<void(element_t &)> func)
			{
				for (auto & elem : content)
				{
					func(elem);
				}
			}
			template <typename T>
			void visit(std::function<void(const interface_t &)> func) const
			{
				static_assert(std::is_base_of<value_type, T>::value, "T does not derive from value_type");
				for (auto & elem : content_sorted_accessor.at(gcl::type_info::id<T>::value))
				{
					func(*elem);
				}
			}
			template <typename T>
			void visit(std::function<void(interface_t &)> func)
			{
				static_assert(std::is_base_of<value_type, T>::value, "T does not derive from value_type");
				for (auto & elem : content_sorted_accessor.at(gcl::type_info::id<T>::value))
				{
					func(*elem);
				}
			}
			void visit(std::function<void(const interface_t &)> func, gcl::type_info::id_type id) const
			{
				for (auto & elem : content_sorted_accessor.at(id))
				{
					func(*elem);
				}
			}
			void visit(std::function<void(interface_t &)> func, gcl::type_info::id_type id)
			{
				for (auto & elem : content_sorted_accessor.at(id))
				{
					func(*elem);
				}
			}

			template <class Fun> // same synthax, as unique_ptr-s overload operator->
			void remove_if(Fun func)
			{
				for (auto & content_accessor : content_sorted_accessor)
				{
					content_accessor.second.erase
					(
						std::remove_if(std::begin(content_accessor.second), std::end(content_accessor.second), func),
						std::end(content_accessor.second)
					);
				}
				content.erase
				(
					std::remove_if(std::begin(content), std::end(content), func),
					std::end(content)
				);
			}
			/*template <class T, class Fun>
			void remove_if(Fun func); // cost too much */

		protected:
			using content_t = std::vector<element_t>;
			using content_sorted_accessor_t = std::unordered_map<gcl::type_info::id_type, std::vector<interface_t*> >;

			content_t					content;
			content_sorted_accessor_t	content_sorted_accessor;
		};
	}
}

#endif // GCL_CONTAINER__POLYMORPHIC_VECTOR_HPP__