#ifndef __GLC_EXCEPTION__
# define __GLC_EXCEPTION__

# include <string>
# include <stdexcept>
# include <exception>

namespace GCL
{
	class Exception : public std::exception
	{
	public:
		explicit Exception(const std::string & msg)
		: _msg(msg)
		{}

		virtual ~Exception() throw()
		{}

		virtual const char* what() const throw ()
		{
			return this->_msg.c_str();
		}

	protected:
		const std::string _msg;
	};
};

#endif // __GLC_EXCEPTION__
