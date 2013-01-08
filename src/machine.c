//-----------------------------------------------------------------------------
//
// This module defines abstractions for representing the state of the Forth
// Abstract Machine; specifically, instruction-related registers, the stacks and
// the heap.
//
//-----------------------------------------------------------------------------

#include "machine.h"

//-----------------------------------------------------------------------------

Cell parameterStack[PARAMETER_STACK_CELLS];
Cell returnStack[RETURN_STACK_CELLS];
Cell *sp = &parameterStack[PARAMETER_STACK_CELLS];
Cell *rsp = &returnStack[RETURN_STACK_CELLS];
Cell dictionary[DICTIONARY_SIZE / sizeof (Cell)];
CodeWord **ip;
CodeWord *w;

//-----------------------------------------------------------------------------

