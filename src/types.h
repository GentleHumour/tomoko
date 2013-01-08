#ifndef TOMOKO_TYPES_H
#define TOMOKO_TYPES_H

#include <inttypes.h>

//-----------------------------------------------------------------------------
/**
 * This type defines the width of a stack element as a signed integer.
 */
typedef int32_t Cell;

/**
 * This type defines the width of a stack element as a unsigned integer.
 */
typedef int32_t UCell;

/**
 * This type defines the double-width signed integer type.
 */
typedef int64_t DCell;

/**
 * Type of the function pointer that is the codeword of a Forth word.
 *
 * Pointers of this type will point to the native functions that are used to
 * implement words.
 */
typedef void (*CodeWord)();

/**
 * Type of a link pointer to the previous dictionary word.
 */
typedef const void *Link;

/**
 * Return the Forth form of a Boolean condition cond.
 *
 * This is 0 for false and ~0 (-1) for true.
 */
#define BOOLEAN(cond) ((cond) ? ~(Cell)0 : (Cell)0)

//-----------------------------------------------------------------------------

#endif // TOMOKO_TYPES_H

