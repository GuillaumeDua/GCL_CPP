#ifndef GCL_FUNCTIONNAL_HPP__
# define GCL_FUNCTIONNAL_HPP__

# include <functional>

namespace GCL
{
	namespace Functionnal
	{
		struct OnDestroyExecute : public std::function<void()>
		{
			explicit OnDestroyExecute(std::function<void()> && func)
			{
				func.swap(*this);
			}
			OnDestroyExecute(const OnDestroyExecute & w)
			{
				const_cast<OnDestroyExecute &>(w).swap(*this);
			}
			~OnDestroyExecute()
			{
				if (*this)
					(*this)();
			}
		};
	}
}

#endif // GCL_FUNCTIONNAL_HPP__
