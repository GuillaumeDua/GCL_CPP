#pragma once

#include <functional>
#include <gcl_cpp/type_info.hpp>

namespace gcl::functionnal
{
	template <class F>
	class trait
	{	// todo : multiple operator()
		using base_type = std::decay_t<F>;
		using base_operator_type = decltype(&base_type::operator());

		template <typename return_t, typename ... args_t>
		static constexpr auto deduce(return_t(base_type::*)(args_t...) const)
		{
			struct deduce_t
			{
				constexpr deduce_t() = default;
				using return_type = return_t;
				//static constexpr std::size_t args_size = sizeof...(args_t);
				using arguments_type = gcl::type_info::pack<args_t...>;
			};
			return deduce_t{};
		}

		using deducer = decltype(deduce(&base_type::operator()));

	public:
		using return_type = typename deducer::return_type;
		using arguments_type = typename deducer::arguments_type;
	};

	//template <typename T, typename = void>
	//struct is_callable : std::false_type {};
	//template <typename T>
	//struct is_callable<T, std::void_t<decltype(std::declval<T>().operator())>> : std::true_type {}; // std::decay_t

	template <typename ... Fs, typename ... args_t>
	auto combine_impl(gcl::type_info::pack<args_t...>, Fs && ... funcs)
	{
		if constexpr (sizeof...(args_t) == 0)
			return [=]()
		{
			[[maybe_unused]] int _[] = { (funcs(), 0)... };
		};
		else
			return [=](args_t ... args)
		{
			[[maybe_unused]] int _[] = { (funcs(args...), 0)... };
		};
	}
	template <typename ... Fs>
	auto combine(Fs && ... funcs)
	{
		using type = typename std::tuple_element<0, std::tuple<Fs...>>::type;
		using trait = trait<type>;

		return combine_impl(typename trait::arguments_type{}, std::forward<Fs>(funcs)...);
	}
	template <>
	auto combine() { return []() {}; }

	namespace deprecated
	{
		struct on_destroy_call : public std::function<void()>
		{
			explicit on_destroy_call(std::function<void()> && func)
			{
				func.swap(*this);
			}
			on_destroy_call(const on_destroy_call & w)
			{
				const_cast<on_destroy_call &>(w).swap(*this);
			}
			~on_destroy_call()
			{
				if (*this)
					(*this)();
			}
		};
	}
}

template
<
	class F1, class F2,
	typename std::enable_if_t
	<
		std::is_invocable_v<F1> &&
		std::is_invocable_v<F2>
		, int
	> = 0
>
static auto operator+(F1 && f1, F2 && f2)
{
	return gcl::functionnal::combine(std::forward<F1>(f1), std::forward<F2>(f2));
}

