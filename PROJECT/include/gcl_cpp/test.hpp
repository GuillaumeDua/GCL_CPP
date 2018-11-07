#pragma once // not using standard header-guard in order to not pollute macro completion on GCL_TEST*

#include <type_traits>
#include <algorithm>
#include <functional>
#include <chrono>
#include <cassert>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <iostream>


#ifndef GCL_PREPROCESSOR__NOT_INSTANTIABLE
// remove #include <gcl_cpp/preprocessor.hpp> for stand-alone
#define GCL_PREPROCESSOR__NOT_INSTANTIABLE(type) \
	type() = delete;				\
	type(const type &) = delete;	\
	type(type &&) = delete; 
#endif

namespace gcl
{
	namespace lexical_cast
	{
		template <typename T> std::string to_string(const T & var)
		{
			std::ostringstream oss;
			oss << var;
			return oss.str();
		}
		template <> std::string to_string<std::ostringstream>(const std::ostringstream & var)
		{
			return var.str();
		}
		template <> std::string to_string<std::string>(const std::string & var)
		{
			return var;
		}
	}
}


// todo : static SFINAE tests
// type_trait : test_component<name>::pass / test_component<name>::fail as constexpr booleans value
// => constexpr testing

// todo : ((always_inline)) + remove macro_invokable area

// todo : remove dependencies_t to declare tests, replace by static introspection
//        -> is constexpr ? -> constexpr testing
//        -> else           -> runtime   testing

#pragma warning(disable : 4127) // if constexpr does not work

namespace gcl
{
	namespace test
	{
		struct fail_exception : std::exception
		{
			fail_exception(const std::string & msg)
				: std::exception(msg.c_str())
			{}
			fail_exception(const char* msg)
				: std::exception(msg)
			{}
		};

		struct check
		{
			GCL_PREPROCESSOR__NOT_INSTANTIABLE(check);

			template <typename Fun>
			static inline void expect(Fun to_test, const std::string & user_msg = "")
			{
				if (!to_test())
					throw fail_exception("gcl::test::check::expect failed" + (user_msg.empty() ? "" : " : " + user_msg));
			}
			template <>
			static inline void expect<bool>(bool test_result, const std::string & user_msg)
			{
				if (!test_result)
					throw fail_exception("gcl::test::check::expect failed" + (user_msg.empty() ? "" : " : " + user_msg));
			}

			template <typename value_t, typename U>
			static inline void expect_value(const value_t & value, const U & expected_value, const std::string & user_msg = "")
			{
				if (value != expected_value)
					throw fail_exception("gcl::test::check::expect_value failed" + (user_msg.empty() ? "" : " : " + user_msg));
			}

			template <typename value_t, typename U>
			static inline void expect_not_value(const value_t & value, const U & unexpected_value, const std::string & user_msg = "")
			{
				if (value == unexpected_value)
					throw fail_exception("gcl::test::check::unexpect_value failed" + (user_msg.empty() ? "" : " : " + user_msg));
			}

			template <typename value_t>
			using value_range_t = std::pair<value_t, value_t>;

			template <typename value_t, typename expected_value_t>
			static inline void expect_value_in_range(const value_t & value, const value_range_t<expected_value_t> & expected_values_range, const std::string & user_msg = "")
			{
				assert(expected_values_range.first < expected_values_range.second);

				if (value < expected_values_range.first || value > expected_values_range.second)
					throw fail_exception("gcl::test::check::expect_value_in_range failed" + (user_msg.empty() ? "" : " : " + user_msg));
			}

			template <typename value_t, typename expected_value_t, typename Fun>
			static inline void expect_values(const value_t & value, const expected_value_t & expected_value_before, Fun what, const expected_value_t & expected_value_after, const std::string & user_msg = "")
			{
				if (value != expected_value_before)
					throw fail_exception("gcl::test::check::expect_values failed" + (user_msg.empty() ? std::string{} : " : " + user_msg));
				what();
				if (value != expected_value_after)
					throw fail_exception("gcl::test::check::expect_values failed" + (user_msg.empty() ? std::string{} : " : " + user_msg));
			}

			template <typename exception_t, typename func_t>
			static inline void expect_exception(func_t to_test, const std::string & user_msg = "")
			{
				try
				{
					to_test();
				}
				catch (const exception_t &)
				{
					return;
				}
				throw fail_exception("gcl::test::check::expect_exception failed" + (user_msg.empty() ? "" : " : " + user_msg));
			}

#define WHERE_VARS const std::string & file, const std::size_t line, const std::string & func // __FILE__, __LINE__, __func__
#pragma region macro_invokable
			template <typename value_t>
			static inline void expect(WHERE_VARS, const value_t & to_test, const std::string & user_msg = "")
			{
				expect(to_test, make_where_msg(file, line, func) + (user_msg.length() == 0 ? "" : " : ") + user_msg);
			}

			template <typename value_t, typename expected_value_t>
			static inline void expect_value(WHERE_VARS, const value_t & value, const expected_value_t & expected_value, const std::string & user_msg = "")
			{
				expect_value(value, expected_value, make_where_msg(file, line, func) + (user_msg.length() == 0 ? "" : " : ") + user_msg);
			}

			template <typename value_t, typename expected_value_t>
			static inline void expect_not_value(WHERE_VARS, const value_t & value, const expected_value_t & unexpected_value, const std::string & user_msg = "")
			{
				expect_not_value(value, unexpected_value, make_where_msg(file, line, func) + (user_msg.length() == 0 ? "" : " : ") + user_msg);
			}

			template <typename value_t, typename expected_value_t>
			static inline void expect_value_in_range(WHERE_VARS, const value_t & value, const value_range_t<expected_value_t> & expected_values_range, const std::string & user_msg = "")
			{
				expect_value_in_range(value, expected_values_range, make_where_msg(file, line, func) + (user_msg.length() == 0 ? "" : " : ") + user_msg);
			}

			template <typename value_t, typename expected_value_t, typename Fun>
			static inline void expect_values(WHERE_VARS, const value_t & value, const expected_value_t & val_before, Fun what, const expected_value_t & val_after, const std::string & user_msg = "")
			{
				if (value != val_before)
					throw fail_exception("gcl::test::check::expect_values failed : " + make_where_msg(file, line, func) + (user_msg.empty() ? std::string{} : " : " + user_msg));
				what();
				if (value != val_after)
					throw fail_exception("gcl::test::check::expect_values failed : " + make_where_msg(file, line, func) + (user_msg.empty() ? std::string{} : " : " + user_msg));
			}

			template <typename exception_t, typename func_t>
			static inline void expect_exception(WHERE_VARS, func_t to_test, const std::string & user_msg = "")
			{
				expect_exception<exception_t>(to_test, make_where_msg(file, line, func) + (user_msg.length() == 0 ? "" : " : ") + user_msg);
			}
#pragma endregion

#define GCL_TEST__EXPECT(...)							gcl::test::check::expect(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define GCL_TEST__EXPECT_VALUE(...)						gcl::test::check::expect_value(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define GCL_TEST__EXPECT_NOT_VALUE(...)					gcl::test::check::expect_not_value(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define GCL_TEST__EXPECT_VALUE_IN_RANGE(...)			gcl::test::check::expect_value_in_range(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define GCL_TEST__EXPECT_VALUES(...)					gcl::test::check::expect_values(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define GCL_TEST__EXPECT_EXCEPTION(exception_t, ...)	gcl::test::check::expect_exception<exception_t>(__FILE__, __LINE__, __func__, ##__VA_ARGS__)

		private:
			static inline std::string make_where_msg(WHERE_VARS)
			{
				auto filepos = file.find_last_of("\\/");
				std::string filename{ filepos == std::string::npos ? file : file.substr(filepos + 1) };
				return std::string{ "in " + filename + ":" + std::to_string(line) + ", at function \"" + func + "\"" };
			}
#undef WHERE_VARS
		};

		namespace logger
		{
			static const size_t default_name_w = 50;
			static const size_t default_margin_w = 3;
			static const size_t token_w = 4;

			template <std::size_t preindent_w>
			struct log_t
			{
				static std::string last_label;

				GCL_PREPROCESSOR__NOT_INSTANTIABLE(log_t);

				using next_t = log_t<preindent_w + 1>;
				using prev_t = log_t<preindent_w == 0 ? 0 : preindent_w - 1>;

				template <typename T, char fill_char_v = ' '>
				static void print(const std::string & value = "", const std::string & info = "")
				{
					static const std::size_t preindent_width_v = default_margin_w * preindent_w;
					static const std::size_t msg_width_v = default_name_w - preindent_width_v;

					// [name_too_long...]
					std::string label{ typeid(T).name() };
					if (preindent_w != 0) // avoid namespace for subcomponents
					{
						auto it = label.rfind("::");
						assert(it != std::string::npos);
						label = label.substr(it + 2);
					}
					label = (label.length() > msg_width_v ? label.substr(0, msg_width_v - 4) + "..." : label) + " ";

					if (last_label != typeid(T).name()) // not label != last_label to avoid names collisions. ex : foo::bar and foo::deprecated::bar
						std::cout
						<< std::left
						<< std::setw(preindent_width_v) << "" << (preindent_w == 0 ? "[+]" : " |-") << ' '
						<< std::setfill(fill_char_v) << std::setw(msg_width_v) << label
						<< std::setfill(' ') << "   "
						;
					else
						std::cout
						<< std::setw(preindent_width_v) << "" << " `-" << ' '
						<< std::setfill('-') << std::setw(msg_width_v) << "" << "   "
						<< std::setfill(' ')
						;
					last_label = typeid(T).name();

					std::cout
						<< value
						<< std::endl
						;
					if (!info.empty())
					{
						std::cout
							<< std::setw(preindent_width_v + default_margin_w) << "" << " >> " << info
							<< std::endl
							;
					}
				}
			};
			template <std::size_t preindent_w>
			std::string log_t<preindent_w>::last_label;
		}

		namespace type_traits
		{
			template <typename T, typename = void>
			struct has_dependencies : std::false_type {};
			template <typename T>
			struct has_dependencies<T, std::void_t<typename T::dependencies_t>> : std::true_type {};

			template <typename T, typename = void>
			struct has_proceed : std::false_type {};
			template <typename T>
			struct has_proceed<T, std::void_t<decltype(T::proceed)>> : std::true_type {};
		}

		static std::pair<std::size_t, std::size_t> winrate_counter{ 0, 0 }; // todo : private
		static std::pair<std::size_t, std::size_t> implrate_counter{ 0, 0 }; // todo : private

		template <class component_t, std::size_t deepth = 0>
		struct component
		{
			GCL_PREPROCESSOR__NOT_INSTANTIABLE(component);

			using type = typename component<component_t, deepth>;
			using log_t = typename logger::log_t<deepth>;

			static void test()
			{
				implrate_counter.second++;

				if /*constexpr*/ (deepth == 0)
				{
					winrate_counter = { 0, 0 };
					implrate_counter = { 0, 0 };
				}

				if_has_dependencies_then_it();
				if_has_proceed_then_execute();

				if /*constexpr*/ (deepth == 0)
				{
					log_t::print<component_t, '='>
						(
							"[ " + std::to_string(winrate_counter.first) + " / " + std::to_string(winrate_counter.second) + " ]" +
							(implrate_counter.first == 0 ? "" : "\t(" + std::to_string(implrate_counter.first) + " not implemented yet)")
						);
				}
			}

		// protected: // disable until VS2015 frienship issue is fix

			// 1 - has test dependencies ? [2] : skip
			template <bool has_dependencies = type_traits::has_dependencies<component_t>::value>
			static void if_has_dependencies_then_it()
			{
				static_assert(type_traits::has_dependencies<component_t>::value, "component test dependencies is mandatory");
				log_t::print<component_t>("[dependencies]");
				foreach_elem_do_test(component_t::dependencies_t{});
			}
			template <>
			static void if_has_dependencies_then_it<false>()
			{
				static_assert(!type_traits::has_dependencies<component_t>::value, "component test dependencies not allowed");
			}
			// 2 - foreach test in dependencies
			//     has proceed ? do test : "not implemented"
			//                             or is dependencies
			template <template <class...> class dependencies, class subcomponent_t, class ... T>
			static void foreach_elem_do_test(dependencies<subcomponent_t, T...>)
			{
				foreach_elem_do_test(dependencies<subcomponent_t>{});
				foreach_elem_do_test(dependencies<T...>{});
			}
			template <template <class...> class dependencies, class subcomponent_t>
			static void foreach_elem_do_test(dependencies<subcomponent_t>)
			{
				component<subcomponent_t, deepth + 1>::test();
			}
			template <template <class...> class dependencies>
			static void foreach_elem_do_test(dependencies<>)
			{
				// empty dependencies
			}

			template <bool has_proceed = type_traits::has_proceed<component_t>::value>
			static void if_has_proceed_then_execute()
			{
				++winrate_counter.second;
				try
				{
					using clock_type = std::chrono::steady_clock;

					clock_type::time_point tp_start = clock_type::now();
					auto output = do_proceed(component_t::proceed); //auto output = component_t::proceed();
					const long long elasped_usec = std::chrono::duration_cast<std::chrono::microseconds>(clock_type::now() - tp_start).count();

					log_t::print<component_t>("[PASSED] in " + std::to_string(elasped_usec) + " us", gcl::lexical_cast::to_string(output));
					++winrate_counter.first;
				}
				catch (const fail_exception & ex)
				{
					log_t::print<component_t>("[FAILED]", ex.what());
				}
				catch (const std::exception & ex)
				{
					log_t::print<component_t>("[CRASHED]", ex.what());
				}
				catch (...)
				{
					log_t::print<component_t>("[CRASHED]", "[FATAL_ERROR]");
				}
			}
			template <>
			static void if_has_proceed_then_execute<false>()
			{
				if (type_traits::has_dependencies<component_t>::value) return; // components should have their own test that garther up all subcomponents test ?

				++implrate_counter.first;
				log_t::print<component_t, '.'>("[SKIP]", "[no test implemented]");
			}

		protected:
			template <typename T, typename ... P>
			static auto do_proceed(T(*func)(P...))
			{
				return func();
			}
			template <typename ... P>
			static std::string do_proceed(void(*func)(P...))
			{
				func();
				return {}; // [ISSUE]::[BUG] : cost about 5us
			}
		};

		template <class ... components_t>
		struct components
		{
			static void test()
			{
				int expand_variadic_[] =
				{
					(component<components_t>::test(),
					std::cout << std::endl,
					0)...
				};
			}
		};
	}
}

#pragma warning(default : 4127)