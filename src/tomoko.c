//-----------------------------------------------------------------------------
//
// Compiling:
//   NOTE: This code is written assuming that the size of a pointer is the same
//   as that of a Cell, i.e. 32 bits.  Therefore it must be compiled in 32-bit
//   mode.  On 64-bit systems:
//     g++ -m32 -c tomoko.cpp
//     g++ -m32 -c input.cpp
//     g++ -m32 -c machine.cpp
//     g++ -m32 tomoko.o input.o machine.o -o tomoko -lreadline
//
//   Note that you may need to install the 32-bit glibc library.  On Fedora:
//     yum -y install glibc-devel.i686 readline.i386 readline-devel.i386
//
//-----------------------------------------------------------------------------

#include <stdio.h>

#include "dictionary.h"
#include "machine.h"
#include "input.h"

//-----------------------------------------------------------------------------
// Built-In Constants.
//
// Define 0 and 1 as constants, since they come up a lot, and it saves one cell
// in compiled code (the LIT word).
//
// CELL-1 and CELLMASK are used to compute the number of padding bytes appended
// after the name field of a dictionary entry in order to align the codeword
// to a cell boundary.  CELLMASK is ~3 on 32-bit hosts and ~1 on 16-bit hosts.

extern void fn_DOCOL(void);
extern void fn_DODOES(void);

DEF_CONST(NULL,              VERSION,     "VERSION",     100);       // 0.01.00
DEF_CONST(LINK(VERSION),     CELL,        "CELL",        sizeof (Cell));
DEF_CONST(LINK(CELL),        CELL_1,      "CELL-1",      sizeof (Cell) - 1);
DEF_CONST(LINK(CELL_1),      CELLMASK,    "CELLMASK",    ~(sizeof (Cell) - 1));
DEF_CONST(LINK(CELLMASK),    R0,          "R0",          (Cell)(&returnStack[RETURN_STACK_CELLS]));
DEF_CONST(LINK(R0),          DOCOL,       "DOCOL",       (Cell)(&fn_DOCOL));
DEF_CONST(LINK(DOCOL),       DODOES,      "DODOES",      (Cell)(&fn_DODOES));
DEF_CONST(LINK(DODOES),      F_IMMED,     "F_IMMED",     IMMEDIATE_BIT);
DEF_CONST(LINK(F_IMMED),     F_HIDDEN,    "F_HIDDEN",    HIDDEN_BIT);
DEF_CONST(LINK(F_HIDDEN),    F_LENMASK,   "F_LENMASK",   LENGTH_BITS);
DEF_CONST(LINK(F_LENMASK),   ZERO,        "0",           0);
DEF_CONST(LINK(ZERO),        ONE,         "1",           1);
DEF_CONST(LINK(ONE),         BL,          "BL",          ' '); // A space.
DEF_CONST_STRING(LINK(BL),   QUITPROMPT,  "QUITPROMPT",  "ok "); // Prompt from QUIT.

//-----------------------------------------------------------------------------
// These constants define Linux system calls and related flags.
// TODO: Actually make syscalls work.

//#include <asm-i386/unistd.h>	// you might need this instead
#include <asm/unistd.h>

DEF_CONST(LINK(QUITPROMPT),  SYS_EXIT,    "SYS_EXIT",    __NR_exit);
DEF_CONST(LINK(SYS_EXIT),    SYS_OPEN,    "SYS_OPEN",    __NR_open);
DEF_CONST(LINK(SYS_OPEN),    SYS_CLOSE,   "SYS_CLOSE",   __NR_close);
DEF_CONST(LINK(SYS_CLOSE),   SYS_READ,    "SYS_READ",    __NR_read);
DEF_CONST(LINK(SYS_READ),    SYS_WRITE,   "SYS_WRITE",   __NR_write);
DEF_CONST(LINK(SYS_WRITE),   SYS_CREAT,   "SYS_CREAT",   __NR_creat);
DEF_CONST(LINK(SYS_CREAT),   SYS_BRK,     "SYS_BRK",     __NR_brk);

// These constants lifed from /usr/include/asm-generic/fcntl.h. Duplicated since
// they are #defined there and JonesForth needs these names.
DEF_CONST(LINK(SYS_BRK),     O_RDONLY,    "O_RDONLY",    00000000);
DEF_CONST(LINK(O_RDONLY),    O_WRONLY,    "O_WRONLY",    00000001);
DEF_CONST(LINK(O_WRONLY),    O_RDWR,      "O_RDWR",      00000002);
DEF_CONST(LINK(O_RDWR),      O_CREAT,     "O_CREAT",     00000100);
DEF_CONST(LINK(O_CREAT),     O_EXCL,      "O_EXCL",      00000200);
DEF_CONST(LINK(O_EXCL),      O_TRUNC,     "O_TRUNC",     00001000);
DEF_CONST(LINK(O_TRUNC),     O_APPEND,    "O_APPEND",    00002000);
DEF_CONST(LINK(O_APPEND),    O_NONBLOCK,  "O_NONBLOCK",  00004000);

//-----------------------------------------------------------------------------
// Built-In Variables.
//
// Note that for simple compatibility with JonesForth, Tomoko defines HERE as 
// a variable that points to the next available dictionary cell.  In traditional
// Forths, that variable would be called CP, and HERE would be defined as:
// : HERE CP @ ;
//
// If CASE-SENSITIVE is TRUE, dictionary lookups are case sensitive.  Otherwise
// they are not.

DEF_VAR(LINK(O_NONBLOCK),    PIFA,        "^IFA",        0); // Address of IFA.
DEF_VAR(LINK(PIFA),          STATE,       "STATE",       0); // True if compiling.
DEF_VAR(LINK(STATE),         LATEST,      "LATEST",      0); // Set in main().
DEF_VAR(LINK(LATEST),        HERE,        "HERE",       (Cell)(&dictionary[0]));
DEF_VAR(LINK(HERE),          S0,          "S0",         (Cell)(&parameterStack[PARAMETER_STACK_CELLS]));
DEF_VAR(LINK(S0),            BASE,        "BASE",       10);
DEF_VAR(LINK(BASE),          CASE_SENSITIVE, "CASE-SENSITIVE", 1);

//-----------------------------------------------------------------------------
// Native Words.

#include "native.h"

DEF_CODE(LINK(CASE_SENSITIVE), EXIT,      "EXIT",        0);
DEF_CODE(LINK(EXIT),         BRANCH,      "BRANCH",      0);
DEF_CODE(LINK(BRANCH),       ZBRANCH,     "0BRANCH",     0);
DEF_CODE(LINK(ZBRANCH),      LIT,         "LIT",         0);
DEF_CODE(LINK(LIT),          LITSTRING,   "LITSTRING",   0);
DEF_CODE(LINK(LITSTRING),    LBRAC,       "[",           IMMEDIATE_BIT);
DEF_CODE(LINK(LBRAC),        RBRAC,       "]",           0);
DEF_CODE(LINK(RBRAC),        EXECUTE,     "EXECUTE",     0);
DEF_CODE(LINK(EXECUTE),      TICK,        "'",           0);
DEF_CODE(LINK(TICK),         IPFETCH,     "IP@",         0);
DEF_CODE(LINK(IPFETCH),      HALT,        "HALT",        0);
DEF_CODE(LINK(HALT),         SYSCALL0,    "SYSCALL0",    0);
DEF_CODE(LINK(SYSCALL0),     SYSCALL1,    "SYSCALL1",    0);
DEF_CODE(LINK(SYSCALL1),     SYSCALL2,    "SYSCALL2",    0);
DEF_CODE(LINK(SYSCALL2),     SYSCALL3,    "SYSCALL3",    0);
DEF_CODE(LINK(SYSCALL3),     FIND,        "FIND",        0);
DEF_CODE(LINK(FIND),         DROP,        "DROP",        0);
DEF_CODE(LINK(DROP),         SWAP,        "SWAP",        0);
DEF_CODE(LINK(SWAP),         DUP,         "DUP",         0);
DEF_CODE(LINK(DUP),          PICK,        "PICK",        0);
DEF_CODE(LINK(PICK),         STICK,       "STICK",       0);
DEF_CODE(LINK(STICK),        NTUCK,       "NTUCK",       0);
DEF_CODE(LINK(NTUCK),        OVER,        "OVER",        0);
DEF_CODE(LINK(OVER),         ROT,         "ROT",         0);
DEF_CODE(LINK(ROT),          NROT,        "-ROT",        0);
DEF_CODE(LINK(NROT),         DDROP,       "2DROP",       0);
DEF_CODE(LINK(DDROP),        DDUP,        "2DUP",        0);
DEF_CODE(LINK(DDUP),         DSWAP,       "2SWAP",       0);
DEF_CODE(LINK(DSWAP),        ZDUP,        "?DUP",        0);
DEF_CODE(LINK(ZDUP),         DSPFETCH,    "DSP@",        0);
DEF_CODE(LINK(DSPFETCH),     DSPSTORE,    "DSP!",        0);
DEF_CODE(LINK(DSPSTORE),     TOR,         ">R",          0);
DEF_CODE(LINK(TOR),          FROMR,       "R>",          0);
DEF_CODE(LINK(FROMR),        RSPFETCH,    "RSP@",        0);
DEF_CODE(LINK(RSPFETCH),     RSPSTORE,    "RSP!",        0);
DEF_CODE(LINK(RSPSTORE),     RDROP,       "RDROP",       0);
DEF_CODE(LINK(RDROP),        INCR,        "1+",          0);
DEF_CODE(LINK(INCR),         DECR,        "1-",          0);
DEF_NATIVE(LINK(DECR),       FOURPLUS,  fn_CELLPLUS,  "4+", 0);
DEF_NATIVE(LINK(FOURPLUS),   FOURMINUS, fn_CELLMINUS, "4-", 0);
DEF_CODE(LINK(FOURMINUS),    CELLPLUS,    "CELL+",       0);
DEF_CODE(LINK(CELLPLUS),     CELLMINUS,   "CELL-",       0);
DEF_CODE(LINK(CELLMINUS),    ADD,         "+",           0);
DEF_CODE(LINK(ADD),          SUB,         "-",           0);
DEF_CODE(LINK(SUB),          MUL,         "*",           0);
DEF_CODE(LINK(MUL),          DIV,         "/",           0);
DEF_CODE(LINK(DIV),          MOD,         "MOD",         0);
DEF_CODE(LINK(MOD),          NEGATE,      "NEGATE",      0);
DEF_CODE(LINK(NEGATE),       DIVMOD,      "/MOD",        0);
DEF_CODE(LINK(DIVMOD),       EQ,          "=",           0);
DEF_CODE(LINK(EQ),           NE,          "<>",          0);
DEF_CODE(LINK(NE),           LT,          "<",           0);
DEF_CODE(LINK(LT),           GT,          ">",           0);
DEF_CODE(LINK(GT),           LE,          "<=",          0);
DEF_CODE(LINK(LE),           GE,          ">=",          0);
DEF_CODE(LINK(GE),           EQ0,         "0=",          0);
DEF_CODE(LINK(EQ0),          NE0,         "0<>",         0);
DEF_CODE(LINK(NE0),          LT0,         "0<",          0);
DEF_CODE(LINK(LT0),          GT0,         "0>",          0);
DEF_CODE(LINK(GT0),          LE0,         "0<=",         0);
DEF_CODE(LINK(LE0),          GE0,         "0>=",         0);
DEF_CODE(LINK(GE0),          AND,         "AND",         0);
DEF_CODE(LINK(AND),          OR,          "OR",          0);
DEF_CODE(LINK(OR),           XOR,         "XOR",         0);
DEF_CODE(LINK(XOR),          INVERT,      "INVERT",      0);
DEF_CODE(LINK(INVERT),       STORE,       "!",           0);
DEF_CODE(LINK(STORE),        FETCH,       "@",           0);
DEF_CODE(LINK(FETCH),        PLUSSTORE,   "+!",          0);
DEF_CODE(LINK(PLUSSTORE),    MINUSSTORE,  "-!",          0);
DEF_CODE(LINK(MINUSSTORE),   CSTORE,      "C!",          0);
DEF_CODE(LINK(CSTORE),       CFETCH,      "C@",          0);
DEF_CODE(LINK(CFETCH),       CCOPY,       "C@C!",        0);
DEF_CODE(LINK(CCOPY),        CMOVE,       "CMOVE",       0);
DEF_CODE(LINK(CMOVE),        FILL,        "FILL",        0);
DEF_CODE(LINK(FILL),         WS,          "WS?",         0);
DEF_CODE(LINK(WS),           KEY,         "KEY",         0);
DEF_CODE(LINK(KEY),          WORD,        "WORD",        0);
DEF_CODE(LINK(WORD),         XNUMBERIN,   ">NUMBERIN",   0);
DEF_CODE(LINK(XNUMBERIN),    NUMBERIN,    "NUMBERIN",    0);
DEF_CODE(LINK(NUMBERIN),     INIT,        "INIT",        0);
DEF_CODE(LINK(INIT),         EMIT,        "EMIT",        0);
DEF_CODE(LINK(EMIT),         TELL,        "TELL",        0);
DEF_CODE(LINK(TELL),         DOT,         ".",           0);
DEF_CODE(LINK(DOT),          MSLEEP,      "MSLEEP",      0);

//-----------------------------------------------------------------------------
// String literals as inline code in hand-compiled Forth.

#include "string_lit.h"

//-----------------------------------------------------------------------------
// Hand-compiled colon definitions.
//-----------------------------------------------------------------------------
/**
 * NUMBER ( addr len -- value unparsed-char-count )
 * 
 * For compatibility with the JonesForth number input routine.
 */
BEGIN_COLON(LINK(MSLEEP), NUMBER, "NUMBER", 0, 5)
  XT(BASE), XT(FETCH),              // ( addr len base ) Set up to call NUMBERIN.
  XT(NUMBERIN),                     // ( n addr2 len2 )
  XT(SWAP), XT(DROP),               // ( n len2 )
END_COLON();

//-----------------------------------------------------------------------------
/**
 * CR ( -- )
 *
 * Emit a newline.  This is traditionally called CR.
 */
BEGIN_COLON(LINK(NUMBER), CR, "CR", 0, 3)
  XT(LIT), '\n', XT(EMIT),
END_COLON();

//-----------------------------------------------------------------------------
/**
 * SPACE ( -- )
 *
 * Emit a space.
 */
BEGIN_COLON(LINK(CR), SPACE, "SPACE", 0, 3)
  XT(BL), XT(EMIT),
END_COLON();

//-----------------------------------------------------------------------------
/**
 * \ ( -- )
 *
 * Backslash comments. All characters are skipped until the end of the line is
 * reached. This word is immediate, so that it executes even when compiling.
 */
BEGIN_COLON(LINK(SPACE), BSCOMMENT, "\\", IMMEDIATE_BIT, 15)
  XT(KEY), XT(DUP),                 // ( c c )
  XT(LIT), 13, XT(NE),              // ( c flag ) Carriage return?
  XT(ZBRANCH), 7 * sizeof (Cell),   // ( c ) Jump forward to DROP.
  XT(LIT), 10, XT(NE),              // ( flag ) Line feed?
  XT(ZBRANCH), 4 * sizeof (Cell),   // ( ) Jump forward to EXIT.
  XT(BRANCH), -13 * sizeof (Cell),  // Jump back to KEY.
  XT(DROP),
END_COLON();

//-----------------------------------------------------------------------------
/**
 * CHAR <word>
 *
 * Return the first character of the subsequent word.
 */
BEGIN_COLON(LINK(BSCOMMENT), CHAR, "CHAR", 0, 3)
  XT(WORD),                         // ( addr len ) Read a word.
  XT(DROP), XT(CFETCH),             // ( c ) Return the first character, at addr.
END_COLON();

//-----------------------------------------------------------------------------
/**
 * SP# ( -- depth )
 *
 * Return the depth of the parameter stack in cells.
 */
BEGIN_COLON(LINK(CHAR), SPHASH, "SP#", 0, 7)
  XT(S0), XT(FETCH),                // ( S0 )
  XT(DSPFETCH), XT(SUB),            // ( S0-SP ) Depth in bytes.
  XT(CELL), XT(DIV),                // Depth in cells.  Includes answer itself.
  XT(DECR),                         // Discount the cell used to hold the depth.
END_COLON();

//-----------------------------------------------------------------------------
/**
 * .S ( -- )
 *
 * Non-destructively display the contents of the stack, in the form:
 * <n> sn ... s2 s1 s0
 *
 * where n is the number of items and s0 is the TOS.  Handy for debugging,
 * so I won't wait for full self-hosting capability before defining this.
 */
BEGIN_COLON(LINK(SPHASH), DOT_S, ".S", 0, 23)
  XT(SPHASH),                       // ( depth )
  XT(DUP),                          // ( depth depth )
  XT(LIT), '<', XT(EMIT),           // Show stack depth.
  XT(DOT),                          // ( depth )
  XT(LIT), '>', XT(EMIT),

  XT(DUP),                          // ( depth depth )
  XT(ZBRANCH), 8 * sizeof (Cell),   // Loop done?

  XT(DUP),                          // ( ...stuff... index index ) Note: 0 ==> index itself.
  XT(PICK),                         // ( index si ) Grab item at index.
  XT(SPACE),                        // Precede item by space.
  XT(DOT),                          // ( index )
  XT(DECR),                         // ( index -- index-1 )
  XT(BRANCH), -9 * sizeof (Cell),
  XT(CR),
  XT(DROP),
END_COLON();

//-----------------------------------------------------------------------------
/**
 * >CFA ( lfa -- cfa )
 *
 * Convert Link Field Address to Code Field Address
 * (skip over length and name).  If lfa is 0, return 0.
 */
BEGIN_COLON(LINK(DOT_S), TOCFA, ">CFA", 0, 15)
  XT(DUP),
  XT(ZBRANCH), 13 * sizeof (Cell),  // If lfa == 0, skip all this and return 0.
  XT(CELLPLUS),                     // ( ^link -- ^len ) Point to length.
  XT(DUP), XT(CFETCH),              // ( ^len len ) Get length.
  XT(F_LENMASK), XT(AND),           // ( ^len len ) Mask out flags.
  XT(INCR),                         // ( ^len len+1 ) Account for length byte...
  XT(INCR),                         // ( ^len len+2 ) ...and ASCII NUL terminator.
  XT(ADD),                          // ( ^end ) Address one past end of name.

  XT(CELL_1), XT(ADD),              // ( (^end+CELL-1) ) If ^end is not aligned,
                                    //                   bump into next CELL.
  XT(CELLMASK), XT(AND),            // ( ^cfa ) Re-align to CELL boundary.
END_COLON();

//-----------------------------------------------------------------------------
/**
 * >DFA ( lfa -- pfa )
 *
 * Convert Link Field Address to Parameter Field Address
 * (skip over length, name and codeword).  If lfa is 0, return 0.
 * 
 * For compatibility with JonesForth, this is called "to-DFA" 
 * ("Data Field Address"?).
 */
BEGIN_COLON(LINK(TOCFA), TODFA, ">DFA", 0, 5)
  XT(TOCFA), XT(DUP),               // ( lfa -- cfa cfa ) Can be 0, if lfa is 0.
  XT(ZBRANCH), 2 * sizeof (Cell),   // If 0, skip CELL+, returning 0.
  XT(CELLPLUS),
END_COLON();

//-----------------------------------------------------------------------------
/**
 * ALLOT ( count -- )
 *
 * Increment the address of the next available dictionary byte (stored in HERE)
 * by the specified count of bytes.
 */
BEGIN_COLON(LINK(TODFA), ALLOT, "ALLOT", 0, 2)
  XT(HERE), XT(PLUSSTORE),
END_COLON();

//-----------------------------------------------------------------------------
/**
 * , ( n -- )
 *
 * Compile a cell to the dictionary.
 */
BEGIN_COLON(LINK(ALLOT), COMMA, ",", 0, 5)
  XT(HERE), XT(FETCH), XT(STORE),   // Store cell where HERE points.
  XT(CELL), XT(ALLOT),              // Advance HERE by cell size.
END_COLON();

//-----------------------------------------------------------------------------
/**
 * C, ( n -- )
 *
 * Compile a byte to the dictionary.
 */
BEGIN_COLON(LINK(COMMA), CCOMMA, "C,", 0, 5)
  XT(HERE), XT(FETCH), XT(CSTORE),  // Store byte where HERE points.
  XT(ONE), XT(ALLOT),               // Advance HERE by byte size.
END_COLON();

//-----------------------------------------------------------------------------
/**
 * ERASE ( addr n -- )
 *
 * Fill n bytes, starting at the specified address, with zero.
 */
BEGIN_COLON(LINK(CCOMMA), ERASE, "ERASE", 0, 2)
  XT(ZERO), XT(FILL),
END_COLON();

//-----------------------------------------------------------------------------
/**
 * CREATE ( -- )
 *
 * Create the dictionary header for a word, comprising the link, len and name
 * fields.  Always append an ASCII NUL terminator, to be consistent with
 * the equivalent C macro, and then add padding bytes so that the total size of
 * the len byte, name, NUL and padding is a multiple of the cell size.
 *
 * The traditional definition of this word, it seems, calls WORD to read the
 * name, rather than already having the name on the stack.
 *
 * TODO: This definition might be able to be optimised a bit since I changed how
 * HERE works to be compatible with JonesForth.
 */
BEGIN_COLON(LINK(ERASE), CREATE, "CREATE", 0, 33)
  XT(WORD),                         // ( addr len ) Name.
  XT(HERE), XT(FETCH),              // ( addr len here ) HERE is the LFA.  Save it.
  XT(LATEST), XT(FETCH), XT(COMMA), // ( addr len here ) Put LATEST in the link.
  XT(LATEST), XT(STORE),            // ( addr len ) Set LATEST to point to link.
  XT(DUP), XT(CCOMMA),              // ( addr len ) Store len byte.
  XT(HERE), XT(FETCH), XT(SWAP),    // ( addr here len ) Set up to call CMOVE.
  XT(DUP), XT(ALLOT),               // Advance HERE by len.
  XT(CMOVE),                        // ( ) Copy len bytes from addr to here.
  XT(ZERO), XT(CCOMMA),             // ASCII NUL terminator of name.
  XT(HERE), XT(FETCH),              // ( here )
  XT(CELL_1), XT(ADD),              // ( (here+3)&~3 )  OR  ( (here+1)&~1 )
  XT(CELLMASK), XT(AND),            //    ^ 32-bit Cell        ^ 16-bit Cell
  XT(HERE), XT(FETCH), XT(SUB),     // ( padding ) Number of padding bytes (can be 0).
  XT(HERE), XT(FETCH), XT(SWAP),    // ( here padding ) Set up for ERASE.
  XT(DUP), XT(ALLOT),               // Advance HERE by padding count.
  XT(ERASE),                        // Fill padding with zeroes.
END_COLON();

//-----------------------------------------------------------------------------
/**
 * IMMEDIATE ( -- )
 *
 * Toggle the immediate flag of the latest word.
 */
BEGIN_COLON(LINK(CREATE), IMMEDIATE, "IMMEDIATE", IMMEDIATE_BIT, 9)
  XT(LATEST), XT(FETCH),            // ( ^link ) Link addr of latest defn.
  XT(CELLPLUS),                     // ( ^link -- ^len )
  XT(DUP),                          // ( ^len ^len )
  XT(CFETCH),                       // ( ^len len )
  XT(F_IMMED), XT(XOR),             // ( ^len len^IMMEDIATE )
  XT(SWAP), XT(CSTORE),             // Store in len byte.
END_COLON();

//-----------------------------------------------------------------------------
/**
 * HIDDEN ( lfa -- )
 *
 * Toggle the hidden flag of the word whose LFA is supplied.
 */
BEGIN_COLON(LINK(IMMEDIATE), HIDDEN, "HIDDEN", 0, 7)
  XT(CELLPLUS),                     // ( ^link -- ^len )
  XT(DUP),                          // ( ^len ^len )
  XT(CFETCH),                       // ( ^len len )
  XT(F_HIDDEN), XT(XOR),            // ( ^len len^HIDDEN )
  XT(SWAP), XT(CSTORE),             // Store in len byte.
END_COLON();

//-----------------------------------------------------------------------------
/**
 * HIDE <word>
 *
 * Toggle the hidden flag of the next word of input.
 */
BEGIN_COLON(LINK(HIDDEN), HIDE, "HIDE", 0, 3)
  XT(WORD), XT(FIND), XT(HIDDEN),
END_COLON();

//-----------------------------------------------------------------------------
// This definition of ' (TICK) only works if WORD is buffered in such a way that
// calling it does not (immediately) stomp on the memory used to hold the
// previously read word.  Here, we are using a static buffer that can only
// hold one word at a time, so we use the native version of TICK.
///**
// * ' <word> ( -- addr )
// *
// * Return the CFA of the next word of input.
// */
//BEGIN_COLON(LINK(HIDE), TICK, "'", 0, 3)
//  XT(WORD), XT(FIND), XT(TOCFA),
//END_COLON();

//-----------------------------------------------------------------------------
/**
 * WORDS ( -- )
 *
 * List all words in the dictionary, except hidden words.  This could easily
 * be done natively, but it was initially done as an exercise in testing
 * (and debugging) the interpreter.
 */
BEGIN_COLON(LINK(HIDE), WORDS, "WORDS", 0, 29)
  XT(LATEST),                       // ( ^link )
  XT(FETCH),                        // ( ^link ) Loop start.
  XT(DUP),                          // ( ^link ^link )
  XT(ZBRANCH), 23 * sizeof (Cell),  // If link is NULL, exit loop.
  XT(DUP),                          // ( ^link ^link )
  XT(CELLPLUS),                     // ( ^link ^len ) Skip past link field.
  XT(DUP),                          // ( ^link ^len ^len )
  XT(CFETCH),                       // ( ^link ^len len )
  XT(F_HIDDEN), XT(AND),            // ( ^link ^len flag )
  XT(ZBRANCH), 4 * sizeof (Cell),   // ( ^link ^len) If HIDDEN, skip the following...
  XT(DROP),                         // ( ^link )
  XT(BRANCH), -14 * sizeof (Cell),  // Branch back to FETCH (loop start).
  XT(DUP), XT(CFETCH),              // ( ^link ^len len )
  XT(F_LENMASK), XT(AND),           // ( ^link ^len len ) Mask out flags.
  XT(SWAP),                         // ( ^link len ^len )
  XT(INCR),                         // ( ^link len ^name )
  XT(SWAP),                         // ( ^link ^name len )
  XT(TELL),                         // Show name.
  XT(SPACE),
  XT(BRANCH), -25 * sizeof (Cell),  // Branch back to FETCH (loop start).
  XT(DROP),
  XT(CR),
END_COLON();

//-----------------------------------------------------------------------------
/**
 * INTERPRET
 *
 * Read one word of input and either compile or execute it, depending on the
 * value of STATE.  This word came out very convoluted.  There must be
 * a better way...
 */
BEGIN_COLON(LINK(WORDS), INTERPRET, "INTERPRET", 0, 51)
  XT(WORD),                         // ( addr len ) Read word.
  XT(DDUP),                         // ( addr len addr len )
  XT(FIND), XT(DUP),                // ( addr len lfa lfa ) Find LFA, or 0.
  XT(ZBRANCH), 24 * sizeof (Cell),  // If not in dictionary, skip to #4.
                                    // ( addr len lfa ) Word is in dictionary.
  XT(DUP), XT(TOCFA), XT(SWAP),     // ( addr len cfa lfa ) Execution token.
  XT(STATE), XT(FETCH),             // Are we compiling?
  XT(ZBRANCH), 9 * sizeof (Cell),   // If not then skip to #1, execution.
                                    // ( addr len cfa lfa ) We are compiling...
  XT(CELLPLUS), XT(CFETCH),         // ( addr len cfa length ) Length/flags byte.
  XT(F_IMMED), XT(AND),             // ( addr len cfa immediate? ) Immediate bit.
  XT(ZBRANCH), 8 * sizeof (Cell),   // If not immediate, branch to #3, compilation.
                                    // ( addr len cfa )
  XT(BRANCH),  2 * sizeof (Cell),   // Word is immediate; jump to #2.

// #1                               // ( addr len cfa lfa ) We are executing.
  XT(DROP),                         // ( addr len cfa )
// #2                               // ( addr len cfa )
  XT(ROT),                          // ( cfa addr len ) Word may want to use stack.
  XT(DDROP),                        // ( cfa ) Don't need addr and len.
  XT(EXECUTE),                      // Execute the word.
  XT(EXIT),                         // Return.

// #3                               // ( addr len cfa ) Compilation.
  XT(COMMA),                        // ( addr len ) Compile into current dictionary definition.
  XT(DDROP),                        // () Don't need addr and len.
  XT(EXIT),                         // Return.

// #4                               // ( addr len 0 ) Not in the dictionary.
  XT(DROP),                         // ( addr len )
  XT(BASE), XT(FETCH),              // ( addr len base )
  XT(NUMBERIN),                     // ( num addr2 len2 ) Parse as number.
  XT(DUP),                          // ( num addr2 len2 len2 )
  XT(ZBRANCH), 7 * sizeof (Cell),   // If a valid number, branch to #5.
                                    // ( num addr2 len2 ) Invalid number.
  XT(TELL),                         // Display what couldn't be parsed.
  XT(DROP),                         // ()
  XT(LIT), '?', XT(EMIT),           // Half-baked error message.
  XT(EXIT),                         // Return.

// #5                               // ( num addr2 len2 ) Number is valid.
  XT(DDROP),                        // ( num )
  XT(STATE), XT(FETCH),             // Are we compiling?
  XT(ZBRANCH), 5 * sizeof (Cell),   // If not then branch to #6
                                    // ( num ) Compiling.
  XT(LIT), XT(LIT), XT(COMMA),      // Compile LIT.
  XT(COMMA),                        // Compile the number.

                                    // Else, not compiling... so just leave the
// #6                               // number on TOS.
END_COLON();                        // Return.

//-----------------------------------------------------------------------------
/**
 * QUIT
 *
 * Reset the return stack and repeatedly call INTERPRET.
 */
BEGIN_COLON(LINK(INTERPRET), QUIT, "QUIT", 0, 5)
  XT(R0), XT(RSPSTORE),             // Initialise return stack.
  XT(INTERPRET),
  XT(BRANCH),  -4 * sizeof (Cell),  // Branch back to start.
END_COLON();

//-----------------------------------------------------------------------------
/**
 * :
 *
 * Define : (COLON) to create a new dictionary header, mark the dictionary entry
 * as HIDDEN, and start compiling.
 */
BEGIN_COLON(LINK(QUIT), COLON, ":", 0, 7)
  XT(CREATE),                       // Create dictionary header.
  XT(DOCOL), XT(COMMA),             // Set codeword to point to fn_DOCOL().
  XT(LATEST), XT(FETCH), XT(HIDDEN),// Hide this definition, for now.
  XT(RBRAC),                        // Start compiling.
END_COLON();

//-----------------------------------------------------------------------------
/**
 * ;
 *
 * Define ; (SEMICOLON) to compile EXIT and reveal the completed definition.
 * Note that it is an IMMEDIATE word.
 */
BEGIN_COLON(LINK(COLON), SEMICOLON, ";", IMMEDIATE_BIT, 7)
  XT(LIT), XT(EXIT), XT(COMMA),     // Append EXIT to the colon definition.
  XT(LATEST), XT(FETCH), XT(HIDDEN),// Reveal the completed definition.
  XT(LBRAC),                        // Stop compiling.
END_COLON();

//-----------------------------------------------------------------------------

BEGIN_COLON(LINK(SEMICOLON), MAIN, "MAIN", 0, 2)
  XT(INIT),
  XT(QUIT),
END_COLON();

//-----------------------------------------------------------------------------
/**
 * Main program.
 */
int main()
{
  // Set LATEST to the LFA of the last word defined.
  LATEST_value = (Cell) LINK(MAIN);

  // Start in MAIN.
  ip = (CodeWord**) MAIN.code;

  for (;;)
  {
    NEXT();
  }
  return 0;
} // main

//-----------------------------------------------------------------------------
