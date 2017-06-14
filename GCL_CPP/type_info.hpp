#ifndef GCL_TUPLE_INFO__
# define GCL_TUPLE_INFO__

#include <tuple>
#include <memory>
#include <functional>
#include <cassert>

#include <gcl_cpp/preprocessor.hpp>

namespace gcl
{
	namespace type_info
	{
		using id_type = uint32_t;

		template <typename ... Ts>
		struct tuple
		{
			GCL_PREPROCESSOR__NOT_INSTANTIABLE(tuple)

			using _Types = std::tuple<Ts...>;

			template <typename T>
			static constexpr inline size_t indexOf(void)
			{
				return indexOf_impl<T, 0, Ts...>();
			}

			template <size_t N>
			using type_at = typename std::tuple_element<N, _Types>::type;

		private:
			template <typename T_Needle, size_t It, typename T_It, typename ... T_HayStack>
			static constexpr inline size_t indexOf_impl(void)
			{
				return (std::is_same<T_Needle, T_It>::value ? It : indexOf_impl<T_Needle, It + 1, T_HayStack...>());
			}
			template <typename T_Needle, size_t It>
			static constexpr inline size_t indexOf_impl(void)
			{
				throw std::out_of_range("tuple::indexOf_impl : Not found");
			}
		};
		template <typename ... Ts>
		using pack = tuple<Ts...>;

		template <typename T>
		struct id
		{
			static const id_type value; // [todo]::[C++14] : MS compiler (VS2015) do not support variable-template
		};

		template <typename interface_t>
		struct holder final
		{
			using value_type = std::unique_ptr<interface_t>;

			holder() = delete;
			holder(holder &) = default;
			holder(holder &&) = default;

			holder(id_type _id, value_type && ptr)
				: value(std::move(ptr))
				, id(id)
			{}

			template <typename concret_t>
			holder(std::unique_ptr<concret_t> && ptr)
				: value(std::forward<std::unique_ptr<concret_t>>(ptr))
				, id(type_info::id<concret_t>::value)
			{
				check_<concret_t>();
			}
			template <typename concret_t>
			holder(concret_t * ptr)
				: value(std::move(ptr))
				, id(type_info::id<concret_t>::value)
			{
				check_<concret_t>();
			}

			const id_type id;
			const value_type value;

		private :
			template <typename concret_t>
			static constexpr void check_()
			{
				static_assert(!std::is_same<interface_t, concret_t>::value, "interface and concrete types are the same");
				static_assert(std::is_base_of<interface_t, concret_t>::value, "interface_t is not base of concrete_t");
			}
		};

		namespace experimental
		{
			struct any
			{
				any(const any &) = delete;
				any(const any &&) = delete;
				virtual inline ~any() = 0 {}

				virtual inline const type_info::id_type id() const = 0;

				inline bool operator==(const any & other)
				{
					return other.id() == id() && compare_impl(other);
				}
			protected:
				virtual inline const bool compare_impl(const any &) const = 0;

				any() = default;
			};

			template <typename T>
			struct any_impl : public any, public T
			{
				friend any;
				using type_t = any_impl<T>;

				template <typename ... Args>
				any_impl(Args... args)
					: any()
					, T(std::forward<std::decay<Args...>::type>(args...))
				{}
				~any_impl() override {}

				operator T()
				{
					return static_cast<T&>(*this);
				}

				inline const type_info::id_type id() const
				{
					return gcl::type_info::id<T>::value;
				}
				inline const bool compare_impl(const any & other) const
				{
					assert(id() == other.id());
					return (dynamic_cast<const T &>(dynamic_cast<const type_t &>(other)) == dynamic_cast<const T&>(*this));
				}
			};
		}
	}

#pragma warning (disable : 4311 4302)
	template <typename T>
	const type_info::id_type type_info::id<T>::value = reinterpret_cast<type_info::id_type>(&(type_info::id<T>::value));
#pragma warning (default : 4311 4302)
}

#endif // GCL_TUPLE_INFO__
