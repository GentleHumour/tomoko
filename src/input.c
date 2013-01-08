//-----------------------------------------------------------------------------
// Input
//-----------------------------------------------------------------------------

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <readline/readline.h>
#include <linux/limits.h>       // For PATH_MAX.

#include "input.h"
#include "machine.h"

//-----------------------------------------------------------------------------

char word[TOMOKO_WORD_MAX];
char prompt[TOMOKO_PROMPT_MAX] = "> ";
 
//-----------------------------------------------------------------------------

void die(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  fflush(stderr);
  exit(EXIT_FAILURE);
} // die

//-----------------------------------------------------------------------------
/**
 * Number of characters to save (maximum) from a filename, including NUL.
 * The directory names are omitted.
 */
#define TOMOKO_PATH_MAX 16

/**
 * Maximum size of an input line before it is truncated (buffer size).
 */
#define TOMOKO_LINE_MAX 256

/**
 * The maximum number of simultaneous nested file inclusions using SOURCE.
 */
#define TOMOKO_MAX_SOURCES 8

//-----------------------------------------------------------------------------
/**
 * Records information about each file currently opened by the SOURCE statement.
 *
 * The terminal is counted as the first (0th) source, but it's file handle is 
 * NULL.  An array of these structures is used as a stack to record all of the
 * state needed to resume reading from the same position after a whole file has
 * been read using SOURCE.
 */
typedef struct 
{
  /**
   * The file handle to read from.  For the first InputSource, sources[0], the
   * handle is NULL and input is taken from the keyboard using readline(3).
   */
  FILE *handle;

  /**
   * Current line number, starting at 1.  Used in error reporting.
   */
  int lineNumber;

  /**
   * Index of the next character to read from lineBuffer.
   */
  int lineIndex;
  
  /**
   * The most recently read line from a file, using SOURCE, or the terminal.
   */
  char lineBuffer[TOMOKO_LINE_MAX];
  
  /** 
   * Save the last part of the filename.  Directories are left out.
   * PATH_MAX can be 4096. Not storing all of that.
   */
  char fileName[TOMOKO_PATH_MAX];
} InputSource;

//-----------------------------------------------------------------------------
/**
 * The stack of open sources.
 */
static InputSource sources[TOMOKO_MAX_SOURCES];

/**
 * The index of the currently used source in the sources array.
 *
 * There is initially a single open source, representing the the terminal.
 * It's handle is NULL, since input is read using readline(3) rather than from
 * a file.
 */
static int currentSource = 0;

//-----------------------------------------------------------------------------

void source(const char *fileName)
{
  // If we have not hit the limit on open sources.
  if (currentSource + 1 < TOMOKO_MAX_SOURCES)
  {
    InputSource *nextSource = &sources[currentSource + 1];

    // Try to open the file.
    nextSource->handle = fopen(fileName, "r");
    if (nextSource->handle != NULL)
    {
      // Advance the index to reference he new source.
      ++currentSource;
    
      nextSource->lineNumber = 1;
      nextSource->lineIndex = 0;

      // NUL terminate the line buffer, forcing a read.
      nextSource->lineBuffer[0] = '\0';
      
      // Save the file part (excluding directories) of the name.
      // TODO: This ('/' as separator) is not portable. Resolve.
      // Find the last '/'
      const char *lastSlash = strrchr(fileName, '/');

      // Work out the start of the non-directory part of fileName.
      const char *namePart = (lastSlash == NULL) ? fileName
                                                 : lastSlash + 1;
      // Leave the last character of fileName field untouched (0) always.
      strncpy(nextSource->fileName, namePart, TOMOKO_PATH_MAX-1);
    }
    else // Failed to open the file to read.
    {
      // TODO: Better error handling.
      die("could not open source \"%s\"", fileName);
    }
  }
  else // We have too many open sources to open another.
  {
    // TODO: Better error handling.
    die("too many open sources to open \"%s\"", fileName);
  }  
} // source

//-----------------------------------------------------------------------------

void fn_SOURCE(void)
{
  const char *fileName = (const char*) STACK_POP(sp);
  source(fileName);
} // fn_SOURCE

//-----------------------------------------------------------------------------

void fn_ENDSOURCE()
{
  // If we are actually reading from a file...
  if (currentSource > 0)
  {
    fclose(sources[currentSource].handle);
    
    // Refer to the previous source.
    --currentSource;
  }
} // fn_ENDSOURCE

//-----------------------------------------------------------------------------
/**
 * Return TRUE if the character c is a white-space character.
 *
 * A space, and any character code less than 32 is considered to be white-space.
 */
static int isWS(char c)
{
  return c <= 32;
}

//-----------------------------------------------------------------------------

void fn_WS(void)
{
  Cell c = STACK_POP(sp);
  STACK_PUSH(sp, BOOLEAN(isWS(c)));
}

//-----------------------------------------------------------------------------
/**
 * Return the next character from the input stream (the current SOURCEd file).
 * 
 * This is the native implementation behind the KEY word.
 */
static char charIn(void)
{
  InputSource *source = &sources[currentSource];
  
  // Extract the next character from the buffer and advance the index.
  Cell c = source->lineBuffer[source->lineIndex++];
  
  // Have we reached the end of the buffer?
  if (c != '\0')
  {
    // No. Return the character.
    return c;
  }
  else // End of the current buffered line. Read another.
  {
    // Reset the index pointing to the next character to return.
    source->lineIndex = 0;
  
    // Are we reading from a file?
    if (source->handle != NULL)
    {
      char *result = fgets(source->lineBuffer, sizeof source->lineBuffer, source->handle);
      
      // If we reached the end of the file...
      if (result != source->lineBuffer)
      {
        // Close the current source.
        fn_ENDSOURCE();
      }
    }
    else // We are reading from the terminal.
    {
     // Read a line.
      char *line = readline(prompt);
      if (line == NULL)
      {
        // User entered Ctrl-D interactively. Just quit.
        exit(EXIT_SUCCESS);
      }
      
      // Copy the line into the InputSource's buffer and add '\n' terminator.
      strncpy(source->lineBuffer, line, sizeof source->lineBuffer - 2);
      source->lineBuffer[sizeof source->lineBuffer - 2] = '\0';
      strcat(source->lineBuffer, "\n");
      
      // Deallocate the buffer allocated by readline.
      free(line);
    }
    
    // Now retry the character read.
    return charIn();
  }
} // charIn

//-----------------------------------------------------------------------------
/**
 * Put the most recently read character back into the input buffer.
 *
 * 
 */
static void unReadChar()
{
  // After a call to charIn() the current InputSource should always have a 
  // buffer with at least one character and lineIndex > 0.
  --sources[currentSource].lineIndex;
}

//-----------------------------------------------------------------------------

void fn_KEY(void)
{
  STACK_PUSH(sp, (Cell) charIn());
}

//-----------------------------------------------------------------------------
// For convenience of debugging, we append a NUL terminator to the word, even
// though Forth doesn't need it.

void fn_WORD(void)
{
  char c;
  
  // Skip white space.
  do 
  {
    c = charIn();
  } while (isWS(c));
  
  size_t length = 0;
  do 
  {
    word[length++] = c;
    c = charIn();
  } while (! isWS(c) && length < sizeof word - 1);

  // TODO: Deal intelligently with words longer than the buffer size. Currently
  // a full buffer is handled as if a white space was found.
  if (isWS(c))
  {
    // If a white space terminated the word (normal, non-overflow case), put
    // the space back into the input to be read again.  This is specifically
    // to allow \ (the comment word) at the end of a line to read the newline
    // character.
    unReadChar();
  }

  // NUL terminator.
  word[length] = '\0';
  STACK_PUSH(sp, word);
  STACK_PUSH(sp, length);
} // fn_WORD

//-----------------------------------------------------------------------------

void fn_XNUMBERIN(void)
{
  // Valid bases are between 1 and 36.
  Cell base = STACK_POP(sp);
  if (base < 1 || base > 36)
  {
    // TODO: should really barf with an exception here.  For now, do nothing.
  }
  else
  {
    Cell len = STACK_POP(sp);
    const char *addr = (const char *) STACK_POP(sp);
    UCell uacc = STACK_POP(sp);
    while (len > 0)
    {
      char digit = toupper(*addr);

      // Set to an invalid value to exit loop if no valid digit found.
      UCell digitValue = 99;

      // 0 to 9?
      if (digit >= '0' && digit <= '9')
      {
        digitValue = digit - '0';
      }
      // A to Z?
      else if (digit >= 'A' && digit <= 'Z')
      {
        digitValue = 10 + digit - 'A';
      }

      // Digit valid within base?
      if (digitValue < base)
      {
        uacc = (uacc * base) + digitValue;
        ++addr;
        --len;
      }
      else
      {
        break;
      }
    } // while
    STACK_PUSH(sp, uacc);
    STACK_PUSH(sp, addr);
    STACK_PUSH(sp, len);
  } // else
} // fn_XNUMBERIN

//-----------------------------------------------------------------------------

void fn_NUMBERIN(void)
{
  // Sign of final result.
  Cell sign = 1;
  Cell base = STACK_POP(sp);
  Cell len = STACK_POP(sp);
  const char *addr = (const char *) STACK_POP(sp);
  if (*addr == '-' || *addr == '+')
  {
    if (*addr == '-')
    {
      sign = -1;
    }

    // Skip sign character.
    ++addr;
    --len;
  }

  // Push accumulator and other args to >NUMBERIN.
  STACK_PUSH(sp, 0);
  STACK_PUSH(sp, addr);
  STACK_PUSH(sp, len);
  STACK_PUSH(sp, base);
  fn_XNUMBERIN();

  // Multiply the sign into the final result.
  *STACK_ADDR(sp,2) *= sign;
} // fn_NUMBERIN

//-----------------------------------------------------------------------------

void fn_INIT(void)
{
  char fileName[PATH_MAX];
  const char *home = getenv("HOME");
  if (home != NULL)
  {
    strcpy(fileName, home);
    strcat(fileName, "/.tomoko");
    
    source(fileName);
  }
  else
  {  
    die("$HOME is not set. Quitting.\n");
  }
} // fn_INIT

//-----------------------------------------------------------------------------


