#ifndef GCL_CPP_IO__
# define GCL_CPP_IO__

# include <iostream>
# include "Preprocessor.h"

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
					is.read(reinterpret_cast<char *>(&var), sizeof(T));
					_GCL_ASSERT(is);
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
					is.read(reinterpret_cast<char *>(var), sizeof(T));
					_GCL_ASSERT(is);
				}

				template <>
				static void read<std::string>(std::istream & is, std::string & var)
				{
					std::string::size_type size;
					is.read(reinterpret_cast<char *>(&size), sizeof(std::string::size_type));
					_GCL_ASSERT(is);
					var.resize(size, '\0');
					is.read(&var[0], size);
					_GCL_ASSERT(is);
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