#pragma once

#include <vector>
#include <typeindex>
#include <memory>

namespace gcl::container::component_aggregator
{	// a mix of CRTP and type erasure
	// for polymorphic usage :
	//
	// wrapper value {type<component_1, component_2>{}};

	using signature_type = std::vector<std::type_index>;

	template
	<
		template <typename> typename ... components
	>
	struct type : components<type<components...>>...
	{	// CRTP
		static inline const signature_type type_signatures{ typeid(components)... };
		signature_type const & signatures = type_signatures;

		template <template <typename> typename component>
		component<type> & as_component()
		{
			return static_cast<component<type>&>(*this);
		}
		template <template <typename> typename component>
		const component<type> & as_component() const
		{
			return static_cast<const component<type>&>(*this);
		}

		template <template <typename> typename component>
		bool has_component()
		{
			return has_component(typeid(component));
		}
		bool has_component(const std::type_index & index)
		{
			return std::find(std::begin(signatures), std::end(signatures), index) != std::end(signatures);
		}
	};

	struct wrapper
	{	// type erasure
		struct base
		{
			template <template <typename> typename component>
			bool has_component()
			{
				return has_component(typeid(component));
			}
			virtual bool has_component(const std::type_index & index) = 0;
		};
		template <typename T>
		struct impl : base, T
		{
			impl(T && value)
				: T{ std::forward<T>(value) }
			{}

			template <template <typename> typename component>
			bool has_component()
			{
				return has_component(typeid(component));
			}
			bool has_component(const std::type_index & index)
			{
				return static_cast<T&>(*this).has_component(index);
			}
		};

		template <template <typename> typename component>
		bool has_component()
		{
			return has_component(typeid(component));
		}
		bool has_component(const std::type_index & index)
		{
			return value->has_component(index);
		}

		template <typename T>
		wrapper(T && arg)
			: value{ new impl<T>{ std::forward<T>(arg) } }
		{}

		template <typename T>
		inline T & cast() noexcept
		{	// resolve type. unsafe
			auto & y_node = static_cast<impl<T>&>(*value);
			return static_cast<T&>(y_node);
		}

	private:
		std::unique_ptr<base> value;
	};
}