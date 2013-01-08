//-----------------------------------------------------------------------------
// Native implementations of miscellaneous Forth words.
//-----------------------------------------------------------------------------

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "native.h"
#include "machine.h"
#include "dictionary.h"

//-----------------------------------------------------------------------------
// External references to a few Forth global variables:

extern Cell STATE_value;
extern Cell LATEST_value;
extern Cell CASE_SENSITIVE_value;

//-----------------------------------------------------------------------------
// Interpreter basics.
//-----------------------------------------------------------------------------

void fn_DOCOL(void)
{
  STACK_PUSH(rsp, ip);

  // Skip over the codeword to the PFA.
  ip = (CodeWord**) (w + 1);
}

//-----------------------------------------------------------------------------

void fn_DODOES(void)
{
  STACK_PUSH(rsp, ip);
  STACK_PUSH(sp, w + 2);

  // Set IP to the address pointed to by IFA.
  ip = (CodeWord**) *(w + 1);
}

//-----------------------------------------------------------------------------

void fn_EXIT(void)
{
  ip = (CodeWord**) STACK_POP(rsp);
}

//-----------------------------------------------------------------------------

void fn_BRANCH(void)
{
  // The offset is expressed in bytes, not cells...
  ip = (CodeWord**) ((char*)ip + *(Cell*)ip);
}

//-----------------------------------------------------------------------------

void fn_ZBRANCH(void)
{
  if (STACK_POP(sp) == 0)
  {
    // The offset is expressed in bytes, not cells...
    ip = (CodeWord**) ((char*)ip + *(Cell*)ip);
  }
  else
  {
    // Skip ip past the offset.
    ++ip;
  }
}

//-----------------------------------------------------------------------------

void fn_LIT(void)
{
  STACK_PUSH(sp, *(Cell*)ip);

  // Skip over the cell containing the literal.
  ++ip;
}

//-----------------------------------------------------------------------------

void fn_LITSTRING(void)
{
  // By the time we get into this function, the instruction pointer has 
  // advanced past the XT of LITSTRING and points at the length cell.
  const Cell *len = (const Cell*) ip;
  
  STACK_PUSH(sp, len + 1); 
  STACK_PUSH(sp, *len);
  
  // Compute number of bytes to skip forward.
  Cell skipped = ((Cell)*len + 2 * sizeof (Cell) - 1) & ~(sizeof (Cell) - 1);
  ip = (CodeWord**) ((char*)ip + skipped);
}

//-----------------------------------------------------------------------------

void fn_LBRAC(void)
{
  STATE_value = 0;
}

//-----------------------------------------------------------------------------

void fn_RBRAC(void)
{
  STATE_value = 1;
}

//-----------------------------------------------------------------------------

void fn_CONST(void)
{
  // The address of this codeword is in w.
  // Push the first Cell from the PFA.
  STACK_PUSH(sp, w[1]);
}

//-----------------------------------------------------------------------------

void fn_CONST_STRING(void)
{
  // The address of this codeword is in w.
  // The first cell after the codeword is the string length, and the address
  // of the cell after that is the start of the string.
  STACK_PUSH(sp, w + 2); // Address.
  STACK_PUSH(sp, w[1]);  // Length.
}

//-----------------------------------------------------------------------------

void fn_VAR(void)
{
  // The address of this codeword is in w.
  // The Cell in the PFA is the address of the variable.
  STACK_PUSH(sp, w[1]);
}

//-----------------------------------------------------------------------------

void fn_EXECUTE(void)
{
  // Pop the eXecution Token / Code Field Address.
  w = (CodeWord*) STACK_POP(sp);

  // Call the codeword.
  (*w)();
}

//-----------------------------------------------------------------------------

void fn_TICK(void)
{
  // Push the next (compiled) codeword and then skip over it.
  STACK_PUSH(sp, *ip++);
}

//-----------------------------------------------------------------------------

void fn_IPFETCH(void)
{
  // Note that when this instruction is executing, ip will already have advanced
  // to point to the next instruction.
  STACK_PUSH(sp, ip - 1);
}

//-----------------------------------------------------------------------------

void fn_HALT(void)
{
  exit(EXIT_SUCCESS);
}

//-----------------------------------------------------------------------------

void fn_SYSCALL0(void)
{
  // TODO: get up to speeed with Linux inline assembler. :)
	//pop %eax		// System call number (see <asm/unistd.h>)
	//int $0x80
	//push %eax		// Result (negative for -errno)
}

//-----------------------------------------------------------------------------

void fn_SYSCALL1(void)
{
  // TODO: get up to speeed with Linux inline assembler. :)
	//pop %eax		// System call number (see <asm/unistd.h>)
	//pop %ebx		// First parameter.
	//int $0x80
	//push %eax		// Result (negative for -errno)
}

//-----------------------------------------------------------------------------

void fn_SYSCALL2(void)
{
  // TODO: get up to speeed with Linux inline assembler. :)
	//pop %eax		// System call number (see <asm/unistd.h>)
	//pop %ebx		// First parameter.
	//pop %ecx		// Second parameter
	//int $0x80
	//push %eax		// Result (negative for -errno)
}

//-----------------------------------------------------------------------------

void fn_SYSCALL3(void)
{
  // TODO: get up to speeed with Linux inline assembler. :)
  //pop %eax		// System call number (see <asm/unistd.h>)
	//pop %ebx		// First parameter.
	//pop %ecx		// Second parameter
	//pop %edx		// Third parameter
	//int $0x80
	//push %eax		// Result (negative for -errno)
}

//-----------------------------------------------------------------------------
// Dictionary Manipulation.
//-----------------------------------------------------------------------------

void fn_FIND(void)
{
  Cell targetLength  = STACK_POP(sp);
  const char *target = (const char *) STACK_POP(sp);
  const Cell *link   = (const Cell*) LATEST_value; // most recently defined word
  while (link != NULL)
  {
    // Set name to point to the length byte.
    // Is this entry hidden?
    const char *name = (const char*) (link + 1);
    if ((*name & HIDDEN_BIT) == 0)
    {
      // Set length to length of name, and advance to first char.
      char length = *name & LENGTH_BITS;
      ++name;

      // If the length of this word matches the search target, compare
      // character by character.
      if (length == targetLength)
      {
        if (CASE_SENSITIVE_value)
        {
          int i;
          for (i = 0; i < targetLength; ++i)
          {
            if (name[i] != target[i])
            {
              goto Next;
            }
          }
        }
        else
        {
          // FIND uses case insensitive string comparisons.
          int i;
          for (i = 0; i < targetLength; ++i)
          {
            if (toupper(name[i]) != toupper(target[i]))
            {
              goto Next;
            }
          }
        }

        // If we get to here, then it's an exact match.
        break;
      }
    }

  Next:
    // Advance to next link.
    link = (const Cell*) *link;
  } // while

  // Whatever we found...
  STACK_PUSH(sp, link);
} // fn_FIND

//-----------------------------------------------------------------------------
// Stack Manipulation.
//-----------------------------------------------------------------------------

void fn_DROP(void)
{
  (void) STACK_POP(sp);
}

//-----------------------------------------------------------------------------

void fn_SWAP(void)
{
  Cell c0 = STACK_POP(sp);
  Cell c1 = STACK_POP(sp);
  STACK_PUSH(sp, c0);
  STACK_PUSH(sp, c1);
}

//-----------------------------------------------------------------------------

void fn_DUP(void)
{
  // Avoid evaluation order issues.
  Cell top = *sp;
  STACK_PUSH(sp, top);
}

//-----------------------------------------------------------------------------

void fn_PICK(void)
{
  Cell index = STACK_POP(sp);
  Cell item = STACK_PICK(sp, index);
  STACK_PUSH(sp, item);
}

//-----------------------------------------------------------------------------

void fn_STICK(void)
{
  Cell index = STACK_POP(sp);
  Cell item = STACK_POP(sp);
  // TODO: if index is out of bounds, throw.
  *STACK_ADDR(sp, index) = item;
}

//-----------------------------------------------------------------------------

void fn_NTUCK(void)
{
  Cell index = STACK_POP(sp);
  // (index <= 0) ==> leave x as TOS.  TODO: throw for index < 0
  if (index > 0)
  {
    // x is current TOS.
    Cell x = *sp;

    // Shift everything up by one cell, overwriting.
    Cell i;
    for (i = 0; i < index; ++i)
    {
      // By using the STACK_ADDR() macro, this definition works irrespective
      // of the direction of stack growth.
      *STACK_ADDR(sp, i) = *STACK_ADDR(sp, i + 1);
    }

    // Poke x underneath.
    *STACK_ADDR(sp,index) = x;
  }
} // fn_NTUCK

//-----------------------------------------------------------------------------

void fn_OVER(void)
{
  Cell item = STACK_PICK(sp, 1);
  STACK_PUSH(sp, item);
}

//-----------------------------------------------------------------------------

void fn_ROT(void)
{
  Cell n3 = STACK_POP(sp);
  Cell n2 = STACK_POP(sp);
  Cell n1 = STACK_POP(sp);
  STACK_PUSH(sp, n3);
  STACK_PUSH(sp, n1);
  STACK_PUSH(sp, n2);
}

//-----------------------------------------------------------------------------

void fn_NROT(void)
{
  Cell n3 = STACK_POP(sp);
  Cell n2 = STACK_POP(sp);
  Cell n1 = STACK_POP(sp);
  STACK_PUSH(sp, n2);
  STACK_PUSH(sp, n3);
  STACK_PUSH(sp, n1);
}

//-----------------------------------------------------------------------------

void fn_DDROP(void)
{
  (void) STACK_POP(sp);
  (void) STACK_POP(sp);
}

//-----------------------------------------------------------------------------

void fn_DDUP(void)
{
  Cell n2 = STACK_POP(sp);
  Cell n1 = STACK_POP(sp);
  STACK_PUSH(sp, n1);
  STACK_PUSH(sp, n2);
  STACK_PUSH(sp, n1);
  STACK_PUSH(sp, n2);
}

//-----------------------------------------------------------------------------

void fn_DSWAP(void)
{
  Cell n4 = STACK_POP(sp);
  Cell n3 = STACK_POP(sp);
  Cell n2 = STACK_POP(sp);
  Cell n1 = STACK_POP(sp);
  STACK_PUSH(sp, n3);
  STACK_PUSH(sp, n4);
  STACK_PUSH(sp, n1);
  STACK_PUSH(sp, n2);
}

//-----------------------------------------------------------------------------

void fn_ZDUP(void)
{
  Cell top = *sp;
  if (top != 0)
  {
    STACK_PUSH(sp, top);
  }
}

//-----------------------------------------------------------------------------

void fn_DSPFETCH(void)
{
  Cell value = (Cell) sp;
  STACK_PUSH(sp, value);
}

//-----------------------------------------------------------------------------

void fn_DSPSTORE(void)
{
  Cell value = STACK_POP(sp);
  sp = (Cell *) value;
}

//-----------------------------------------------------------------------------
// Return Stack
//-----------------------------------------------------------------------------

void fn_TOR(void)
{
  STACK_PUSH(rsp, STACK_POP(sp));
}

//-----------------------------------------------------------------------------

void fn_FROMR(void)
{
  STACK_PUSH(sp, STACK_POP(rsp));
}

//-----------------------------------------------------------------------------

void fn_RSPFETCH(void)
{
  Cell value = (Cell) rsp;
  STACK_PUSH(sp, value);
}

//-----------------------------------------------------------------------------

void fn_RSPSTORE(void)
{
  Cell value = STACK_POP(sp);
  rsp = (Cell *) value;
}

//-----------------------------------------------------------------------------

void fn_RDROP(void)
{
  (void) STACK_POP(rsp);
}

//-----------------------------------------------------------------------------
// Arithmetic
//-----------------------------------------------------------------------------

void fn_INCR(void)
{
  Cell value = STACK_POP(sp);
  STACK_PUSH(sp, value + 1);
}

//-----------------------------------------------------------------------------

void fn_DECR(void)
{
  Cell value = STACK_POP(sp);
  STACK_PUSH(sp, value - 1);
}

//-----------------------------------------------------------------------------

void fn_CELLPLUS(void)
{
  Cell value = STACK_POP(sp);
  STACK_PUSH(sp, value + sizeof (Cell));
}

//-----------------------------------------------------------------------------

void fn_CELLMINUS(void)
{
  Cell value = STACK_POP(sp);
  STACK_PUSH(sp, value - sizeof (Cell));
}

//-----------------------------------------------------------------------------
/**
 * Return the result of a unary integer operator.
 */
#define DEF_UNARY_OP_FN(name,op)                                              \
  void fn_##name(void)                                                        \
  {                                                                           \
    Cell n = STACK_POP(sp);                                                   \
    STACK_PUSH(sp, op(n));                                                    \
  }

/**
 * Return the result of a signed integer binary operator.
 */
#define DEF_BINARY_OP_FN(name,op)                                             \
  void fn_##name(void)                                                        \
  {                                                                           \
    Cell n2 = STACK_POP(sp);                                                  \
    Cell n1 = STACK_POP(sp);                                                  \
    STACK_PUSH(sp, n1 op n2);                                                 \
  }

DEF_BINARY_OP_FN(ADD, +);
DEF_BINARY_OP_FN(SUB, -);
DEF_BINARY_OP_FN(MUL, *);
DEF_BINARY_OP_FN(DIV, /); // JonesForth defines this in terms of /MOD.
DEF_BINARY_OP_FN(MOD, %); // JonesForth defines this in terms of /MOD.
DEF_UNARY_OP_FN(NEGATE, -);

//-----------------------------------------------------------------------------

void fn_DIVMOD(void)
{
  Cell n2 = STACK_POP(sp);
  Cell n1 = STACK_POP(sp);
  STACK_PUSH(sp, n1 % n2);
  STACK_PUSH(sp, n1 / n2);
}

//-----------------------------------------------------------------------------
// Comparison
//-----------------------------------------------------------------------------

#define DEF_COMPARISON_FN(name,op)                                            \
  void fn_##name(void)                                                        \
  {                                                                           \
    Cell n2 = STACK_POP(sp);                                                  \
    Cell n1 = STACK_POP(sp);                                                  \
    STACK_PUSH(sp, BOOLEAN(n1 op n2));                                        \
  }

DEF_COMPARISON_FN(EQ, ==);
DEF_COMPARISON_FN(NE, !=);
DEF_COMPARISON_FN(LT, <);
DEF_COMPARISON_FN(GT, >);
DEF_COMPARISON_FN(LE, <=);
DEF_COMPARISON_FN(GE, >=);

#define DEF_COMPARISON0_FN(name,op)                                           \
  void fn_##name(void)                                                        \
  {                                                                           \
    Cell n = STACK_POP(sp);                                                   \
    STACK_PUSH(sp, BOOLEAN(n op 0));                                          \
  }

DEF_COMPARISON0_FN(EQ0, ==);
DEF_COMPARISON0_FN(NE0, !=);
DEF_COMPARISON0_FN(LT0, <);
DEF_COMPARISON0_FN(GT0, >);
DEF_COMPARISON0_FN(LE0, <=);
DEF_COMPARISON0_FN(GE0, >=);

//-----------------------------------------------------------------------------
// Bitwise
//-----------------------------------------------------------------------------

DEF_BINARY_OP_FN(AND, &);
DEF_BINARY_OP_FN(OR, |);
DEF_BINARY_OP_FN(XOR, ^);
DEF_UNARY_OP_FN(INVERT, ~);

//-----------------------------------------------------------------------------
// Memory
//-----------------------------------------------------------------------------

void fn_STORE(void)
{
  Cell *addr = (Cell*) STACK_POP(sp);
  *addr = STACK_POP(sp);
}

//-----------------------------------------------------------------------------

void fn_FETCH(void)
{
  const Cell *addr = (const Cell*) STACK_POP(sp);
  STACK_PUSH(sp, *addr);
}

//-----------------------------------------------------------------------------

void fn_PLUSSTORE(void)
{
  Cell *addr = (Cell*) STACK_POP(sp);
  *addr += STACK_POP(sp);
}

//-----------------------------------------------------------------------------

void fn_MINUSSTORE(void)
{
  Cell *addr = (Cell*) STACK_POP(sp);
  *addr += STACK_POP(sp);
}

//-----------------------------------------------------------------------------

void fn_CSTORE(void)
{
  uint8_t *addr = (uint8_t*) STACK_POP(sp);
  *addr = STACK_POP(sp);
}

//-----------------------------------------------------------------------------

void fn_CFETCH(void)
{
  const uint8_t *addr = (const uint8_t*) STACK_POP(sp);
  STACK_PUSH(sp, *addr);
}

//-----------------------------------------------------------------------------

void fn_CCOPY(void)
{
        uint8_t *dest   = (      uint8_t*) STACK_POP(sp);
  const uint8_t *source = (const uint8_t*) STACK_POP(sp);
  *dest++ = *source;
  STACK_PUSH(sp, source);
  STACK_PUSH(sp, dest);
}

//-----------------------------------------------------------------------------

void fn_CMOVE(void)
{
  Cell count = STACK_POP(sp);
        void *dest   = (void*)       STACK_POP(sp);
  const void *source = (const void*) STACK_POP(sp);
  if (count > 0)
  {
    memmove(dest, source, count);
  }
}

//-----------------------------------------------------------------------------

void fn_FILL(void)
{
  uint8_t b = STACK_POP(sp);
  Cell n = STACK_POP(sp);
  uint8_t *addr = (uint8_t*) STACK_POP(sp);
  while (n-- > 0)
  {
    *addr++ = b;
  }
}

//-----------------------------------------------------------------------------
// Output
//-----------------------------------------------------------------------------
/**
 * Output a single character to the terminal.
 *
 * This is intended to be a single maintenance point for porting this
 * functionality.
 *
 * @param c the character to display.
 */
static void charOut(char c)
{
  putchar(c);
  fflush(stdout);
}

//-----------------------------------------------------------------------------

void fn_EMIT(void)
{
  charOut(STACK_POP(sp));
}

//-----------------------------------------------------------------------------

void fn_TELL(void)
{
  Cell length = STACK_POP(sp);
  const char *addr = (const char*) STACK_POP(sp);
  while (length-- > 0)
  {
    charOut(*addr++);
  }
}

//-----------------------------------------------------------------------------

void fn_DOT(void)
{
  Cell n = STACK_POP(sp);
  printf("%d", n);
  fflush(stdout);
}

//-----------------------------------------------------------------------------
// Time.
//-----------------------------------------------------------------------------

void fn_MSLEEP(void)
{
  usleep(STACK_POP(sp) * 1000);
}

//-----------------------------------------------------------------------------

