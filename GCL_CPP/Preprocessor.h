#ifndef __GCL__PREPROCESSOR__
# define __GCL__PREPROCESSOR__

#ifdef _DEBUG
# define DEBUG_INSTRUCTION(instruction) instruction
#else
# define DEBUG_INSTRUCTION(instruction)
#endif

#ifdef _GCL_DEBUG
# define _GCL_DEBUG_INSTRUCTION(instruction) instruction
#else
# define _GCL_DEBUG_INSTRUCTION(instruction)
#endif

#endif // __GCL__PREPROCESSOR__