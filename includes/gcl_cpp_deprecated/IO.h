#ifndef GCL_CPP_IO__
# define GCL_CPP_IO__

#include <gcl_cpp/preprocessor.hpp>
#include <iostream>
#include <type_traits>
#include <typeinfo> // check with #if defined(__cpp_rtti)

// TODO : merge with gcl::io::fd_proxy
// TODO : should rely on gcl::typeinfo::hashcode/typename if no __cpp_rtti

namespace gcl::io::introspection
{
	// has std::ostream & << declval<T>()
	template <typename T, typename = void>
	struct has_ostream_shift_operator : std::false_type {};
	template <typename T>
	struct has_ostream_shift_operator
	<
		T,
		std::void_t<decltype(std::declval<std::ostream>() << std::declval<T>())>
	> : std::true_type {};

	template <typename T, typename = void>
	struct has_istream_shift_operator : std::false_type {};
	template <typename T>
	struct has_istream_shift_operator
	<
		T,
		std::void_t<decltype(std::declval<std::istream>() >> std::declval<T>())>
	> : std::true_type {};
}

template
<
	class CharT,
	class Traits,
	typename T,
	typename = std::enable_if_t<not gcl::io::introspection::has_ostream_shift_operator<T>::value>
>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits> & os,
	const T & value)
{
	// todo :
	//	if __cpp_rtti, std::string_view typeid(func_sig : CharT == `template-type-parameter-1`)
	//	then do not display the message below
	# pragma message						\
	(										\
		"gcl::compilation_warning : in "	\
		__FUNCSIG__							\
		"\ngcl::io : use of default-generated std::ostream&operator<<(T)")

	#if defined(__cpp_rtti)
	return os << "[gcl::io]{0x" << &value << "}(" << typeid(T).name() << ')';
	#endif
	return os << "[gcl::io]{0x" << &value << "}(no __cpp_rtti)";
}

namespace gcl::io::policy
{
	struct binary
	{
		template <typename T>
		static void	write(std::ostream & os, const T & var)
		{
			os.write(reinterpret_cast<const char *>(&var), sizeof(T));
			GCL_ASSERT(os);
		}
		template <typename T>
		static void	read(std::istream & is, T & var)
		{
			GCL_ASSERT(is);
			is.read(reinterpret_cast<char *>(&var), sizeof(T));
		}
		template <typename T>
		static void	write(std::ostream & os, const T * var)
		{
			os.write(reinterpret_cast<const char *>(var), sizeof(T));
			GCL_ASSERT(os);
		}
		template <typename T>
		static void	read(std::istream & is, T * var)
		{
			GCL_ASSERT(is);
			is.read(reinterpret_cast<char *>(var), sizeof(T));
		}

		template <>
		static void read<std::string>(std::istream & is, std::string & var)
		{
			std::string::size_type size;
			GCL_ASSERT(is);
			is.read(reinterpret_cast<char *>(&size), sizeof(std::string::size_type));
			var.resize(size, '\0');
			GCL_ASSERT(is);
			is.read(&var[0], size);
		}
		template <>
		static void write<std::string>(std::ostream & os, const std::string & var)
		{
			std::string::size_type size = var.length();
			os.write(reinterpret_cast<char *>(&size), sizeof(std::string::size_type));
			GCL_ASSERT(os);
			os.write(&var[0], var.length());
			GCL_ASSERT(os);
		}
	};
	struct stream
	{
		template <typename T>
		static void	write(std::ostream & os, const T & var)
		{
			os << var;
			GCL_ASSERT(os);
		}
		template <typename T>
		static void	read(std::istream & is, T & var)
		{
			GCL_ASSERT(is);
			is >> var;
		}
		template <typename T>
		static void	write(std::ostream & os, const T * var)
		{
			binary::write<T>(os, var);
		}
		template <typename T>
		static void	read(std::istream & is, T * var)
		{
			binary::read<T>(is, var);
		}

		template <>
		static void read<std::string>(std::istream & is, std::string & var)
		{
			binary::read<std::string>(is, var);
		}
		template <>
		static void write<std::string>(std::ostream & os, const std::string & var)
		{
			binary::write<std::string>(os, var);
		}
	};
}

# include <sstream> // For test purpose only

namespace gcl::io
{	// todo : move to test/gcl_cpp/test/io.h
	struct Test
	{
		static bool Proceed(void)
		{
			std::stringstream ss;

			std::string r_str1 = "std::string1_str";
			std::string r_str2 = "this_is_std::string2_str";
			int r_int1 = 42;
			int r_int2 = 11223344;

			ss
				<< r_str1
				<< r_int1
				<< r_str2
				<< r_int2
				;

			std::string w_str1, w_str2;
			int w_int1, w_int2;

			ss
				>> w_str1
				>> w_int1
				>> w_str2
				>> w_int2
				;

			return (
				r_str1 == w_str1
				&& r_str2 == w_str2
				&& r_int1 == w_int1
				&& r_int2 == w_int2
				);
		}
	};
}

#endif // GCL_CPP_IO__