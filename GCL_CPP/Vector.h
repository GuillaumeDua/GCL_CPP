#ifndef __GLC_VECTOR__
# define __GLC_VECTOR__

# include <vector>

namespace GCL
{
	template <typename T>
	struct Vector : public std::vector<T>
	{
		Vector & operator +=(const T && elem)
		{
			this->push_back(elem);
			return *this;
		}
	};
}
#endif // __GLC_VECTOR__
