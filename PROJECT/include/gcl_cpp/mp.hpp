#ifndef GCL_TMP_H_
# define GCL_TMP_H_

#include <gcl_cpp/type_index.hpp>
#include <array>

namespace gcl
{
	namespace mp
	{	// C++17
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
	}

    namespace deprecated::mp
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
				for_each<T...>::call<T_Functor>();
			}
			template <template <typename> class T_Functor, size_t N = 0>
			static void	call_at(const size_t pos)
			{
				if (N == pos)	T_Functor<T0>::call();
				else			for_each<T...>::call_at<T_Functor, (N + 1)>(pos);
			}
			template <template <typename> class T_Functor, size_t N = 0>
			static typename T_Functor<T0>::return_type	call_at_with_return_value(const size_t pos)
			{
				if (N == pos)	return T_Functor<T0>::call();
				else			return for_each<T...>::call_at_with_return_value<T_Functor, (N + 1)>(pos);
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
}

#endif // GCL_TMP_H_
