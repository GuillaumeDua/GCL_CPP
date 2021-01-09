#pragma once
namespace gcl::container
{	// todo : impl + tests
	template <typename T>
	struct linear_vector
	{	// vector wrapper where elements have constant memory places
		using element_type = std::unique_ptr<T>;
		using element_place_holder = std::reference_wrapper<element_type>;

		template <typename ...arg_types>
		auto add(arg_types && ... args)
		{
			static_assert(std::is_constructible_v<T, arg_types...>, "container<T>::add : invalid_argument");
			auto elem = std::make_unique<T>(std::forward<arg_types>(args)...);
			base_container.push_back(std::move(elem));
			return std::ref(*base_container.back());
		}

		using base_container_type = std::vector<element_type>;
		base_container_type base_container;
	};
}