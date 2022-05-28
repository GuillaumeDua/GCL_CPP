#ifndef GCL_EXCEPTION_HPP__
# define GCL_EXCEPTION_HPP__

# include <string>
# include <stdexcept>
# include <exception>

namespace gcl
{
	class exception : public std::exception
	{
	public:
		explicit exception(const std::string & msg)
		: std::exception(msg.c_str())
		{}
	};

	// todo : custom exception stack/pack ?
	// -> channeling exception stack
};

#endif // GCL_EXCEPTION_HPP__
