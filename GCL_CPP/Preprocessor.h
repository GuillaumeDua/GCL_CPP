#ifndef __GCL__PREPROCESSOR__
# define __GCL__PREPROCESSOR__

# include <exception>
# include <stdexcept>
# include <cassert>

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
#endif

#ifdef _GCL_DEBUG
# define _GCL_DEBUG_INSTRUCTION(instruction)	instruction
# define _GCL_RELEASE_INSTRUCTION(instruction)
# define _GCL_ASSERT(instruction)				assert(instruction)
#else
# define _GCL_DEBUG_INSTRUCTION(instruction)
# define _GCL_RELEASE_INSTRUCTION(instruction)	instruction
# define _GCL_ASSERT(instruction)				{ if (not instruction) throw std::runtime_error("_GCL_ASSERT"); }
#endif

#endif // __GCL__PREPROCESSOR__