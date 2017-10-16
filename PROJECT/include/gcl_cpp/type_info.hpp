#ifndef GCL_TUPLE_INFO__
# define GCL_TUPLE_INFO__

#include <gcl_cpp/preprocessor.hpp>

#include <tuple>
#include <memory>
#include <functional>
#include <cassert>
#include <vector>

namespace gcl
{
	namespace type_info
	{
		using id_type = uint32_t;

		template <typename ... Ts>
		struct tuple
		{
			GCL_PREPROCESSOR__NOT_INSTANTIABLE(tuple)

			using types_t = std::tuple<Ts...>;

			template <typename T>
			static constexpr inline size_t indexOf(void)
			{
				return indexOf_impl<T, 0, Ts...>();
			}

			template <size_t N>
			using type_at = typename std::tuple_element<N, types_t>::type;

		private:
			template <typename needle_t, size_t it_n, typename iterated_t, typename ... haystack_t>
			static constexpr inline size_t indexOf_impl(void)
			{
				return (std::is_same<needle_t, iterated_t>::value ? it_n : indexOf_impl<needle_t, it_n + 1, haystack_t...>());
			}
			template <typename needle_t, size_t iterated_t>
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
			// Warning : Values change per program instance
			static const id_type value; // [todo]::[C++14] : MS compiler (VS2015) do not support variable-template
										// [todo]::[C++17] : seems to no be able to self-reference a template variable
		};

		template <typename T>
		constexpr id_type value = &value<T>; // bad : won't compile as &value<T> is not a constant expression

		template<typename... T>
		static constexpr std::vector<id_type> make_ids_vector() { return { id<T>::value... }; }
		template<typename... T>
		static constexpr std::array<id_type, sizeof...(T)> make_ids_array() { return { id<T>::value... }; }

		template <typename interface_t>
		struct holder final
		{
			using value_type = std::unique_ptr<interface_t>;

			holder() = delete;
			holder(holder &) = default;
			holder(holder &&) = default;

			holder(id_type _id, value_type && ptr)
				: value(std::move(ptr))
				, id(_id)
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
			template <typename T>
			constexpr std::string_view type_hash(/*no parameters allowed*/)
			{
#ifdef _MSC_VER
				std::string_view str_view = __FUNCSIG__;

				str_view.remove_prefix(str_view.find(__FUNCTION__) + sizeof(__FUNCTION__));
				str_view.remove_suffix(str_view.length() - str_view.rfind('>'));
#elif defined (__GNUC__)
				std::string_view str_view = __PRETTY_FUNCTION__;

				str_view.remove_prefix(str_view.find(__FUNCTION__) + sizeof(__FUNCTION__));
				const char prefix[] = " [with T = ";
				str_view.remove_prefix(str_view.find(prefix) + sizeof(prefix) - 1);
				str_view.remove_suffix(str_view.length() - str_view.find(';'));
#else
				static_assert(false, "not supported compiler");
#endif
				return str_view;
			}

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
