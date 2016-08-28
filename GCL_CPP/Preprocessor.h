#ifndef __GCL__PREPROCESSOR__
# define __GCL__PREPROCESSOR__

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
#else
# define _GCL_DEBUG_INSTRUCTION(instruction)
# define _GCL_RELEASE_INSTRUCTION(instruction)	instruction
#endif

#endif // __GCL__PREPROCESSOR__