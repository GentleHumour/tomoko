//-----------------------------------------------------------------------------
//
// This module defines abstractions for representing the state of the Forth
// Abstract Machine; specifically, instruction-related registers, the stacks and
// the heap.
//
//-----------------------------------------------------------------------------

#ifndef TOMOKO_MACHINE_H
#define TOMOKO_MACHINE_H

#include "types.h"

//-----------------------------------------------------------------------------
// Stacks
// ~~~~~~
// Return and parameter stacks both grow downwards.  ATmega168 supports indirect
// addressing with pre-decrement and post-increment.  So the use of
// pre-decrement for push means that the stack pointer will always point to the
// current top of the stack.

/**
 * Size of the parameter stack in cells.
 */
#define PARAMETER_STACK_CELLS 64

/**
 * Size of the parameter stack in cells.
 */
#define RETURN_STACK_CELLS 32

/**
 * Size of the statically allocated dictionary, in bytes.
 */
#define DICTIONARY_SIZE 8192

/**
 * Storage for the parameter stack.
 */
extern Cell parameterStack[PARAMETER_STACK_CELLS];

/**
 * Storage for the return stack.
 */
extern Cell returnStack[RETURN_STACK_CELLS];

/**
 * The parameter stack pointer.
 */
extern Cell *sp; // = &parameterStack[PARAMETER_STACK_CELLS];

/**
 * The return stack pointer.
 */
extern Cell *rsp; // = &returnStack[RETURN_STACK_CELLS];

/**
 * The part of the dictionary that can be affected by HERE ALLOT CREATE , C,
 * and the like.
 *
 * Declared as an array of Cells so that the compiler will align it on a word
 * boundary.
 *
 * TODO: actually, that assumption is not guaranteed for all architectures.
 * Ensure that it will work.  Works OK on x86, but what about ATmega168?
 *
 * Hand-compiled words, constants and variables are declared const so that they
 * will reside in flash, whereas this part of the dictionary resides in RAM.
 */
extern Cell dictionary[DICTIONARY_SIZE / sizeof (Cell)];

//-----------------------------------------------------------------------------
/**
 * The Forth instruction pointer.
 *
 * An eXecution Token (XT) is the address of the codeword field of the word's
 * dictionary entry, i.e. the Code Field Address (CFA).  The codeword field 
 * contains a pointer to a native function that implements the word.
 *
 * The compiled form of a colon definition is a sequence of XTs in the
 * parameter field (which immediately follows the code field).
 *
 * The Instruction Pointer (IP) a pointer to the XT for the next Forth word
 * to be executed.  That is, it is a pointer to a pointer to a CodeWord.
 * The IP increments by the size of an XT (the size of a pointer, the same as
 * the size of a Cell), as each Forth word is invoked.
 */
extern CodeWord **ip;

//-----------------------------------------------------------------------------
/**
 * This is the execution token (XT) of the currently executing word.  That is,
 * it is the address of that word's codeword.
 *
 * This variable is conventionally called "W" in many discussions of the
 * Forth abstract machine.
 *
 * Since it is the Code Field Address, this must be known when executing
 * colon definitions, since it will be used to compute the address of the
 * Parameter Field where the code resides (the Parameter Field begins
 * at the cell after the Code Field).
 *
 * For almost all words, this value *could* be worked out by subtracting the
 * size of one cell from ip after it has advanced to the next instruction.
 * However, that fails in the cast of EXECUTE, where we want to continue with
 * whatever code called EXECUTE, usually INTERPRET.  In the case of EXECUTE,
 * we set w = TOS, then call (*w)(), then continue on our merry way in NEXT().
 * See the code for NEXT().
 */
extern CodeWord *w;

//-----------------------------------------------------------------------------
/**
 * Fetch the next codeword into w, advance the instruction pointer, and finally
 * execute the codeword.
 */
#define NEXT()  \
  do {          \
    w = *ip++;  \
    (*w)();     \
  } while (0)

//-----------------------------------------------------------------------------
/**
 * Push the specified value using the specified stack pointer.
 */
#define STACK_PUSH(ptr,value) do { *--ptr = (Cell)(value); } while (0)

/**
 * Return the top of the stack popped from the specified stack pointer.
 */
#define STACK_POP(ptr) (*ptr++)

/**
 * Return the address of the nth cell of the stack, n >= 0.
 *
 * (n == 0) ==> address of TOS.
 */
#define STACK_ADDR(ptr, n) ((ptr) + (n))

/**
 * Return the nth cell on the stack, n >= 0.
 *
 * (n == 0) ==> return the top of the stack (equivalent to DUP).
 */
#define STACK_PICK(ptr,n) (*STACK_ADDR(ptr,n))

//-----------------------------------------------------------------------------

#endif // TOMOKO_MACHINE_H

