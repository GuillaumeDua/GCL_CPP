#pragma once

#include <gcl_cpp/mp.hpp>

namespace gcl
{
	struct tuple_utils
	{
		template <size_t N, typename ... ts>
		using type_at = typename std::tuple_element<N, std::tuple<ts...>>::type;

		template <typename ... ts>
		static constexpr std::size_t size(const std::tuple<ts...> &)
		{
			return std::tuple_size_v<std::tuple<ts...>>;
		}

		template <typename to_find, typename ... Ts>
		static constexpr std::size_t index_of(const std::tuple<Ts...> &)
		{
			return gcl::mp::get_index<to_find, Ts...>();
		}

		template <typename T, typename ... ts>
		static constexpr  bool contains(const std::tuple<ts...>&)
		{
			return std::disjunction<std::is_same<T, ts>...>::value;
		}

		template <typename func_type, typename ... Ts>
		static void for_each(std::tuple<Ts...> & value, func_type func)
		{
			using tuple_type = std::decay_t<decltype(value)>;
			using indexes_type = std::make_index_sequence<std::tuple_size_v<tuple_type>>;

			for_each_impl(func, value, indexes_type{});
		}
		template <typename func_type, typename ... Ts>
		static void for_each(const std::tuple<Ts...> & value, func_type func)
		{
			using tuple_type = std::decay_t<decltype(value)>;
			using indexes_type = std::make_index_sequence<std::tuple_size_v<tuple_type>>;

			for_each_impl(func, value, indexes_type{});
		}

	private:
		template <typename func_type, typename tuple_type, std::size_t ... indexes>
		static void for_each_impl(func_type func, tuple_type & variadic_template, std::index_sequence<indexes...>)
		{
			(func(std::get<indexes>(variadic_template)), ...);
		}
		template <typename func_type, typename tuple_type, std::size_t ... indexes>
		static void for_each_impl(func_type func, const tuple_type & variadic_template, std::index_sequence<indexes...>)
		{
			(func(std::get<indexes>(variadic_template)), ...);
		}
	};
}