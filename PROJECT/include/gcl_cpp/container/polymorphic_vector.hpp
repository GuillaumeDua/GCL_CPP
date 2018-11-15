#ifndef GCL_CONTAINER__POLYMORPHIC_VECTOR_HPP__
# define GCL_CONTAINER__POLYMORPHIC_VECTOR_HPP__

#include <gcl_cpp/type_info.hpp>

#include <unordered_map>
#include <vector>
#include <functional>
#include <any>
#include <typeinfo>
#include <typeindex>

namespace gcl::container
{
	struct polymorphic_vector
	{	// C++17
		// ordered container with type erasure, and type-grouped operations
		// e.g for (auto && elem : polymorphic_vector::get<my_type>())

		using value_type = std::any;
		using value_type_ptr = std::unique_ptr<value_type>;
		using value_type_raw_ptr = value_type * ;

		template <typename ... values_t>
		polymorphic_vector(values_t && ... values)
			// : content(sizeof...(values_t)) + std::generate
		{
			(push_back(std::forward<values_t>(values)), ...);
		}

		template <typename T, typename ... Args>
		T & emplace_back(Args&&... args)
		{
			static_assert((std::is_constructible_v<T, Args&&> && ...));
			push_back(std::move(T{ std::forward<Args>(args)... }));
			return std::any_cast<T&>(*(content.back().get()));
		}
		template <typename T>
		void push_back(T && obj)
		{
			content.push_back(std::make_unique<std::any>(std::forward<T>(obj)));
			auto & elem = content.back();
			register_type(elem);
		}

		template <typename T>
		inline auto const & get() const
		{
			return content_sorted_accessor.at(typeid(T));
		}
		inline auto const & get(const std::type_index & index) const
		{
			return content_sorted_accessor.at(index);
		}
		inline auto const & get() const
		{
			return content;
		}

		void visit(std::function<void(const value_type &)> func) const
		{
			for (const auto & elem : content)
			{
				func(*elem);
			}
		}
		void visit(std::function<void(value_type &)> func)
		{
			for (auto & elem : content)
			{
#ifdef _DEBUG // enforce type safety for debug only
				const std::type_index type_id = elem->type();
				func(*elem);
				if (std::type_index{ elem->type() } != type_id)
					throw std::bad_typeid();
#else
				func(*elem);
#endif
			}
		}
		template <typename T>
		void visit(std::function<void(const T &)> func) const
		{
			for (const auto & elem : content_sorted_accessor.at(typeid(T)))
			{
				func(std::any_cast<const T&>(*elem));
			}
		}
		template <typename T>
		void visit(std::function<void(T &)> func)
		{
			for (auto & elem : content_sorted_accessor.at(typeid(T)))
			{
				func(std::any_cast<T&>(*elem));
			}
		}

		template <typename T>
		void remove(const T & value)
		{
			auto & container = content_sorted_accessor.at(typeid(T));
			auto container_remover = [&value](const std::decay_t<decltype(container)>::value_type & elem)
			{
				return std::any_cast<const T&>(*elem) == value;
			};
			container.erase
			(
				std::remove_if(std::begin(container), std::end(container), container_remover),
				std::end(container)
			);
			auto content_remover = [&value, type_id = typeid(T)](const std::decay_t<decltype(container)>::value_type & elem)
			{
				return elem->type() == type_id && std::any_cast<const T&>(*elem) == value;
			};
			content.erase
			(
				std::remove_if(std::begin(content), std::end(content), content_remover),
				std::end(content)
			);
		}
		template <typename T>
		void remove()
		{
			auto & container = content_sorted_accessor.at(typeid(T));
			container.clear();
			auto remover = [](const std::decay_t<decltype(content)>::value_type & elem)
			{
				return elem->type() == typeid(T);
			};
			content.erase
			(
				std::remove_if(std::begin(content), std::end(content), remover),
				std::end(content)
			);
		}

		void remove_if(std::function<bool(const value_type &)> func)
		{
			auto content_sorted_accessor_remover = [&func](const value_type_raw_ptr & elem)
			{
				return func(*elem);
			};

			for (auto & content_accessor : content_sorted_accessor)
			{
				content_accessor.second.erase
				(
					std::remove_if(std::begin(content_accessor.second), std::end(content_accessor.second), content_sorted_accessor_remover),
					std::end(content_accessor.second)
				);
			}

			auto content_remover = [&func](const value_type_ptr & elem)
			{
				return func(*elem);
			};

			content.erase
			(
				std::remove_if(std::begin(content), std::end(content), content_remover),
				std::end(content)
			);
		}
		template <typename T>
		void remove_if(std::function<bool(const T &)> func)
		{
			auto & container = content_sorted_accessor.at(typeid(T));
			auto container_remover = [&func](const std::decay_t<decltype(container)>::value_type & elem)
			{
				return func(std::any_cast<const T&>(*elem));
			};
			container.erase
			(
				std::remove_if(std::begin(container), std::end(container), container_remover),
				std::end(container)
			);

			auto content_remover = [&func, type_id = typeid(T)](const std::decay_t<decltype(content)>::value_type & elem)
			{
				return elem->type == type_id && func(std::any_cast<const T&>(*elem));
			};
			content.erase
			(
				std::remove_if(std::begin(content), std::end(content), remover),
				std::end(content)
			);
		}

	protected:

		template <typename T>
		void register_type(const value_type_ptr & value, const std::type_index & type = typeid(T))
		{
			content_sorted_accessor[type].emplace_back(value.get());
		}
		void register_type(const value_type_ptr & value)
		{
			content_sorted_accessor[value->type()].emplace_back(value.get());
		}

		private:

		using content_t = std::vector<value_type_ptr>;
		using content_sorted_per_type_t = std::unordered_map<std::type_index, std::vector<value_type_raw_ptr> >;

		content_t					content;
		content_sorted_per_type_t	content_sorted_accessor;
	};
}

namespace gcl::deprecated::container
{	// C++11
	template <typename interface_t>
	struct polymorphic_vector
	{	// vector with type abstraction, and type operations
		// e.g for (auto && elem : polymorphic_vector<interface_t>::get<impl_1>())

		using value_type = interface_t;
		using value_holder_t = gcl::deprecated::type_info::holder<value_type>;
		using element_t = std::unique_ptr<value_type>;

		template <typename T>
		void push_back(std::unique_ptr<T> && obj)
		{
			static_assert(std::is_base_of<value_type, T>::value, "T does not derive from value_type");
			content_sorted_accessor[gcl::deprecated::type_info::id<T>::value].emplace_back(obj.get());
			content.push_back(std::forward<std::unique_ptr<T>>(obj)); // interface_t ?
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
			auto elem{ std::make_unique<T>(std::forward<Args>(args)...) };
			push_back(std::move(elem));
			return static_cast<T&>(*(content.back().get()));
		}

		template <typename T>
		inline auto const & get()
		{
			// static_assert(std::is_base_of<value_type, T>::value, "T does not derive from value_type");
			return content_sorted_accessor.at(gcl::deprecated::type_info::id<T>::value);
		}
		inline auto const & get(gcl::deprecated::type_info::id_type id)
		{
			return content_sorted_accessor.at(id);
		}
		inline auto const & get()
		{
			return content;
		}

		void visit(std::function<void(const element_t &)> func) const
		{
			for (const auto & elem : content)
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
			for (const auto & elem : content_sorted_accessor.at(gcl::deprecated::type_info::id<T>::value))
			{
				func(*elem);
			}
		}
		template <typename T>
		void visit(std::function<void(interface_t &)> func)
		{
			static_assert(std::is_base_of<value_type, T>::value, "T does not derive from value_type");
			for (auto & elem : content_sorted_accessor.at(gcl::deprecated::type_info::id<T>::value))
			{
				func(*elem);
			}
		}
		void visit(std::function<void(const interface_t &)> func, gcl::deprecated::type_info::id_type id) const
		{
			for (const auto & elem : content_sorted_accessor.at(id))
			{
				func(*elem);
			}
		}
		void visit(std::function<void(interface_t &)> func, gcl::deprecated::type_info::id_type id)
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

		//private:
		using content_t = std::vector<element_t>;
		using content_sorted_per_type_t = std::unordered_map<gcl::deprecated::type_info::id_type, std::vector<interface_t*> >;

		content_t					content;
		content_sorted_per_type_t	content_sorted_accessor;
	};
}

#endif // GCL_CONTAINER__POLYMORPHIC_VECTOR_HPP__