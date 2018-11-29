#pragma once

#include <array>
#include <bitset>

namespace gcl
{
	class mp
	{	// C++17
		template <typename T, typename T_it = void, typename... ts>
		static constexpr std::size_t index_of_impl()
		{
			if constexpr (std::is_same_v<T, T_it>)
				return 0;
			if constexpr (sizeof...(ts) == 0)
				throw 0; // "index_of : no match found";
			return 1 + index_of_impl<T, ts...>();
		}
		template <std::size_t remain_index, typename T, typename ... ts, std::size_t ...indexes>
		constexpr static bool is_unique_impl(std::index_sequence<indexes...>)
		{
			return not contains
			<
				T,
				gcl::mp::type_at<remain_index + indexes, ts...> ...
			>;
		}
		template <typename ... ts, std::size_t ... indexes>
		constexpr static bool are_unique_impl(std::index_sequence<indexes...>)
		{
			return (gcl::mp::is_unique_v<gcl::mp::type_at<indexes, ts...>, ts...> && ...);
		}

	public:

		template <typename ... ts>
		struct type_pack
		{	// constexpr type that has variadic parameter
			// use this instead of std::tuple for viariadic-as-std-tuple-parameter,
			// if your optimization level does not skip unused variables
		};

		template <typename ... ts>
		struct super : ts...
		{};

		template <template <typename...> class trait_type, typename ... ts>
		struct partial_template
		{
			template <typename ... us>
			using type = trait_type<ts..., us...>;

			template <typename ... us>
			static constexpr bool value = trait_type<ts..., us...>::value;
		};

		template <typename T, template <typename> class ... traits>
		constexpr static bool meet_requirement = (traits<T>::value && ...);

		template <template <typename> class ... constraint_type>
		struct require
		{
			template <typename T>
			static constexpr void on()
			{
				(check_constraint<constraint_type, T>(), ...);
			}

			template <typename T>
			static inline constexpr
				std::array<bool, sizeof...(constraint_type)>
				values_on{ std::move(constraint_type<T>::value)... };

		private:
			template <template <typename> class constraint_type, typename T>
			static constexpr void check_constraint()
			{
				static_assert(constraint_type<T>::value, "constraint failed to apply. see template context for more infos");
			}
		};

		template <typename T, typename ... ts>
		static constexpr inline bool contains = std::disjunction<std::is_same<T, ts>...>::value;

		template <typename to_find, typename ... ts>
		constexpr auto get_index()
		{
			return index_of<to_find, ts...>;
			// [C++20] constexpr => std::count, std::find
			/*static_assert(contains<to_find, ts...>);

			constexpr auto result = gcl::mp::require
			<
				gcl::mp::partial_template<std::is_same, ts>::type
				...
			>::values_on<to_find>;

			auto count = std::count(std::cbegin(result), std::cend(result), true);
			if (count > 1)
				throw std::runtime_error("get_index : duplicate type");
			if (count == 0)
				throw std::out_of_range("get_index : no match");

			return std::distance
			(
				std::cbegin(result),
				std::find(std::cbegin(result), std::cend(result), true)
			);*/
		}

		// C++17 constexpr index_of. Use recursion. remove when C++20 is ready. see get_index comments for more infos.
		template <typename T, typename ...ts>
		static constexpr inline auto index_of = index_of_impl<T, ts...>();

		template <size_t N, typename ...ts>
		// better than std::decay_t<decltype(std::get<index>(std::tuple<ts...>{}))>
		// because one or more types in ts... may not be default constructible
		using type_at = typename std::tuple_element<N, std::tuple<ts...>>::type;

		template <typename T, typename ... ts>
		constexpr static bool is_unique()
		{
			constexpr std::size_t remain_index = gcl::mp::index_of<T, ts...> + 1;
			return is_unique_impl<remain_index, T, ts...>(std::make_index_sequence<sizeof...(ts) - remain_index>());
		}
		template <typename T, typename ... ts>
		constexpr static bool inline is_unique_v = is_unique<T, ts...>();

		template <typename ... ts>
		constexpr static inline bool are_unique = are_unique_impl<ts...>(std::make_index_sequence<sizeof...(ts)>());

		struct filter
		{	// allow filtering operation on variadic type,
			// using gcl::mp::contains and std::bitset
			// or_as_bitset impl is :
			// { T0, T1, T2 } | {T1, T4, T5} => 010

			using std_bitset_initializer_type = unsigned long long; // not type-alias in std::bitset for constexpr constructor parameter
			static_assert(std::is_constructible_v<std::bitset<8>, std_bitset_initializer_type>);

			template <typename ... ts, typename ... us>
			constexpr static auto as_bitset_initializer(type_pack<ts...>, type_pack<us...>)
			{
				std_bitset_initializer_type value{ 0 };
				return ~
				(
					((value |= gcl::mp::contains<ts, us...>) << 1),
					...
				);
			}
			template <typename ... ts, typename ... us>
			constexpr static auto as_bitset(type_pack<ts...>, type_pack<us...>)
			{
				const auto initializer = as_bitset_initializer(type_pack<ts...>{}, type_pack<us...>{});
				return std::bitset<sizeof...(ts)>{initializer};
			}
		};
	};
}

namespace gcl::deprecated::mp
{	// C++98
	template <class T, class ... T_Classes>
	struct super
	{
		struct Type : T, super<T_Classes...>::Type
		{};
	};
	template <class T>
	struct super<T>
	{
		using Type = T;
	};

	// constexpr if
	template <bool condition, typename _THEN, typename _ELSE>	struct IF
	{};
	template <typename _THEN, typename _ELSE>					struct IF<true, _THEN, _ELSE>
	{
		using _Type = _THEN;
	};
	template <typename _THEN, typename _ELSE>					struct IF<false, _THEN, _ELSE>
	{
		using _Type = _ELSE;
	};

	struct out_of_range {};
	template <size_t N_id = 0>
	struct list
	{
		static constexpr size_t id = N_id;

		using type_t = list<id>;
		using next = list<id + 1>;
		using previous = mp::IF<(id == 0), out_of_range, list<id - 1> >;
		constexpr static const bool is_head = mp::IF<(id == 0), true, false>;
	};

	template <template <typename> class T_trait>
	struct apply_trait
	{
		template <typename T>
		static constexpr bool value = T_trait<T>::value;
	};

	template <template <typename> class T_Constraint>
	struct require
	{
		template <typename T>
		static constexpr void on()
		{
			static_assert(T_Constraint<T>::value, "gcl::mp::apply_constraint : constraint not matched");
		}
	};

	template <typename ... T>				struct for_each;
	template <typename T0, typename ... T>	struct for_each<T0, T...>
	{
		for_each() = delete;
		for_each(const for_each &) = delete;
		for_each(const for_each &&) = delete;

		template <template <typename> class T_Constraint>
		struct require
		{
			T_Constraint<T0> _check; // Check by generation, not value
			typename for_each<T...>::template require<T_Constraint> _next;
		};
		template <template <typename> class T_Functor>
		static void	call(void)
		{
			T_Functor<T0>::call();
			for_each<T...>::template call<T_Functor>();
		}
		template <template <typename> class T_Functor, size_t N = 0>
		static void	call_at(const size_t pos)
		{
			if (N == pos)	T_Functor<T0>::call();
			else			for_each<T...>::template call_at<T_Functor, (N + 1)>(pos);
		}
		template <template <typename> class T_Functor, size_t N = 0>
		static typename T_Functor<T0>::return_type	call_at_with_return_value(const size_t pos)
		{
			if (N == pos)	return T_Functor<T0>::call();
			else			return for_each<T...>::template call_at_with_return_value<T_Functor, (N + 1)>(pos);
		}
	};
	template <>								struct for_each<>
	{
		for_each() = delete;
		for_each(const for_each &) = delete;
		for_each(const for_each &&) = delete;

		template <template <typename> class T_Constraint>
		struct require
		{};
		template <template <typename> class T_Functor>
		static void	call(void)
		{}
		template <template <typename> class T_Functor, size_t N = 0>
		static void	call_at(const size_t pos)
		{
			throw std::out_of_range("template <typename ... T> struct for_each::call_at");
		}
		template <template <typename> class T_Functor, size_t N = 0>
		static typename T_Functor<void>::return_type	call_at_with_return_value(const size_t pos)
		{
			throw std::out_of_range("template <typename ... T> struct for_each::call_at_with_return_value");
		}
	};
}

