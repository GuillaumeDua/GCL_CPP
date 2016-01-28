#ifndef __GCL_MATHS__
#define __GCL_MATHS__

namespace GCL
{
	namespace Maths
	{
		static const double PI = 3.14159265;

		static inline const float Cos(const float angle) { return static_cast<float>(std::cos(angle * PI / 180.0)); }
		static inline const float Sin(const float angle) { return static_cast<float>(std::sin(angle * PI / 180.0)); }
	}
}

#endif // __GCL_MATHS__