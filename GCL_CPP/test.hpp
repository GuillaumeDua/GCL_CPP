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

// todo : static SFINAE tests
// type_trait : test_component<name>::pass / test_component<name>::fail as constexpr booleans value

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

			template <typename T>
			static inline void expect(const T & to_test, const std::string & user_msg)
			{
				if (!to_test())
					throw fail_exception(user_msg);
			}
			template <>
			static inline void expect<bool>(const bool & test_result, const std::string & user_msg)
			{
				if (!test_result)
					throw fail_exception(user_msg);
			}

			template <typename T>
			static inline void expect(const std::string & file, const std::size_t line, const std::string & func, const T & to_test, const std::string & user_msg = "")
			{
				expect(to_test, get_where(file, line, func) + (user_msg.length() == 0 ? "" : " : ") + user_msg);
			}

#define GCL_TEST__EXPECT(...) gcl::test::check::expect(__FILE__, __LINE__, __func__, ##__VA_ARGS__)

			/*template <typename T, typename Fun>
			static inline void expect_value(const T & value, const T & val_before, Fun what, const T & val_after, const std::string & user_msg = "")
			{
				try
				{
					if (value != val_before)
						throw fail_exception();
					what();
					if (value != val_after)
						throw fail_exception();
				}
				catch ()
				{
					throw;
				}
			}*/

		private:
			static inline std::string get_where(const std::string & file, const std::size_t line, const std::string & func)
			{
				auto filepos = file.find_last_of("\\/");
				std::string filename{ filepos == std::string::npos ? file : file.substr(filepos + 1) };
				return std::string{ "in " + filename + ":" + std::to_string(line) + ", at function \"" + func + "\"" };
			}
		};

		namespace indent_style
		{
			static const size_t default_name_w = 50;
			static const size_t default_margin_w = 3;
			static const size_t token_w = 4;

			template <std::size_t preindent_w>
			struct log_t
			{
				GCL_PREPROCESSOR__NOT_INSTANTIABLE(log_t);

				using next_t = log_t<preindent_w + 1>;
				using prev_t = log_t<preindent_w == 0 ? 0 : preindent_w - 1>;

				template <typename T>
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
					label = (label.length() > msg_width_v ? label.substr(0, msg_width_v - 3) + "..." : label);

					static std::string last_label;
					if (label != last_label)
						std::cout
						<< std::left
						<< std::setw(preindent_width_v) << "" << (preindent_w == 0 ? "[+]" : " |-") << ' '
						<< std::setw(msg_width_v) << label << "   "
						;
					else
						std::cout
						<< std::setw(preindent_width_v) << "" << " `-" << ' '
						<< std::setfill('.') << std::setw(msg_width_v) << "" << "   " << std::setfill(' ')
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

		template <class component_t, std::size_t deepth = 0>
		struct component
		{
			GCL_PREPROCESSOR__NOT_INSTANTIABLE(component);

			using type = typename component<component_t, deepth>;
			using log_t = typename indent_style::log_t<deepth>;

			static void test()
			{
				if_has_pack_then_it();
				if_has_proceed_then_execute();
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
				try
				{
					auto output = do_proceed(component_t::proceed);
					log_t::print<component_t>("[PASSED]", output);
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
				log_t::print<component_t>("[SKIP]", "no test implemented");
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