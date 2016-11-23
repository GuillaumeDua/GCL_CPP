#ifndef GCL_CPP_IO__
# define GCL_CPP_IO__

# include <iostream>
# include "Preprocessor.h"

# include <sstream> // For test purpose only

namespace GCL
{
	namespace IO
	{
		namespace Policy
		{
			struct Binary
			{
				template <typename T>
				static void	write(std::ostream & os, const T & var)
				{
					os.write(reinterpret_cast<const char *>(&var), sizeof(T));
					_GCL_ASSERT(os);
				}
				template <typename T>
				static void	read(std::istream & is, T & var)
				{
					_GCL_ASSERT(is);
					is.read(reinterpret_cast<char *>(&var), sizeof(T));
				}
				template <typename T>
				static void	write(std::ostream & os, const T * var)
				{
					os.write(reinterpret_cast<const char *>(var), sizeof(T));
					_GCL_ASSERT(os);
				}
				template <typename T>
				static void	read(std::istream & is, T * var)
				{
					_GCL_ASSERT(is);
					is.read(reinterpret_cast<char *>(var), sizeof(T));
				}

				template <>
				static void read<std::string>(std::istream & is, std::string & var)
				{
					std::string::size_type size;
					_GCL_ASSERT(is);
					is.read(reinterpret_cast<char *>(&size), sizeof(std::string::size_type));
					var.resize(size, '\0');
					_GCL_ASSERT(is);
					is.read(&var[0], size);
				}
				template <>
				static void write<std::string>(std::ostream & os, const std::string & var)
				{
					std::string::size_type size = var.length();
					os.write(reinterpret_cast<char *>(&size), sizeof(std::string::size_type));
					_GCL_ASSERT(os);
					os.write(&var[0], var.length());
					_GCL_ASSERT(os);
				}
			};
			struct Stream
			{
				template <typename T>
				static void	write(std::ostream & os, const T & var)
				{
					os << var;
					_GCL_ASSERT(os);
				}
				template <typename T>
				static void	read(std::istream & is, T & var)
				{
					_GCL_ASSERT(is);
					is >> var;
				}
				template <typename T>
				static void	write(std::ostream & os, const T * var)
				{
					Binary::write<T>(os, var);
				}
				template <typename T>
				static void	read(std::istream & is, T * var)
				{
					Binary::read<T>(is, var);
				}

				template <>
				static void read<std::string>(std::istream & is, std::string & var)
				{
					Binary::read<std::string>(is, var);
				}
				template <>
				static void write<std::string>(std::ostream & os, const std::string & var)
				{
					Binary::write<std::string>(os, var);
				}
			};
		}

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
}

#endif // GCL_CPP_IO__