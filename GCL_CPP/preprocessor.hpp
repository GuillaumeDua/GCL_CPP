#ifndef GCL_PREPROCESSOR_HPP__
# define GCL_PREPROCESSOR_HPP__

#ifdef _MSC_VER	// synthaxic sugar used by GCC and CLANG
# define and	&&
# define and_eq &=
# define bitand &
# define bitor	|
# define compl	~
# define not	!
# define not_eq	!=
# define or		||
# define or_eq	|=
# define xor	^
# define xor_eq	^=
#endif

#ifdef _DEBUG
# define DEBUG_INSTRUCTION(instruction)		instruction
# define RELEASE_INSTRUCTION(instruction)
#else
# define DEBUG_INSTRUCTION(instruction)
# define RELEASE_INSTRUCTION(instruction)	instruction
#endif // _DEBUG

#ifdef _GCL_DEBUG
# include <cassert>
# define GCL_DEBUG_INSTRUCTION(instruction)		instruction
# define GCL_RELEASE_INSTRUCTION(instruction)
# define GCL_ASSERT(instruction)				assert(instruction)
#else
# include <gcl_cpp/exception.hpp>
# define GCL_DEBUG_INSTRUCTION(instruction)
# define GCL_RELEASE_INSTRUCTION(instruction)	instruction
# define GCL_ASSERT(instruction)				{	if (not instruction) throw gcl::exception(						\
													"GCL_ASSERT : " __FILE__ " : " + std::to_string(__LINE__) + "\n"	\
													);																	\
												}
#endif

#endif // GCL_PREPROCESSOR_HPP__