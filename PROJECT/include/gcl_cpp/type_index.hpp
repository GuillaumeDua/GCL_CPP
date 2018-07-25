#ifndef GCL_TYPE_INDEX_HPP__
# define GCL_TYPE_INDEX_HPP__

# include <gcl_cpp/static_introspection.hpp> // append to current file
# include <gcl_cpp/preprocessor.hpp>
# include <gcl_cpp/type_info.hpp>

# include <vector>
# include <unordered_map>
# include <functional>
# include <iostream>
# include <string>

namespace gcl
{
	namespace type_index
	{
		// static-dynamique bridge for type index
		template <typename t_interface>
		struct interface_is
		{
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
				using pack_t = gcl::type_info::tuple<Types...>;

				template <typename T>
				struct element : basic_container_type::value_type
				{
					static_assert(std::is_base_of<interface_t, T>::value, "gcl::type_index::interface_is<I>::of_types<...T>::element<T>");
					element()
						: basic_container_type::value_type{ pack_t::template index_of<T>(), { std::ref(type_helper::get_default_constructor_t<T>::value) } }
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
						return pack_t::template index_of<T>();
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
}

#endif // GCL_TYPE_INDEX_HPP__
