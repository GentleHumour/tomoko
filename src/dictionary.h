//-----------------------------------------------------------------------------
//
// Purpose:
//   This header defines constants pertaining to bit fields in the length
//   fields of the dictionary header of Forth words, and macros which allow
//   Forth words to be compiled by hand and pre-defined in the Tomoko executable
//   in the form of constant data.  Constants are defined using the C 
//   preprocessor (rather than in the C++ style) so that this code can be 
//   compiled either as C or C++.
//
// Main Macros:
//   DEF_CONST(,,,)           Define a Forth constant.
//   DEF_CONST_STRING(,,,)    Define a Forth string constant.
//   DEF_VAR(,,,)             Define a Forth variable.
//   DEF_CODE(,,,)            Define a Forth word implemented in terms of a 
//                            native (i.e. C/C++) function.
//   BEGIN_COLON(,,,,)        
//     XT(WORD1), XT(WORD2),  
//     ..., XT(WORDn)
//   END_COLON()              BEGIN_COLON() and END_COLON() mark the start and 
//                            end, respectively of a hand-compiled Forth colon
//                            definition.  That is, a Forth word written in 
//                            terms of other Forth words.  XT() returns the 
//                            Execution Token of a Forth word, which is what
//                            must be compiled in a colon definition.  
//                            END_COLON() always compiles the execution token
//                            of the Forth word EXIT.
//   LINK()                   Returns the Link Field Address (LFA) of any
//                            hand-compiled Forth word defined using one of the 
//                            above macros.  All of the macros (with the 
//                            exception of XT()) take a linkInit parameter
//                            that should be computed using LINK().
//
//-----------------------------------------------------------------------------

#ifndef TOMOKO_DICTIONARY_H
#define TOMOKO_DICTIONARY_H

#include "types.h"

//-----------------------------------------------------------------------------
/**
 * The immediate flag, added into the length field for immediate words.
 */
#define IMMEDIATE_BIT 0x80

/**
 * The hidden flag, added into the length field for words that are hidden from
 * dictionary searches.
 */
#define HIDDEN_BIT 0x40

/**
 * A bit mask selecting the length field bits but not the hidden or immediate
 * flags.
 *
 * Beware of signed characters.
 */
#define LENGTH_BITS (0xFF ^ (IMMEDIATE_BIT|HIDDEN_BIT))

//-----------------------------------------------------------------------------
/**
 * Declare the struct type of the dictionary header for a word.  The header size
 * is rounded up to a whole number multiple of Cells.
 *
 * The header fields are as follows:
 *   link            the address of the previous dictionary entry, initialised 
 *                   using LINK(label), where label is the label parameter to 
 *                   the DEF_...() macro that defined the previous entry.
 *   length          the string length of forthName, excluding the trailing 
 *                   NUL terminator.
 *
 * Macro parameters:
 * @param forthName  the name of the Forth word as it appears in Forth source.
 *
 * NOTE: C allows the forthName string to be jammed into the name field without
 * allowing space for the ASCII NUL terminator.  But C++ requires that the field
 * be large enough to hold all characters, including the NUL.  As a concession 
 * to C++ compatibility, we include the NUL terminator in the name field of the
 * header, even though Forth doesn't use it.
 */
#define DECLARE_HEADER(forthName)                                             \
  union                                                                       \
  {                                                                           \
    struct                                                                    \
    {                                                                         \
      Link link;                                                              \
      char length;                                                            \
      char name[sizeof (forthName)];                                          \
    } h;                                                                      \
    char padding[                                                             \
      (sizeof (Link) + sizeof (char) + sizeof (forthName) + (sizeof (Cell)-1))\
      / sizeof (Cell) * sizeof (Cell)                                         \
    ];                                                                        \
  } u

//-----------------------------------------------------------------------------
/**
 * DEF_CONST() defines a dictionary entry for a constant.  It references
 * fn_CONST(void), which is the native implementation, to be defined later.
 *
 * @param linkInit   the address of the previous dictionary entry. It should be 
 *                   passed a value of the form LINK(label), where label is the 
 *                   label parameter to the DEF_...() macro that defined the 
 *                   previous entry.
 * @param label      the C variable name used to hold the dictionary entry 
 *                   defined by this macro.
 * @param forthName  the name of the Forth word as a string constant.
 * @param valueInit  the initial value of the constant (a constant expression).
 */
#define DEF_CONST(linkInit,label,forthName,valueInit)                         \
  const struct                                                                \
  {                                                                           \
    DECLARE_HEADER(forthName);                                                \
    CodeWord codeWord;                                                        \
    Cell value;                                                               \
  } label = {                                                                 \
    { { (linkInit), sizeof (forthName) - 1, (forthName) } },                  \
    &fn_CONST, (valueInit)                                                    \
  }

extern void fn_CONST(void);

//-----------------------------------------------------------------------------
/**
 * DEF_CONST_STRING() defines a dictionary entry for a string constant.  It
 * references fn_CONST_STRING(void), which is the native implementation, to be
 * defined later.
 *
 *
 * @param linkInit   the address of the previous dictionary entry. It should be 
 *                   passed a value of the form LINK(label), where label is the 
 *                   label parameter to the DEF_...() macro that defined the 
 *                   previous entry.
 * @param label      the C variable name used to hold the dictionary entry 
 *                   defined by this macro.
 * @param forthName  the name of the Forth word as a string constant.
 * @param valueInit  the value of the string, as a double-quoted string literal.
 */
#define DEF_CONST_STRING(linkInit,label,forthName,valueInit)                  \
  const struct                                                                \
  {                                                                           \
    DECLARE_HEADER(forthName);                                                \
    CodeWord codeWord;                                                        \
    union                                                                     \
    {                                                                         \
      struct                                                                  \
      {                                                                       \
        Cell length;                                                          \
        char chars[sizeof (forthName)];                                       \
      } value;                                                                \
      char valuePadding[                                                      \
        (sizeof (Cell) + (sizeof (Cell) - 1) + sizeof (valueInit))            \
         / sizeof (Cell) * sizeof (Cell)                                      \
      ];                                                                      \
    };                                                                        \
  } label = {                                                                 \
    { { (linkInit), sizeof (forthName) - 1, (forthName) } },                  \
    &fn_CONST_STRING,                                                         \
    { { sizeof (valueInit) - 1, (valueInit) } }                               \
  }

extern void fn_CONST_STRING(void);

//-----------------------------------------------------------------------------
/**
 * DEF_VAR() defines a dictionary entry for a variable.  It references
 * fn_VAR(void), which is the native implementation, to be defined later.  A Cell
 * is reserved for the value of the variable, and initialised to valueInit.
 *
 * @param linkInit   the address of the previous dictionary entry. It should be 
 *                   passed a value of the form LINK(label), where label is the 
 *                   label parameter to the DEF_...() macro that defined the 
 *                   previous entry.
 * @param label      the C variable name used to hold the dictionary entry 
 *                   defined by this macro.
 * @param forthName  the name of the Forth word as a string constant.
 * @param valueInit  the initial value of the variable, of type Cell.
 */
#define DEF_VAR(linkInit,label,forthName,valueInit)                           \
  Cell label##_value = (valueInit);                                           \
  const struct                                                                \
  {                                                                           \
    DECLARE_HEADER(forthName);                                                \
    CodeWord codeWord;                                                        \
    Cell *const address;                                                      \
  } label = {                                                                 \
    { { (linkInit), sizeof (forthName) - 1, (forthName) } },                  \
    &fn_VAR, &label##_value                                                   \
  }

extern void fn_VAR(void);

//-----------------------------------------------------------------------------
/**
 * DEF_NATIVE() defines a dictionary entry for a native word, comprising a 
 * header and a codeword, which is the address of the native function called
 * function().
 *
 * @param linkInit   the address of the previous dictionary entry. It should be 
 *                   passed a value of the form LINK(label), where label is the 
 *                   label parameter to the DEF_...() macro that defined the 
 *                   previous entry.
 * @param label      the C variable name used to hold the dictionary entry 
 *                   defined by this macro.
 * @param function   the address of the C function stored in the codeword.
 * @param forthName  the name of the Forth word as a string constant.
 * @param flags      the bit flags (IMMEDIATE_BIT and/or HIDDEN_BIT) that 
 *                   control the behaviour of this code word. They are masked
 *                   into the high-order bits of the length field.
 */
#define DEF_NATIVE(linkInit,label,function,forthName,flags)                   \
  extern void function(void);                                                 \
  const struct                                                                \
  {                                                                           \
    DECLARE_HEADER(forthName);                                                \
    CodeWord codeWord;                                                        \
  } label = {                                                                 \
    { { (linkInit), (flags) | (sizeof (forthName) - 1), (forthName) } },      \
    &(function)                                                               \
  }

//-----------------------------------------------------------------------------
/**
 * DEF_CODE() defines a dictionary entry for a native word, comprising a
 * header and a codeword, which is the address of the native function called
 * fn_##label(void).
 *
 * @param linkInit   the address of the previous dictionary entry. It should be 
 *                   passed a value of the form LINK(label), where label is the 
 *                   label parameter to the DEF_...() macro that defined the 
 *                   previous entry.
 * @param label      the C variable name used to hold the dictionary entry 
 *                   defined by this macro.
 * @param forthName  the name of the Forth word as a string constant.
 * @param flags      the bit flags (IMMEDIATE_BIT and/or HIDDEN_BIT) that 
 *                   control the behaviour of this code word. They are masked
 *                   into the high-order bits of the length field.
 */
#define DEF_CODE(linkInit,label,forthName,flags)                              \
  DEF_NATIVE(linkInit,label,fn_##label,forthName,flags)

//-----------------------------------------------------------------------------
/**
 * BEGIN_COLON() begins a dictionary entry for a colon definition.
 *
 * It references fn_DOCOL(void), which is the native implementation, to be 
 * defined later.
 *
 * A hand-compiled colon definition should consist of BEGIN_COLON(), then a
 * comma-separated list of (xtCount+1) eXecution Tokens (addresses of code
 * fields, as defined by the XT macro), and finally the END_COLON() macro.
 * (The extra XT - the +1 added to xtCount - is for the final EXIT instruction
 * appended by END_COLON().)
 *
 * @param linkInit   the address of the previous dictionary entry. It should be 
 *                   passed a value of the form LINK(label), where label is the 
 *                   label parameter to the DEF_...() macro that defined the 
 *                   previous entry.
 * @param label      the C variable name used to hold the dictionary entry 
 *                   defined by this macro.
 * @param forthName  the name of the Forth word as a string constant.
 * @param flags      the bit flags (IMMEDIATE_BIT and/or HIDDEN_BIT) that 
 *                   control the behaviour of this code word. They are masked
 *                   into the high-order bits of the length field.
 */
#define BEGIN_COLON(linkInit,label,forthName,flags,xtCount)                   \
  const struct                                                                \
  {                                                                           \
    DECLARE_HEADER(forthName);                                                \
    CodeWord codeWord;                                                        \
    Cell code[(xtCount) + 1];                                                 \
  } label = {                                                                 \
    { { (linkInit), (flags) + sizeof (forthName) - 1, (forthName) } },        \
    &fn_DOCOL, {

//-----------------------------------------------------------------------------
/**
 * Include the EXIT instruction in END_COLON(), since it is needed in 99.999%
 * of colon definitions and it is a pain to forget.
 */
#define END_COLON() XT(EXIT) }}

//-----------------------------------------------------------------------------
/**
 * Return the address of the link field of the dictionary header with the
 * specified (C identifier) label.
 */
#define LINK(label) (&label.u.h.link)

//-----------------------------------------------------------------------------
/**
 * Returns the eXecution Token (XT) of the word with the specified name.
 *
 * The XT is the address of the codeword field of the word's dictionary entry.
 * It is cast to type Cell.
 */
#define XT(name) ((Cell)(&name.codeWord))

//-----------------------------------------------------------------------------

#endif // TOMOKO_DICTIONARY_H

