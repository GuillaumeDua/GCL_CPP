#ifndef __GLC_VECTOR__
# define __GLC_VECTOR__

# include <vector>

namespace GCL
{
	template <typename T>
	struct Vector : public std::vector<T>
	{
		Vector()
			: std::vector<T>()
		{}
		Vector(const std::vector<T> & stdVector)
			: std::vector<T>(stdVector)
		{}
		Vector(const std::vector<T> && stdVector)
			: std::vector<T>(stdVector)
		{}
		Vector(const std::initializer_list<T> & initList)
			: std::vector<T>(initList)
		{}

		Vector & operator+=(T && elem)
		{
			this->emplace_back(elem);
			return *this;
		}
		Vector & operator+=(const Vector & vec)
		{
			for (auto & elem : vec)
				this->push_back(elem);
			return *this;
		}
		/*Vector & operator +=(const T & elem)
		{
			this->push_back(elem);
			return *this;
		}*/
	};
}
#endif // __GLC_VECTOR__
