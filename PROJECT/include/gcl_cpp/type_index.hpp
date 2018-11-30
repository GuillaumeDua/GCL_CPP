#ifndef GCL_TYPE_INDEX_HPP__
# define GCL_TYPE_INDEX_HPP__

#include <gcl_cpp/mp.hpp>
#include <gcl_cpp/functionnal.hpp>

#include <tuple>
#include <functional>
#include <type_traits>

namespace gcl
{
	template <class t_interface, class ... ts_impl>
	struct type_indexer
	{	// C++17
		static_assert(std::conjunction_v<std::is_base_of<t_interface, ts_impl>...>);
		static_assert(std::conjunction_v<std::is_default_constructible<ts_impl>...>);

		using interface_type = t_interface;
		using children_type = std::tuple<ts_impl...>;

		using constructor_type = gcl::functionnal::cx::function<t_interface*()>;
		using content_type = std::array<constructor_type, sizeof...(ts_impl)>;

		auto generate(std::size_t index)
		{
			if (index >= sizeof...(ts_impl))
				throw std::out_of_range("type_indexer::generate");
			auto & generator = constructors.at(index);
			return generator();
		}
		template <std::size_t index, typename ... args_type>
		auto generate(args_type && ... args)
			-> interface_type*
		{
			using type = std::decay_t<decltype(std::get<index>(std::tuple<ts_impl...>{})) > ;
			return new type(std::forward<args_type>(args)...);
		}
		template <typename T, typename ... args_type>
		auto generate(args_type && ... args)
			-> interface_type*
		{
			static_assert(gcl::mp::contains<T, ts_impl...>);
			return new T(std::forward<args_type>(args)...);
		}

		template <typename T, typename ... args_type>
		constexpr static auto constructor()
			-> constructor_type
		{
			return constructor_type
			{
				[](args_type ... args) -> interface_type * { return new T(args...); }
			};
		}
		template <typename T>
		constexpr static constructor_type constructor_v = constructor<T>();

		constexpr static inline const content_type constructors
		{
			constructor<ts_impl>()...
		};
	};
}

# include <gcl_cpp/type_info.hpp>

# include <vector>
# include <unordered_map>
# include <functional>
# include <string>

namespace gcl::deprecated::type_index
{	// static-dynamique bridge for type index
	template <typename t_interface>
	struct interface_is
	{	// C++14
		using interface_t = t_interface;
		using index_type = size_t;

		struct type_helper
		{
			using default_constructor_t = std::function<interface_t*(void)>;

			template <class T>
			struct get_default_constructor_t
			{
				static_assert(std::is_default_constructible<T>::value, "gcl::type_index::interface_is<I>::get_default_constructor_t<T>");
				static const default_constructor_t value;
			};
			const default_constructor_t & default_constructor;
		};
		using basic_container_type = typename std::unordered_map<index_type, type_helper>;

		template <typename ... Types>
		struct of_types
		{
			using pack_t = typename gcl::type_info::variadic_template<Types...>;

			template <typename T>
			struct element : basic_container_type::value_type
			{
				static_assert(std::is_base_of<interface_t, T>::value, "gcl::type_index::interface_is<I>::of_types<...T>::element<T>");
				element()
					: basic_container_type::value_type{ pack_t::template index_of<T>, { std::ref(type_helper::template get_default_constructor_t<T>::value) } }
				{}
			};

			struct indexer
				: public std::unordered_map<index_type, typename basic_container_type::mapped_type>
			{
				explicit indexer()
					: basic_container_type{ element<Types>()... }
				{}

				template <typename T>
				static inline constexpr index_type index_of(void)
				{
					return pack_t::template index_of<T>;
				}
				template <typename T>
				inline typename basic_container_type::mapped_type at(void) const
				{
					return basic_container_type::at(index_of<T>());
				}
			} /*static index*/; // [TODO]::[FixMe] : How to duplicate pack expansion [?]
			/*template <typename T_Interface>
			template <typename ... Types>
			typename interface_is<T_Interface>::of_types<Types...>::indexer interface_is<T_Interface>::of_types<Types...>::index;*/
			static const basic_container_type index;
		};
	};
}

#endif // GCL_TYPE_INDEX_HPP__
