#ifndef GCL_TYPE_INFO__
# define GCL_TYPE_INFO__

#include <gcl_cpp/preprocessor.hpp>

#include <tuple>
#include <memory>
#include <functional>
#include <cassert>
#include <vector>
#include <typeindex>

#include <gcl_cpp/mp.hpp>
#include <gcl_cpp/tuple_utils.hpp>

namespace gcl::type_info
{
	namespace id
	{
		using type = std::type_index;
#if defined(__cpp_rtti)
		template <typename T>
		static const type value = typeid(T);
#endif
		template <typename T>
		static constexpr auto type_name = experimental::type_name<T>();
	}

	template<typename ... Ts>
	static constexpr std::vector<id::type> make_ids_vector() { return { id<Ts>::value... }; }
	template<typename ... Ts>
	static constexpr std::array<id::type, sizeof...(Ts)> make_ids_array() { return { id<T>::value... }; }

	template <typename ... Ts>
	struct variadic_template
	{
		using std_type = std::tuple<Ts...>;

		inline static constexpr auto size = sizeof...(Ts); // or std::tuple_size_v<tuple_type>

		template <size_t N>
		using type_at = typename std::tuple_element<N, std_type>::type;

		template <typename T>
		static constexpr inline size_t index_of = gcl::mp::index_of<T, Ts...>;

		template <typename T>
		static constexpr inline bool contains = gcl::mp::contains<T, Ts...>;

		auto to_std()
		{
			return std_type{};
		}
	};
	template <typename ... Ts>
	using pack = variadic_template<Ts...>;

	namespace experimental
	{
		template <typename T>
		constexpr std::string_view type_name(/*no parameters allowed*/)
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

		struct holder final
		{	// hold a value anonymously
			// /!\ cost much more than std::any
			struct interface_t
			{	// as interface, for safe y-shaped inheritance tree (type erasure)
				interface_t() = default;
				interface_t(const interface_t &) = delete;
				interface_t(interface_t &&) = delete;
				virtual ~interface_t() = default;
			};
			using value_type = std::unique_ptr<interface_t>;
			template <typename T>
			struct type_eraser : interface_t, std::decay_t<T>
			{
				using target_type = std::decay_t<T>;

				explicit type_eraser(target_type && value)
					: target_type{ std::forward<target_type>(value) }
				{}
			};

			holder() = delete;
			holder(holder &) = delete;
			holder(holder &&) = default;

			template <typename concret_t>
			explicit holder(concret_t && elem)
				: value(std::unique_ptr<interface_t>(new type_eraser<concret_t>{ std::forward<concret_t>(elem) }))
				, id(type_info::id::value<concret_t>)
			{}

			template <typename T>
			T & cast() const
			{
				using target_type = std::decay_t<T>;

				if (id != gcl::type_info::id::value<target_type>)
					throw std::bad_cast();

				auto * as_type_eraser_ptr = static_cast<type_eraser<target_type&>*>(value.get());
				return static_cast<target_type&>(*as_type_eraser_ptr);
			}

			const gcl::type_info::id::type id;
			const value_type value;
		};
	}
}

inline std::ostream & operator<<(std::ostream & os, const std::type_index & index)
{
	return os << "type_id=[(" << index.hash_code() << ')' << index.name() << ']';
}

#include <memory>

namespace gcl::deprecated::type_info
{	//	C++11
	template <typename ... Ts>
	struct variadic_template
	{
		using tuple_type = std::tuple<Ts...>;

		inline static constexpr auto size = sizeof...(Ts); // or std::tuple_size_v<tuple_type>

		template <size_t N>
		using type_at = typename std::tuple_element<N, tuple_type>::type;

		template <typename T>
		static constexpr inline size_t index_of(void)
		{
			return index_of_impl<T, 0, Ts...>();
		}

		auto to_std()
		{
			return tuple_type{};
		}

	private:
		template <typename needle_t, size_t it_n, typename iterated_t, typename ... haystack_t>
		static constexpr inline size_t index_of_impl(void)
		{
			return (std::is_same<needle_t, iterated_t>::value ? it_n : index_of_impl<needle_t, it_n + 1, haystack_t...>());
		}
		template <typename needle_t, size_t iterated_t>
		static constexpr inline size_t index_of_impl(void)
		{
			throw std::out_of_range("variadic_template::index_of_impl : Not found");
		}
};

#if (defined _DEBUG && defined GCL_UNSAFE_CODE)
	using id_type = uint64_t;
	template <typename T>
	struct id
	{
		// Warning : Values change per program instance
		// Error   : compiler optimization may set the same value per instance
		static const id_type value; // [todo]::[C++14] : MS compiler (VS2015) do not support variable-template
									// [todo]::[C++17] : seems to no be able to self-reference a template variable
	};

#pragma warning (disable : 4311 4302)
	template <typename T>
	const gcl::deprecated::type_info::id_type gcl::deprecated::type_info::id<T>::value
		= reinterpret_cast<gcl::deprecated::type_info::id_type>(&(gcl::deprecated::type_info::id<T>::value));
#pragma warning (default : 4311 4302)
#else
	using id_type = std::type_index;
	template <typename T>
	struct id
	{
		static inline const id_type value = typeid(T);
	};
#endif

	template <typename interface_t>
	struct holder final
	{
		using typeid_type = id_type;
		using value_type = std::unique_ptr<interface_t>;

		holder() = delete;
		holder(holder &) = default;
		holder(holder &&) = default;

		holder(id_type _id, value_type && ptr)
			: value(std::move(ptr))
			, id(_id)
		{}

		template <typename concret_t>
		explicit holder(std::unique_ptr<concret_t> && ptr)
			: value(std::forward<std::unique_ptr<concret_t>>(ptr))
			, id(gcl::deprecated::type_info::id<concret_t>::value)
		{
			check_<concret_t>();
		}
		template <typename concret_t>
		explicit holder(concret_t * ptr)
			: value(std::move(ptr))
			, id(gcl::deprecated::type_info::id<concret_t>::value)
		{
			check_<concret_t>();
		}

		const id_type id;
		const value_type value;

	private:
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

			virtual inline const gcl::deprecated::type_info::id_type id() const = 0;

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
			explicit any_impl(Args... args)
				: any()
				, T(std::forward<std::decay<Args...>::type>(args...))
			{}
			~any_impl() override {}

			operator T()
			{
				return static_cast<T&>(*this);
			}

			inline const gcl::deprecated::type_info::id_type id() const override
			{
				return gcl::deprecated::type_info::id<T>::value;
			}
			inline const bool compare_impl(const any & other) const override
			{
				assert(id() == other.id());
				return (dynamic_cast<const T &>(dynamic_cast<const type_t &>(other)) == dynamic_cast<const T&>(*this));
			}
		};
	}
}

#endif // GCL_TYPE_INFO__
