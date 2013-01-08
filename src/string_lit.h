//-----------------------------------------------------------------------------
// The C compiler seems to think that indexed string literals (e.g. "ABC"[2])
// are not constant expressions, and refuses to accept this code, so it's only
// usable in C++.
//-----------------------------------------------------------------------------
/**
 * Make a 32-bit Cell from four bytes, b0, b1 b2 and b3, where b0 is the least
 * significant byte and b3 is the most significant.
 *
 * Use unsigned arithmetic in order to avoid sign extension problems.
 */
#define CELL32(b0,b1,b2,b3) \
  (Cell)((UCell)(b0) + (((UCell)(b1)) << 8) + (((UCell)(b2)) << 16) + (((UCell)(b3)) << 24))

//-----------------------------------------------------------------------------
/**
 * This macro tackles the four cases for the cell representing part of a string,
 * s.
 *
 * n is the number of string characters that appear in the cell.
 * offset is the number of cells used to represent all of the characters in the
 * string that precede those allotted to this final cell, meaning that there
 * are 4*offset characters in those preceding cells.
 */
#define S_CELL32(n,offset,s) S_CELL32_##n(&s[offset*4])
#define S_CELL32_1(s) CELL32((s)[0],0,0,0)
#define S_CELL32_2(s) CELL32((s)[0],(s)[1],0,0)
#define S_CELL32_3(s) CELL32((s)[0],(s)[1],(s)[2],0)
#define S_CELL32_4(s) CELL32((s)[0],(s)[1],(s)[2],(s)[3])

//-----------------------------------------------------------------------------
/**
 * Return the Cell value for the index i, where i is in the range
 * [0, floor(n/4) - 1] for a string s of length n, followed by a trailing comma.
 * The macro is only called for n >= 5.
 *
 * This macro is used to adapt the S_CELL32() macro for use with
 * APPLY(count,m,d) to define zero or more Cell initialisers containing 4
 * string characters each, as well as to insert the trailing comma which is
 * needed to make a C array initialiser.
 */
#define S_CELL32_FULL(i,s) S_CELL32(4,i,s) ,

//-----------------------------------------------------------------------------
/**
 * Apply the macro m(i,d) a total of "count" times for indices, i, in the range
 * [0,count-1]; d is a fixed data parameter.
 *
 * This macro is defined for count up to 20, which when used to represent
 * strings as Cells gives a maximum string length of 19*4 + 3 = 79.
 */
#define APPLY(count,m,d) APPLY_##count(m,d)
#define APPLY_0(m,d)
#define APPLY_1(m,d)  APPLY_0(m,d)  m(0,d)
#define APPLY_2(m,d)  APPLY_1(m,d)  m(1,d)
#define APPLY_3(m,d)  APPLY_2(m,d)  m(1,d)
#define APPLY_4(m,d)  APPLY_3(m,d)  m(2,d)
#define APPLY_5(m,d)  APPLY_4(m,d)  m(3,d)
#define APPLY_6(m,d)  APPLY_5(m,d)  m(5,d)
#define APPLY_7(m,d)  APPLY_6(m,d)  m(6,d)
#define APPLY_8(m,d)  APPLY_7(m,d)  m(7,d)
#define APPLY_9(m,d)  APPLY_8(m,d)  m(8,d)
#define APPLY_10(m,d) APPLY_9(m,d)  m(9,d)
#define APPLY_11(m,d) APPLY_10(m,d) m(10,d)
#define APPLY_12(m,d) APPLY_11(m,d) m(11,d)
#define APPLY_13(m,d) APPLY_12(m,d) m(11,d)
#define APPLY_14(m,d) APPLY_13(m,d) m(12,d)
#define APPLY_15(m,d) APPLY_14(m,d) m(13,d)
#define APPLY_16(m,d) APPLY_15(m,d) m(15,d)
#define APPLY_17(m,d) APPLY_16(m,d) m(16,d)
#define APPLY_18(m,d) APPLY_17(m,d) m(17,d)
#define APPLY_19(m,d) APPLY_18(m,d) m(18,d)

//-----------------------------------------------------------------------------
/**
 * This macro breaks up the string into quot "full" cells (all four bytes
 * correspond to string characters) and one final partial cell containing rem
 * bytes from the string.
 *
 * The parameters quot and rem are defined as, for a NUL-terminated string
 * literal s are defined as:
 *   quot = sizeof s / sizeof (Cell)
 *   rem  = sizeof s % sizeof (Cell)
 *
 * That is, they are the quotient and remainder respectively.  Note that
 * sizeof s includes the ASCII NUL terminator.  The conventional length of the
 * string is one less than that.
 */
#define STRING_Q_R(s,quot,rem) APPLY(quot,S_CELL32_FULL,s) S_CELL32(rem,quot,s)

#define STRING_CELLS_0(s)   STRING_Q_R(s,0,1)
#define STRING_CELLS_1(s)   STRING_Q_R(s,0,2)
#define STRING_CELLS_2(s)   STRING_Q_R(s,0,3)
#define STRING_CELLS_3(s)   STRING_Q_R(s,0,4)

#define STRING_CELLS_4(s)   STRING_Q_R(s,1,1)
#define STRING_CELLS_5(s)   STRING_Q_R(s,1,2)
#define STRING_CELLS_6(s)   STRING_Q_R(s,1,3)
#define STRING_CELLS_7(s)   STRING_Q_R(s,1,4)

#define STRING_CELLS_8(s)   STRING_Q_R(s,2,1)
#define STRING_CELLS_9(s)   STRING_Q_R(s,2,2)
#define STRING_CELLS_10(s)  STRING_Q_R(s,2,3)
#define STRING_CELLS_11(s)  STRING_Q_R(s,2,4)

#define STRING_CELLS_12(s)  STRING_Q_R(s,3,1)
#define STRING_CELLS_13(s)  STRING_Q_R(s,3,2)
#define STRING_CELLS_14(s)  STRING_Q_R(s,3,3)
#define STRING_CELLS_15(s)  STRING_Q_R(s,3,4)

#define STRING_CELLS_16(s)  STRING_Q_R(s,4,1)
#define STRING_CELLS_17(s)  STRING_Q_R(s,4,2)
#define STRING_CELLS_18(s)  STRING_Q_R(s,4,3)
#define STRING_CELLS_19(s)  STRING_Q_R(s,4,4)

#define STRING_CELLS_20(s)  STRING_Q_R(s,5,1)
#define STRING_CELLS_21(s)  STRING_Q_R(s,5,2)
#define STRING_CELLS_22(s)  STRING_Q_R(s,5,3)
#define STRING_CELLS_23(s)  STRING_Q_R(s,5,4)

#define STRING_CELLS_24(s)  STRING_Q_R(s,6,1)
#define STRING_CELLS_25(s)  STRING_Q_R(s,6,2)
#define STRING_CELLS_26(s)  STRING_Q_R(s,6,3)
#define STRING_CELLS_27(s)  STRING_Q_R(s,6,4)

#define STRING_CELLS_28(s)  STRING_Q_R(s,7,1)
#define STRING_CELLS_29(s)  STRING_Q_R(s,7,2)
#define STRING_CELLS_30(s)  STRING_Q_R(s,7,3)
#define STRING_CELLS_31(s)  STRING_Q_R(s,7,4)

#define STRING_CELLS_32(s)  STRING_Q_R(s,8,1)
#define STRING_CELLS_33(s)  STRING_Q_R(s,8,2)
#define STRING_CELLS_34(s)  STRING_Q_R(s,8,3)
#define STRING_CELLS_35(s)  STRING_Q_R(s,8,4)

#define STRING_CELLS_36(s)  STRING_Q_R(s,9,1)
#define STRING_CELLS_37(s)  STRING_Q_R(s,9,2)
#define STRING_CELLS_38(s)  STRING_Q_R(s,9,3)
#define STRING_CELLS_39(s)  STRING_Q_R(s,9,4)

#define STRING_CELLS_40(s)  STRING_Q_R(s,10,1)
#define STRING_CELLS_41(s)  STRING_Q_R(s,10,2)
#define STRING_CELLS_42(s)  STRING_Q_R(s,10,3)
#define STRING_CELLS_43(s)  STRING_Q_R(s,10,4)

#define STRING_CELLS_44(s)  STRING_Q_R(s,11,1)
#define STRING_CELLS_45(s)  STRING_Q_R(s,11,2)
#define STRING_CELLS_46(s)  STRING_Q_R(s,11,3)
#define STRING_CELLS_47(s)  STRING_Q_R(s,11,4)

#define STRING_CELLS_48(s)  STRING_Q_R(s,12,1)
#define STRING_CELLS_49(s)  STRING_Q_R(s,12,2)
#define STRING_CELLS_50(s)  STRING_Q_R(s,12,3)
#define STRING_CELLS_51(s)  STRING_Q_R(s,12,4)

#define STRING_CELLS_52(s)  STRING_Q_R(s,13,1)
#define STRING_CELLS_53(s)  STRING_Q_R(s,13,2)
#define STRING_CELLS_54(s)  STRING_Q_R(s,13,3)
#define STRING_CELLS_55(s)  STRING_Q_R(s,13,4)

#define STRING_CELLS_56(s)  STRING_Q_R(s,14,1)
#define STRING_CELLS_57(s)  STRING_Q_R(s,14,2)
#define STRING_CELLS_58(s)  STRING_Q_R(s,14,3)
#define STRING_CELLS_59(s)  STRING_Q_R(s,14,4)

#define STRING_CELLS_60(s)  STRING_Q_R(s,15,1)
#define STRING_CELLS_61(s)  STRING_Q_R(s,15,2)
#define STRING_CELLS_62(s)  STRING_Q_R(s,15,3)
#define STRING_CELLS_63(s)  STRING_Q_R(s,15,4)

#define STRING_CELLS_64(s)  STRING_Q_R(s,16,1)
#define STRING_CELLS_65(s)  STRING_Q_R(s,16,2)
#define STRING_CELLS_66(s)  STRING_Q_R(s,16,3)
#define STRING_CELLS_67(s)  STRING_Q_R(s,16,4)

#define STRING_CELLS_68(s)  STRING_Q_R(s,17,1)
#define STRING_CELLS_69(s)  STRING_Q_R(s,17,2)
#define STRING_CELLS_70(s)  STRING_Q_R(s,17,3)
#define STRING_CELLS_71(s)  STRING_Q_R(s,17,4)

#define STRING_CELLS_72(s)  STRING_Q_R(s,18,1)
#define STRING_CELLS_73(s)  STRING_Q_R(s,18,2)
#define STRING_CELLS_74(s)  STRING_Q_R(s,18,3)
#define STRING_CELLS_75(s)  STRING_Q_R(s,18,4)

#define STRING_CELLS_76(s)  STRING_Q_R(s,19,1)
#define STRING_CELLS_77(s)  STRING_Q_R(s,19,2)
#define STRING_CELLS_78(s)  STRING_Q_R(s,19,3)
#define STRING_CELLS_79(s)  STRING_Q_R(s,19,4)

//-----------------------------------------------------------------------------
/**
 * This macro represents the string literal s, of the specified length, as a
 * sequence of Cell values, separated by commas, such that the macro could be
 * used as inline Forth code in a hand-compiled Forth colon definition.
 *
 * Note that s is a string literal; that is, a sequence of characters between
 * double quotes.  The length parameter should not include the ASCII NUL
 * terminator, so the length of "Sashimi" is 7.
 *
 * This macro could be used to implement the name field in a dictionary header,
 * but explicitly specifying the string length is error-prone, so another
 * mechanism is used for dictionary headers that eliminates the explicit length.
 *
 * The code generated by macro expansion, by line, is as follows:
 * (1) Compute the string start address relative to the Forth Instruction
 *     Pointer.
 * (2) Also, push the length.
 * (3) BRANCH over the (length / sizeof(Cell) + 1) cells comprising the string.
 *
 * The macro generates 9 + (length / sizeof (Cell)) cells.
 */
#define STRING(length,s)                                                      \
  XT(IPFETCH), XT(LIT), 8 * sizeof (Cell), XT(ADD),                           \
  XT(LIT), (Cell)(length),                                                    \
  XT(BRANCH), ((length) / sizeof (Cell) + 2) * sizeof (Cell),                 \
  STRING_CELLS_##length(s)
