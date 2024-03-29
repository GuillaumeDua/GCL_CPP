#pragma once

#include <gcl_cpp/mp.hpp>

namespace gcl
{
	struct tuple_utils
	{	// wrapper to access gcl::mp::<func_name>,
		// using std::tuple<ts...> parameter for ts resolution

		template <size_t N, typename ... ts>
		using type_at = gcl::mp::type_at<N, ts...>;

		template <typename to_find, typename ... Ts>
		static constexpr std::size_t index_of(const std::tuple<Ts...> &)
		{
			return gcl::mp::get_index<to_find, Ts...>();
		}

		template <typename ... ts>
		static constexpr std::size_t size(const std::tuple<ts...> &)
		{
			return std::tuple_size_v<std::tuple<ts...>>;
		}

		template <typename T, typename ... ts>
		static constexpr bool contains(const std::tuple<ts...>&)
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

		template <typename func_type, typename ... Ts>
		static void for_each_with_index(std::tuple<Ts...> & value, func_type func)
		{	// example :
			//		gcl::tuple_utils::for_each_with_index(std::tuple<int, char, double, float>{}, [](auto index, auto & arg)
			//		{
			//			std::cout << index << " -> " << typeid(std::decay_t<decltype(arg)>).name() << '\n';
			//		});
			// may output :
			//		0 -> int
			//		1 -> char
			//		2 -> double
			//		3 -> float
			using tuple_type = std::decay_t<decltype(value)>;
			using indexes_type = std::make_index_sequence<std::tuple_size_v<tuple_type>>;

			for_each_with_index_impl(func, value, indexes_type{});
		}
		template <typename func_type, typename ... Ts>
		static void for_each_with_index(const std::tuple<Ts...> & value, func_type func)
		{
			using tuple_type = std::decay_t<decltype(value)>;
			using indexes_type = std::make_index_sequence<std::tuple_size_v<tuple_type>>;

			for_each_with_index_impl(func, value, indexes_type{});
		}

		template <typename t_func, typename ... ts_args_1, typename ... ts_args_2>
		static void visit(t_func & func, std::tuple<ts_args_1...> & args_1, std::tuple<ts_args_2...> & args_2)
		{	// sync index visit.
			// call func `sizeof...(args_1)` times, with std::get<index>(args_1), std::get<index>(args_2);
			// example :
			//		gcl::tuple_utils::visit([](auto & arg1, auto && arg2) {arg1 = std::move(arg2); }, values, arguments);
			// is equal to :
			//		args_1 = args_2;
			// e.g	std::tuple<ts_1...>::operator=(std::tuple<ts_2...>)

			static_assert(sizeof...(ts_args_1) == sizeof...(ts_args_2)); // cld be `<=` with warning "partial expansion use for `ts_args_2`"
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

		template <typename func_type, typename tuple_type, std::size_t ... indexes>
		static void for_each_with_index_impl(func_type func, tuple_type & variadic_template, std::index_sequence<indexes...>)
		{
			(func(indexes, std::get<indexes>(variadic_template)), ...);
		}

		template <typename func_type, typename tuple_type, std::size_t ... indexes>
		static void for_each_with_index_impl(func_type func, const tuple_type & variadic_template, std::index_sequence<indexes...>)
		{
			(func(indexes, std::get<indexes>(variadic_template)), ...);
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