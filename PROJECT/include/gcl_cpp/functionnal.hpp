#pragma once

#include <functional>
#include <gcl_cpp/type_info.hpp>

namespace gcl::functionnal
{
	template <class... Ts>
	struct overload : Ts...
	{
		using Ts::operator()...;
	};
	template <class... Ts> overload(Ts...) -> overload<Ts...>;

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
	auto combine_homogeneous_impl(gcl::type_info::pack<args_t...>, Fs && ... funcs)
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
	auto combine_homogeneous(Fs && ... funcs)
	{
		using type = typename std::tuple_element<0, std::tuple<Fs...>>::type;
		using trait = trait<type>;

		// todo : static_assert(is_all<...>, "failed to combine heterogenous functions types");
		//        or constexpr if that call combine_heterogeneous
		// std::conjunction_v

		return combine_homogeneous_impl(typename trait::arguments_type{}, std::forward<Fs>(funcs)...);
	}
	template <>
	auto combine_homogeneous() { return []() {}; }

	template <typename ... Fs>
	auto combine_heterogeneous(Fs ... funcs)
	{
		struct adapter : std::decay_t<Fs>...
		{
			adapter(std::decay_t<Fs> ... values)
				: std::decay_t<Fs>(std::move(values))...
			{}

			using Fs::operator()...;
		};
		return adapter{ std::forward<Fs>(funcs)... };
	}

	// todo : combine(Fs ... funcs)
	// -> if duplicate signatures, then merge them using combine_homogeneous / operator+
	//    e.g [](int){ 1; } + [](int){ 2; } + [](int){ 3; } => [](int){ 1; 2; 3; }
	// -> then, merge all types heterogeneously combine_heterogeneous / combine_heterogeneous_t

	template<class... Ts> struct combine_heterogeneous_t : Ts... { using Ts::operator()...; };
	template<class... Ts> combine_heterogeneous_t (Ts...) -> combine_heterogeneous_t<Ts...>;

	namespace deprecated
	{
		struct finally : public std::function<void()>
		{
			explicit finally(std::function<void()> && func)
			{
				func.swap(*this);
			}
			finally(const finally & w)
			{
				const_cast<finally &>(w).swap(*this);
			}
			~finally()
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
	return gcl::functionnal::combine_homogeneous(std::forward<F1>(f1), std::forward<F2>(f2));
}

namespace gcl::functionnal::cx
{
	template <class>
	struct function;

	template <class t_return, class... ts_args>
	struct function<t_return(ts_args...)>
	{	// wrapper to constexpr function ptr
		// thus, lambda cannot have a storage (e.g capture)

		using result_type = t_return;
		using arguments_type = gcl::mp::type_pack<ts_args...>;
		using raw_type = t_return(*)(ts_args...);

		constexpr function() = default;
		constexpr function(function && func) = default;
		constexpr function(const function & func) = delete;

		constexpr function(raw_type ptr)
			: value{ ptr }
		{}

		constexpr operator bool() const
		{
			return value != nullptr;
		}
		constexpr bool operator!() const
		{
			return !static_cast<bool>(*this);
		}
		constexpr t_return operator()(ts_args ... args) const
		{
			if (!*this)
				throw std::bad_function_call();
			return (*value)(std::forward<ts_args>(args)...);
		}

	private:
		raw_type value = nullptr;
	};
}