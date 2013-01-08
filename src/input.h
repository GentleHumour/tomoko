#ifndef TOMOKO_INPUT_H
#define TOMOKO_INPUT_H

//-----------------------------------------------------------------------------
// Buffers.

/**
 * Size of the buffer that holds the next input word.
 */
#define TOMOKO_WORD_MAX 32

/**
 * Size of the buffer that holds the prompt string.
 */
#define TOMOKO_PROMPT_MAX 6

/**
 * A buffer for the next word read from input.
 */
extern char word[TOMOKO_WORD_MAX];

/**
 * A buffer for the prompt string.
 */
extern char prompt[TOMOKO_PROMPT_MAX];


//-----------------------------------------------------------------------------
/**
 * Print a formatted error message to stderr and exit(EXIT_FAILURE).
 */
extern void die(const char *format, ...);

//-----------------------------------------------------------------------------
/**
 * Open the file named by the ASCII NUL terminated string fileName as the 
 * current input source to get characters with KEY.  The previously opened 
 * source file is remembered and input coninues from there once the end of the
 * new file is reached.
 * @param fileName the file to open.
 */
extern void source(const char *fileName);

//-----------------------------------------------------------------------------
// Words.
//-----------------------------------------------------------------------------
/**
 * SOURCE ( fileName -- )
 *
 * This is the Forth word corresponding to source().
 *
 * Open the file named by the ASCII NUL terminated string fileName as the 
 * current input source to get characters with KEY.  The previously opened 
 * source file is remembered and input coninues from there once the end of the
 * new file is reached.
 */
extern void fn_SOURCE(void);

//-----------------------------------------------------------------------------
/**
 * ENDSOURCE ( -- )
 *
 * End the currently-being-sourced file (possibly before EOF has been reached).
 */
extern void fn_ENDSOURCE();

//-----------------------------------------------------------------------------
/**
 * WS? ( c -- flag )
 *
 * Return TRUE if the character c is a white-space character.
 */
extern void fn_WS(void);

//-----------------------------------------------------------------------------
/**
 * KEY ( -- c )
 *
 * Read a single character from the input stream.
 */
extern void fn_KEY(void);

//-----------------------------------------------------------------------------
/**
 * WORD ( -- address length )
 *
 * Read the next space-delineated word from input.  WS? defines what characters
 * are considered to be white space.  The word is read into the static buffer,
 * word.  The current implementation doesn't handle buffer overflow (buffer
 * size is 32).
 *
 * WORD treats backslash comments as equivalent to white-space (i.e. skips
 * them).
 *
 * TODO: raise an exception if the buffer would overflow.
 * TODO: therefore, I need to implement exceptions.
 */
extern void fn_WORD(void);

//-----------------------------------------------------------------------------
/**
 * >NUMBERIN ( uacc1 addr1 len1 base -- uacc2 addr2 len2 )
 *
 * Modelled after >NUMBER, as described in Starting Forth, but extended.
 * Read up to len1 digits, interpreted in the specified base, appearing at
 * addr1 and add them into unsigned accumulator uacc1.  Stop when all digits
 * have been consumed or an invalid character is found.
 *
 * Return the new accumulated value, the address of the first unused character
 * and the number of characters yet to be consumed.
 *
 * This word doesn't handle signed numbers.  It can be used to construct a
 * signed numeric input routine.
 */
extern void fn_XNUMBERIN(void);

//-----------------------------------------------------------------------------
/**
 * NUMBERIN ( addr1 len1 base -- n addr2 len2 )
 *
 * Interpret the string (addr1,len1) as a possibly signed number in the
 * specified base.  Return that number, along with (addr2,len2) specifying the
 * tail of the input string that could not be parsed (len2 == 0) if all of it
 * was parsed.
 */
extern void fn_NUMBERIN(void);

//-----------------------------------------------------------------------------
/**
 * INIT ( -- )
 *
 * Initialise the interpreter by loading definitions from "~/.tomoko"
 * (as in, source that file).
 */
extern void fn_INIT(void);

//-----------------------------------------------------------------------------

#endif // TOMOKO_INPUT_H

