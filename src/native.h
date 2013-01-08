//-----------------------------------------------------------------------------
// Native implementations of miscellaneous Forth words.
//-----------------------------------------------------------------------------

#ifndef TOMOKO_NATIVE_H
#define TOMOKO_NATIVE_H

//-----------------------------------------------------------------------------
// Interpreter basics.
//-----------------------------------------------------------------------------
/**
 * DOCOL is the native word that is the body of colon definitions.
 *
 * It pushes the IP on the return stack and then sets it to refer to the
 * Parameter Field Address (PFA) of the word.
 */
extern void fn_DOCOL(void);

//-----------------------------------------------------------------------------
/**
 * DODOES is the native word that implements the run-time behaviour of
 * words defined using a defining word specified by <BUILDS and DOES>.
 *
 * At run-time, DODOES pushes the PFA onto the stack and loads the contents of
 * the Instruction Field Address (IFA) (see diagram below) into the Forth
 * Instruction Pointer (IP) in order to execute the code that follows DOES>.
 *
 * The IFA does not exist in ordinary native words or colon definitions in the
 * dictionary.  It occupies the position normally taken by the first cell of
 * the Parameter Field in other words.  The dictionary structure of a defined
 * word, X', and its corresponding defining word, X, look like this:
 *
 *      Defined Word                      Defining Word
 * | link                 |           | link                |
 * | len | X' NUL padding |           | len | X NUL padding |
 * | codeword = DODOES    |           | codeword = DOCOL    |
 * | ifa (pointer)        |----+      | ' <BUILDS           |
 * | pfa                  |    |      | <code to create>    |
 *                             |      | ' EXIT              |
 *                             +----->| <code after DOES>>  |
 *
 * DODOES does the following:
 * # Saves the IP on the return stack, for use by EXIT.
 * # Pushes the PFA onto the parameter stack.  This is 2 cells past the CFA.
 * # Sets IP to the value in IFA.
 *
 * Note that it would be possible to include an IFA field in all dictionary
 * headers, not just those of words defined using a Forth defining word.  In
 * order to do so, the hand-compilation macros would need to define that
 * field and initialise it to point to the code cells in the Parameter Field.
 * (This initialisation has been verified to work under G++ and GCC.)  However,
 * for most native words, the IFA would be unused, and for colon definitions,
 * it would be redundant, since the Parameter Field is always the cell after
 * the CFA.  So, for native words and colon definitions, adding an IFA would
 * make a consistent dictionary header but would waste a cell.  An alternative
 * way of achieving this consistency (implemented by Tomoko) is to ensure that
 * words that reflect the structure of dictionary headers, such as >CFA and
 * >PFA know of the existence of the IFA cell in the case when the codeword
 * (XT) is DODOES.
 */
extern void fn_DODOES(void);

//-----------------------------------------------------------------------------
/**
 * EXIT is the inverse of DOCOL.  It is compiled at the end of every colon
 * definition to pop the old ip off the return stack and continue in the
 * calling word.
 */
extern void fn_EXIT(void);

//-----------------------------------------------------------------------------
/**
 * Unconditionally branch by adding a literal offset to ip.
 *
 * The offset is added to ip after it has advanced past the BRANCH XT, meaning
 * that *ip refers to the offset when BRANCH executes.
 */
extern void fn_BRANCH(void);

//-----------------------------------------------------------------------------
/**
 * 0BRANCH ( flag -- )
 *
 * Conditionally branch if TOS is 0, by adding a literal offset to ip.
 *
 * The offset is added to ip after it has advanced past the BRANCH XT, meaning
 * that *ip refers to the offset when BRANCH executes.
 */
extern void fn_ZBRANCH(void);

//-----------------------------------------------------------------------------
/**
 * Push a literal onto the stack.
 *
 * The literal occupies the Cell immediately following the XT for LIT.
 */
extern void fn_LIT(void);

//-----------------------------------------------------------------------------
/**
 * LITSTRING ( -- addr len )
 *
 * Push a string literal onto the stack.  The length is stored as a full cell
 * in the dictionary, immediately after the execution token for LITSTRING. 
 * Then the string characters follow, and finally some optional padding to 
 * align to the next cell boundary.  The implementation has to branch over the 
 * stored length cell, characters and padding.
 */
extern void fn_LITSTRING(void);

//-----------------------------------------------------------------------------
/**
 * Leave compilation mode, [.
 *
 * A better name for STATE would be COMPILING.  It is true (non-zero) when
 * we are compiling, and false when interpreting.  But STATE is the standard.
 */
extern void fn_LBRAC(void);

//-----------------------------------------------------------------------------
/**
 * Enter compilation mode, ].
 */
extern void fn_RBRAC(void);

//-----------------------------------------------------------------------------
/**
 * The native implementation of constants.  See DEF_CONST().
 */
extern void fn_CONST(void);

//-----------------------------------------------------------------------------
/**
 * ( -- addr length )
 *
 * The native implementation of string constants.  See DEF_CONST_STRING().
 */
extern void fn_CONST_STRING(void);

//-----------------------------------------------------------------------------
/**
 * The native implementation of variables.  See DEF_VAR().
 */
extern void fn_VAR(void);

//-----------------------------------------------------------------------------
/**
 * EXECUTE ( cfa -- )
 *
 * Execute the word whose eXecution Token (XT) or code field address (CFA) is
 * on TOS.
 */
extern void fn_EXECUTE(void);

//-----------------------------------------------------------------------------
/**
 * ' ( -- cfa )
 *
 * Return the Code Field Address of the next word of input.  It uses the
 * cheat's method borrowed from JonesForth, which in turn borrowed it from
 * buzzard92.
 *
 * This implementation only works in compiled code.
 */
extern void fn_TICK(void);

//-----------------------------------------------------------------------------
/**
 * IP@ ( -- ip )
 *
 * Return the Forth instruction pointer, which is the address of this
 * instruction.  This is used only in the implementation of the STRING() macro
 * for string literals in hand-compiled colon definitions.
 */
extern void fn_IPFETCH(void);

//-----------------------------------------------------------------------------
/**
 * HALT ( -- )
 *
 * Halt the Forth interpreter (exit() the process).
 */
extern void fn_HALT(void);

//-----------------------------------------------------------------------------
/**
 * SYSCALL0 ( call#          -- result )
 * SYSCALL1 ( call# p1       -- result )
 * SYSCALL2 ( call# p1 p2    -- result )
 * SYSCALL3 ( call# p1 p2 p3 -- result )
 *
 * Linux system calls with 0 to 3 parameters.
 */
extern void fn_SYSCALL0(void);
extern void fn_SYSCALL1(void);
extern void fn_SYSCALL2(void);
extern void fn_SYSCALL3(void);

//-----------------------------------------------------------------------------
// Dictionary Manipulation.
//-----------------------------------------------------------------------------
/**
 * FIND ( address length -- lfa )
 *
 * Search the dictionary for the word in the word buffer, and return the link
 * address if found (i.e. the address of the link field), or 0 (FALSE) if not.
 */
extern void fn_FIND(void);

//-----------------------------------------------------------------------------
// Stack Manipulation.
//-----------------------------------------------------------------------------
/**
 * DROP ( x -- )
 */
extern void fn_DROP(void);

//-----------------------------------------------------------------------------
/**
 * DROP ( x y -- y x )
 */
extern void fn_SWAP(void);

//-----------------------------------------------------------------------------
/**
 * DROP ( x -- x x )
 */
extern void fn_DUP(void);

//-----------------------------------------------------------------------------
/**
 * PICK ( nN ... n2 n1 n0 i -- nN ... n2 n1 n0 ni )
 *
 * Copy the ith item of the stack to the top, where i >= 0.
 * "0 PICK" is equivalent to DUP.
 * "1 PICK" is equivalent to OVER.
 */
extern void fn_PICK(void);

//-----------------------------------------------------------------------------
/**
 * STICK ( nN ... n2 n1 n0 x i -- nN ... n(i+1) x n(i-1) ... n2 n1 n0 )
 *
 * Overwrite element i of the stack (i >= 0) with the value x.
 * The index i is interpreted after x and i are popped, so the affected items
 * are as depicted above.  i.e. (i == 0) ==> write the third element prior
 * to popping x and i.
 */
extern void fn_STICK(void);

//-----------------------------------------------------------------------------
/**
 * NTUCK ( nN ... n3 n2 n1 x i -- nN ... n(i+1) x n(i-1) ... n2 n1 )
 *
 * Insert x into the stack at depth i from the top.
 * "z y x 0 NTUCK" leaves x on top of the stack. ( z y x 0 -- z y x )
 * "z y x 1 NTUCK" is like SWAP.                 ( z y x 1 -- z x y )
 * "z y x 2 NTUCK" is like ROT.                  ( z y x 2 -- x z y )
 * (NOTE: To match JonesForth, ROT and -ROT do the opposite of their normal
 * actions.)
 *
 * NOTES:
 * - the number of cells moved is i.
 * - the final index of x relative to TOS is i.
 */
extern void fn_NTUCK(void);

//-----------------------------------------------------------------------------
/**
 * OVER ( n1 n2 -- n1 n2 n1 )
 */
extern void fn_OVER(void);

//-----------------------------------------------------------------------------
/**
 * ROT ( n1 n2 n3 -- n3 n1 n2 )
 *
 * NOTE: In standard Forths, this word would be called -ROT. To maintain
 * compatibility with JonesForth, the definitions of ROT and -ROT are swapped.
 */
extern void fn_ROT(void);

//-----------------------------------------------------------------------------
/**
 * -ROT ( n1 n2 n3 -- n2 n3 n1 )
 *
 * NOTE: In standard Forths, this word would be called ROT. To maintain
 * compatibility with JonesForth, the definitions of ROT and -ROT are swapped.
 */
extern void fn_NROT(void);

//-----------------------------------------------------------------------------
/**
 * 2DROP ( n1 n2 -- )
 */
extern void fn_DDROP(void);

//-----------------------------------------------------------------------------
/**
 * 2DUP ( n1 n2 -- n1 n2 n1 n2 )
 */
extern void fn_DDUP(void);

//-----------------------------------------------------------------------------
/**
 * 2SWAP ( n1 n2 n3 n4 -- n3 n4 n1 n2 )
 */
extern void fn_DSWAP(void);

//-----------------------------------------------------------------------------
/**
 * ?DUP ( n -- n n )  \ n <> 0
 *      ( n -- n   )  \ n =  0
 */
extern void fn_ZDUP(void);

//-----------------------------------------------------------------------------
/**
 * DSP@ ( -- n )
 *
 * Fetch parameter stack pointer.
 */
extern void fn_DSPFETCH(void);

//-----------------------------------------------------------------------------
/**
 * DSP! ( n -- )
 *
 * Store parameter stack pointer.
 */
extern void fn_DSPSTORE(void);

//-----------------------------------------------------------------------------
// Return Stack
//-----------------------------------------------------------------------------
/**
 * >R   ( n --   )
 *     R(   -- n )
 *
 * Move a cell from the parameter stack to the return stack.
 */
extern void fn_TOR(void);

//-----------------------------------------------------------------------------
/**
 * R>  R( n --  )
 *      (   -- n )
 *
 * Move a cell from the return stack to the parameter stack.
 */
extern void fn_FROMR(void);

//-----------------------------------------------------------------------------
/**
 * RSP@ ( -- n )
 *
 * Fetch return stack pointer.
 */
extern void fn_RSPFETCH(void);

//-----------------------------------------------------------------------------
/**
 * RSP! ( n -- )
 *
 * Store return stack pointer.
 */
extern void fn_RSPSTORE(void);

//-----------------------------------------------------------------------------
/**
 * RDROP R( n -- )
 *
 * Drop a cell from the return stack.
 */
extern void fn_RDROP(void);

//-----------------------------------------------------------------------------
// Arithmetic
//-----------------------------------------------------------------------------
/**
 * 1+ ( n -- n+1 )
 */
extern void fn_INCR(void);

//-----------------------------------------------------------------------------
/**
 * 1- ( n -- n-1 )
 */
extern void fn_DECR(void);

//-----------------------------------------------------------------------------
/**
 * CELL+ ( n -- n+CELL )
 * 4+    ( n -- n+4 )
 *
 * Increment a pointer by the size of a cell.
 */
extern void fn_CELLPLUS(void);

//-----------------------------------------------------------------------------
/**
 * CELL- ( n -- n-CELL )
 * 4-    ( n -- n-4 )
 *
 * Decrement a pointer by the size of a cell.
 */
extern void fn_CELLMINUS(void);

//-----------------------------------------------------------------------------
// Unary and binary arithmetic operators.

extern void fn_ADD(void);
extern void fn_SUB(void);
extern void fn_MUL(void);
extern void fn_DIV(void);
extern void fn_MOD(void);
extern void fn_NEGATE(void);

//-----------------------------------------------------------------------------
/**
 * /MOD ( n1 n2 -- rem quot )
 */
extern void fn_DIVMOD(void);

//-----------------------------------------------------------------------------
// Comparison
//-----------------------------------------------------------------------------
// Binary comparisons.

extern void fn_EQ(void); // =
extern void fn_NE(void); // <>
extern void fn_LT(void); // <
extern void fn_GT(void); // >
extern void fn_LE(void); // <=
extern void fn_GE(void); // >=

// Comparisons against 0.

extern void fn_EQ0(void); // 0 =
extern void fn_NE0(void); // 0 <>
extern void fn_LT0(void); // 0 <
extern void fn_GT0(void); // 0 >
extern void fn_LE0(void); // 0 <=
extern void fn_GE0(void); // 0 >=

//-----------------------------------------------------------------------------
// Bitwise
//-----------------------------------------------------------------------------
/**
 * AND OR XOR INVERT
 */
extern void fn_AND(void);
extern void fn_OR(void);
extern void fn_XOR(void);
extern void fn_INVERT(void);

//-----------------------------------------------------------------------------
// Memory
//-----------------------------------------------------------------------------
/**
 * ! ( n addr -- )
 */
extern void fn_STORE(void);

//-----------------------------------------------------------------------------
/**
 * @ ( addr -- n )
 */
extern void fn_FETCH(void);

//-----------------------------------------------------------------------------
/**
 * +! ( n addr -- )
 */
extern void fn_PLUSSTORE(void);

//-----------------------------------------------------------------------------
/**
 * -! ( n addr -- )
 */
extern void fn_MINUSSTORE(void);

//-----------------------------------------------------------------------------
/**
 * C! ( n addr -- )
 */
extern void fn_CSTORE(void);

//-----------------------------------------------------------------------------
/**
 * C@ ( addr -- n )
 */
extern void fn_CFETCH(void);

//-----------------------------------------------------------------------------
/**
 * C@C! ( source dest -- source+1 dest+1  )
 *
 * Copy a byte from source to dest, and increment both addresses.
 */
extern void fn_CCOPY(void);

//-----------------------------------------------------------------------------
/**
 * CMOVE ( source dest count -- )
 */
extern void fn_CMOVE(void);

//-----------------------------------------------------------------------------
/**
 * FILL ( addr n b -- )
 *
 * Fills n bytes of memory, beginning at the specified address, with value b.
 */
extern void fn_FILL(void);

//-----------------------------------------------------------------------------
// Output
//-----------------------------------------------------------------------------
/**
 * EMIT ( c -- )
 *
 * Write a single character to stdout, whose ASCII code is TOS.
 */
extern void fn_EMIT(void);

//-----------------------------------------------------------------------------
/**
 * TELL ( addr length -- )
 *
 * Writes the string of characters at addr, with the specified length, to
 * stdout.
 */
extern void fn_TELL(void);

//-----------------------------------------------------------------------------
/**
 * DOT ( n -- )
 */
extern void fn_DOT(void);

//-----------------------------------------------------------------------------
// Time.
//-----------------------------------------------------------------------------
/**
 * MSLEEP ( n -- )
 *
 * Sleep n milliseconds.
 */
extern void fn_MSLEEP(void);

//-----------------------------------------------------------------------------

#endif // TOMOKO_NATIVE_H

