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

		template <typename t_func, typename ... ts_args_1, typename ... ts_args_2>
		static void visit(t_func & func, std::tuple<ts_args_1...> & args_1, std::tuple<ts_args_2...> & args_2)
		{	// sync index visit.
			// call func `sizeof...(args_1)` times, with std::get<index>(args_1), std::get<index>(args_2);
			// example :
			// gcl::tuple_utils::visit([](auto & arg1, auto && arg2) {arg1 = std::move(arg2); }, values, arguments);
			// is equal to :
			// args_1 = args_2;
			// e.g std::tuple<ts_1...>::operator=(std::tuple<ts_2...>)

			static_assert(sizeof...(ts_args_1) == sizeof...(ts_args_2));
			using indexes_type = std::make_index_sequence<sizeof...(ts_args_1)>;
			visit_impl(func, args_1, args_2, indexes_type{});
		}
		template <typename t_func, typename ... ts_args_1, typename ... ts_args_2>
		static void visit(const t_func & func, const std::tuple<ts_args_1...> & args_1, const std::tuple<ts_args_2...> & args_2)
		{
			static_assert(sizeof...(ts_args_1) == sizeof...(ts_args_2));
			using indexes_type = std::make_index_sequence<sizeof...(ts_args_1)>;
			visit_impl(func, args_1, args_2, indexes_type{});
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

		template <typename t_func, typename ... ts_args_1, typename ... ts_args_2, std::size_t ... indexes>
		static void visit_impl(t_func & func, std::tuple<ts_args_1...> & args_1, std::tuple<ts_args_2...> & args_2, std::index_sequence<indexes...>)
		{
			(func(std::get<indexes>(args_1), std::get<indexes>(args_2)), ...);
		}
		template <typename t_func, typename ... ts_args_1, typename ... ts_args_2, std::size_t ... indexes>
		static void visit_impl(const t_func & func, const std::tuple<ts_args_1...> & args_1, const std::tuple<ts_args_2...> & args_2, std::index_sequence<indexes...>)
		{
			(func(std::get<indexes>(args_1), std::get<indexes>(args_2)), ...);
		}
	};
}