#ifndef GCL_TEST_HPP__
# define GCL_TEST_HPP__

#include <gcl_cpp/preprocessor.hpp>
//
// No other gcl_cpp components allowed here
// 

#include <type_traits>
#include <iomanip>
#include <sstream>
#include <functional>
#include <chrono>
#include <vector>
#include <algorithm>

// todo : static SFINAE tests
// type_trait : test_component<name>::pass / test_component<name>::fail as constexpr booleans value
// => constexpr testing

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
			static inline void expect(const Fun & to_test, const std::string & user_msg = "")
			{
				if (!to_test())
					throw fail_exception("gcl::test::check::expect failed" + (user_msg.empty() ? "" : " : " + user_msg));
			}
			template <>
			static inline void expect<bool>(const bool & test_result, const std::string & user_msg)
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

#pragma region macro_invokable
#define GCL_TEST_WHERE_VARS const std::string & file, const std::size_t line, const std::string & func // __FILE__, __LINE__, __func__

			template <typename value_t>
			static inline void expect(GCL_TEST_WHERE_VARS, const value_t & to_test, const std::string & user_msg = "")
			{
				expect(to_test, make_where_msg(file, line, func) + (user_msg.length() == 0 ? "" : " : ") + user_msg);
			}

			template <typename value_t, typename expected_value_t>
			static inline void expect_value(GCL_TEST_WHERE_VARS, const value_t & value, const expected_value_t & expected_value, const std::string & user_msg = "")
			{
				expect_value(value, expected_value, make_where_msg(file, line, func) + (user_msg.length() == 0 ? "" : " : ") + user_msg);
			}

			template <typename value_t, typename expected_value_t>
			static inline void expect_value_in_range(GCL_TEST_WHERE_VARS, const value_t & value, const value_range_t<expected_value_t> & expected_values_range, const std::string & user_msg = "")
			{
				expect_value_in_range(value, expected_values_range, make_where_msg(file, line, func) + (user_msg.length() == 0 ? "" : " : ") + user_msg);
			}

			template <typename value_t, typename expected_value_t, typename Fun>
			static inline void expect_values(GCL_TEST_WHERE_VARS, const value_t & value, const expected_value_t & val_before, Fun what, const expected_value_t & val_after, const std::string & user_msg = "")
			{
				if (value != val_before)
					throw fail_exception("gcl::test::check::expect_values failed" + (user_msg.empty() ? std::string{} : " : " + user_msg));
				what();
				if (value != val_after)
					throw fail_exception("gcl::test::check::expect_values failed" + (user_msg.empty() ? std::string{} : " : " + user_msg));
			}
#pragma endregion

#define GCL_TEST__EXPECT(...)					gcl::test::check::expect(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define GCL_TEST__EXPECT_VALUE(...)				gcl::test::check::expect_value(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define GCL_TEST__EXPECT_VALUE_IN_RANGE(...)	gcl::test::check::expect_value_in_range(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define GCL_TEST__EXPECT_VALUES(...)			gcl::test::check::expect_values(__FILE__, __LINE__, __func__, ##__VA_ARGS__)

			// todo : expect value range

		private:
			static inline std::string make_where_msg(GCL_TEST_WHERE_VARS)
			{
				auto filepos = file.find_last_of("\\/");
				std::string filename{ filepos == std::string::npos ? file : file.substr(filepos + 1) };
				return std::string{ "in " + filename + ":" + std::to_string(line) + ", at function \"" + func + "\"" };
			}
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

					if (label != last_label)
						std::cout
						<< std::left
						<< std::setw(preindent_width_v) << "" << (preindent_w == 0 ? "[+]" : " |-") << ' '
						<< std::setfill(fill_char_v) << std::setw(msg_width_v) << label
						<< std::setfill(' ') << "   "
						;
					else
						std::cout
						<< std::setw(preindent_width_v) << "" << " `-" << ' '
						<< std::setfill(fill_char_v) << std::setw(msg_width_v) << "" << "   "
						<< std::setfill(' ')
						;
					last_label = label;

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
			struct has_pack : std::false_type {};
			template <typename T>
			struct has_pack<T, std::void_t<typename T::pack_t>> : std::true_type {};

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

				if (deepth == 0)
					winrate_counter = {0, 0};

				if_has_pack_then_it();
				if_has_proceed_then_execute();

				if (deepth == 0)
				{
					log_t::print<component_t, '='>
						(
							"[ " + std::to_string(winrate_counter.first) + " / " + std::to_string(winrate_counter.second) + " ]" +
							"\t(" + std::to_string(implrate_counter.first) + " not implemented yet)"
						);
				}
			}

		// protected: // disable until VS2015 frienship issue is fix

			// 1 - has test pack ? [2] : skip
			template <bool has_pack = type_traits::has_pack<component_t>::value>
			static void if_has_pack_then_it()
			{
				static_assert(type_traits::has_pack<component_t>::value, "component test pack is mandatory");
				log_t::print<component_t>("[PACK]");
				foreach_elem_do_test(component_t::pack_t{});
			}
			template <>
			static void if_has_pack_then_it<false>()
			{
				static_assert(!type_traits::has_pack<component_t>::value, "component test pack not allowed");
			}
			// 2 - foreach test in pack
			//     has proceed ? do test : "not implemented"
			//                             or is pack
			template <template <class...> class pack, class subcomponent_t, class ... T>
			static void foreach_elem_do_test(pack<subcomponent_t, T...>)
			{
				foreach_elem_do_test(pack<subcomponent_t>{});
				foreach_elem_do_test(pack<T...>{});
			}
			template <template <class...> class pack, class subcomponent_t>
			static void foreach_elem_do_test(pack<subcomponent_t>)
			{
				component<subcomponent_t, deepth + 1>::test();
			}

			template <bool has_proceed = type_traits::has_proceed<component_t>::value>
			static void if_has_proceed_then_execute()
			{
				++winrate_counter.second;
				try
				{
					using clock_t = std::chrono::system_clock;

					clock_t::time_point tp_start = clock_t::now();
					auto output = do_proceed(component_t::proceed);
					const long long elasped_usec = std::chrono::duration_cast<std::chrono::microseconds>(clock_t::now() - tp_start).count();

					log_t::print<component_t>("[PASSED] in " + std::to_string(elasped_usec) + " ms", output);
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
					log_t::print<component_t>("[CRASHED]", "FATAL_ERROR");
				}
			}
			template <>
			static void if_has_proceed_then_execute<false>()
			{
				++implrate_counter.first;
				log_t::print<component_t, '.'>("[SKIP]", "no test implemented");
			}

		protected:
			template <typename T, typename ... P>
			static std::string do_proceed(T(*func)(P...))
			{
				auto ret = func();
				std::ostringstream oss;
				oss << std::left << std::boolalpha << ret;
				return oss.str();
			}
			template <typename ... P>
			static std::string do_proceed(void(*func)(P...))
			{
				func();
				return std::string{};
			}
		};
	}
}

#endif // GCL_TEST_HPP__