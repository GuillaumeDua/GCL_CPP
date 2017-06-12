#ifndef GCL_FUNCTIONNAL_HPP__
# define GCL_FUNCTIONNAL_HPP__

# include <functional>

namespace gcl
{
	namespace functionnal
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

#endif // GCL_FUNCTIONNAL_HPP__
