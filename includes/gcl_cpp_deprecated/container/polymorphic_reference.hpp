#pragma once

#include <functional>
#include <typeindex>

namespace gcl::container
{
	struct polymorphic_reference
	{	// basically, a mix between a std::reference_wrapper and std::any
		// No ressource acquisition,
		// thus it does not store nor extend life-time of value;

		polymorphic_reference(const polymorphic_reference &) = default;
		polymorphic_reference(polymorphic_reference &&) = default;

		template
		<
			typename T,
			typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, std::decay_t<polymorphic_reference>>>
		>
		explicit polymorphic_reference(T & value) noexcept
			: value{ std::addressof(value) }
			, index{ typeid(T) }
		{}

		template <typename T>
		operator T&() const
		{
			static const std::type_index t_index = typeid(T);
			if (index != t_index)
				throw std::bad_cast();
			return *static_cast<T*>(value);
		}
		template <typename T>
		T & get() const
		{
			return static_cast<T&>(*this);
		}

		polymorphic_reference& operator=(const polymorphic_reference&) = default;
		polymorphic_reference& operator=(polymorphic_reference&&) = default;

		template
		<
			typename T,
			typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, std::decay_t<polymorphic_reference>>>
		>
		polymorphic_reference & operator=(const T & new_value)
		{
			static_cast<T&>(*this) = new_value;
			return *this;
		}
		template
		<
			typename T,
			typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, std::decay_t<polymorphic_reference>>>
		>
		polymorphic_reference & operator=(T && new_value)
		{
			static_cast<T&>(*this) = std::forward<T>(new_value);
			return *this;
		}

		template <class T, class ... arg_types>
		std::invoke_result_t<T&, arg_types...> operator()(arg_types && ... args) const
		{
			return invoke<T>(std::forward<arg_types>(args)...);
		}
		template <typename T, class ... arg_types>
		std::invoke_result_t<T&, arg_types...> invoke(arg_types && ... args) const
		{
			return std::invoke(get<T>(), std::forward<arg_types>(args)...);
		}

	private:
		std::type_index index;
		void * value;
	};
}
