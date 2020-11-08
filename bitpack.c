/*************************************************************************
*                             wordpack.c
* 
* 
*      Authors: Jae Hyun Cheigh (jcheig01), Suyu Lui (sliu21)
*
*      Fall 2020 - COMP40
*      HW 4
* 
* 
*      Summary: Implementation file that handles obtaining/assigning
*               info from/to packed bits
*               
**************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "bitpack.h"
#include "assert.h"

Except_T Bitpack_Overflow = { "Overflow packing bits" };

/* left shift of unsigned/signed values */
uint64_t left_shift(uint64_t word, unsigned width);
/* right shift of unsigned values */ 
uint64_t right_shiftu(uint64_t word, unsigned width);
/* right shift of signed values */
int64_t right_shifts(int64_t word, unsigned width);

/****************************************************************
 * Bitpack_fitsu
 * Description: Width test if unsigned value fits in the width of bits
 * Inputs: 1) Unsigned integer of test value
 *         2) Width of bits
 * Output: Bool telling if the value fits or not
 * Implementation: Check if input width is not greater than 64.
 *                 If the value is greater than the left shift
 *                 of bits that is done by the width, then return
 *                 false.
 *****************************************************************/
bool Bitpack_fitsu(uint64_t n, unsigned width)
{
    assert(width <= 64);

    if (width == 0)
    {
        return false;
    }

    if (n > left_shift(1, width) - 1)
    {
        return false;
    }

    return true;
}

/****************************************************************
 * Bitpack_fitss
 * Description: Width test if signed value fits in the width of bits
 * Inputs: 1) Signed integer of test value
 *         2) Width of bits
 * Output: Bool telling if the value fits or not
 * Implementation: Check if input width is not greater than 64.
 *                 Set lower bound by shifting ~0 mask left by
 *                 width-1 and use ~ operation to set upper bound
 *                 and check if the value is in between the bounds.
 *****************************************************************/
bool Bitpack_fitss(int64_t n, unsigned width)
{
    assert(width <= 64);

    if (width == 0)
    {
        return false;
    }
    /* define lower bound and upper bound */
    int64_t low = (int64_t) (left_shift(~0, width - 1));
    int64_t high = ~low;

    if (n >= low && n <= high)
    {
        return true;
    }

    return false;
}

/****************************************************************
 * Bitpack_getu
 * Description: Extract unsigned values from a word
 * Inputs: 1) Unsigned type word
 *         2) Unsigned type width of bits
 *         3) Unsigned type least significant bit
 * Output: Unsigned values
 * Implementation: Check if width is not greater than 64 and
 *                 width and least significant bit combined is
 *                 not greater than 64. Perform left shift and
 *                 right shift on mask to place mask at unsigned
 *                 value will be extracted.
 *****************************************************************/
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
    assert(width <= 64 && width + lsb <= 64);

    uint64_t mask = ~0;
    mask = left_shift(mask, 64 - width);
    mask = right_shiftu(mask, 64 - width - lsb);
    uint64_t value = right_shiftu(word & mask, lsb);

    return value;
}

/****************************************************************
 * Bitpack_gets
 * Description: Extract signed values from a word
 * Inputs: 1) Unsigned type word
 *         2) Unsigned type width of bits
 *         3) Unsigned type least significant bit
 * Output: Signed values
 * Implementation: Check if width is not greater than 64 and
 *                 width and least significant bit combined is
 *                 not greater than 64. Perform Bitpack_getu to
 *                 extract values and perform left and right shift
 *                 to make signed value.
 *****************************************************************/
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
{
    assert(width <= 64 && width + lsb <= 64);

    uint64_t value = Bitpack_getu(word, width, lsb);
    /* additional left shift and right shift to make signed bits*/
    value = left_shift(value, 64 - width);
    int64_t value2 = right_shifts(value, 64 - width);

    return value2;
}

/****************************************************************
 * Bitpack_newu
 * Description: Update value in the word
 * Inputs: 1) Unsigned type word
 *         2) Unsigned type width of bits
 *         3) Unsigned type least significant bit
 *         4) Unsigned value that will be replacing in word
 * Output: Unsigned type of new word
 * Implementation: Check if width is not greater than 64 and
 *                 width and least significant bit combined is
 *                 not greater than 64. Check if the value
 *                 fit in the width. Then, by shift the mask
 *                 to where the value is and make them 0 and
 *                 replace it with value.
 *****************************************************************/
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb, 
                      uint64_t value)
{
    assert(width <= 64 && width + lsb <= 64);

    if (Bitpack_fitsu(value, width) == false)
    {
        RAISE(Bitpack_Overflow);
    }

    uint64_t mask = ~0;
    mask = left_shift(mask, 64 - width);
    mask = right_shiftu(mask, 64 - width - lsb);
    /* get value at mask */
    value = left_shift(value, lsb);
    value = value & mask;
    /* update input value to existing word */
    word = word & ~mask;
    word = word | value;

    return word;
}

/****************************************************************
 * Bitpack_news
 * Description: Update value in the word
 * Inputs: 1) Unsigned type word
 *         2) Unsigned type width of bits
 *         3) Unsigned type least significant bit
 *         4) Signed value that will be replacing in word
 * Output: Unsigned type of new word
 * Implementation: Check if width is not greater than 64 and
 *                 width and least significant bit combined is
 *                 not greater than 64. Check if the value
 *                 fit in the width. Then, by shift the mask
 *                 to where the value is and make them 0 and
 *                 replace it with value.
 *****************************************************************/
uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb, 
                      int64_t value)
{
    assert(width <= 64 && width + lsb <= 64);

    if (Bitpack_fitss(value, width) == false)
    {
        RAISE(Bitpack_Overflow);
    }

    uint64_t mask = ~0;
    mask = left_shift(mask, 64 - width);
    mask = right_shiftu(mask, 64 - width - lsb);
    /* get value at mask */
    value = left_shift(value, lsb);
    value = value & mask;
    /* update input value to existing word */
    word = word & ~mask;
    word = word | value;
    
    return word;
}

/****************************************************************
 * left_shift
 * Description: Left shift bit operation
 * Inputs: 1) Unsigned integer word
 *         2) Width of bit
 * Output: Word that performed left shift on unsigned value
 *         by width amount
 * Implementation: Check if input width is not greater than 64.
 *                 If width is not greater than 63, perform <<
 *                 operation to shift by width.
 *****************************************************************/
uint64_t left_shift(uint64_t word, unsigned width)
{
    assert(width <= 64);

    if (width <= 63)
    {
        return word << width;
    }

    return 0;
}

/****************************************************************
 * right_shiftu
 * Description: Right shift bit operation
 * Inputs: 1) Unsigned integer word
 *         2) Width of bit
 * Output: Word that performed right shift on unsigned value
 *         by width amount
 * Implementation: Check if input width is not greater than 64.
 *                 If width is not greater than 63, perform >>
 *                 operation to shift by width.
 *****************************************************************/
uint64_t right_shiftu(uint64_t word, unsigned width)
{
    assert(width <= 64);

    if (width <= 63)
    {
        return word >> width;
    }

    return 0;
}

/****************************************************************
 * right_shifts
 * Description: Right shift bit operation
 * Inputs: 1) Signed integer word
 *         2) Width of bit
 * Output: Word that perfomed right shift on signed value
 *         by width amount
 * Implementation: Check if input width is not greater than 64.
 *                 If width is not greater than 63, perform >>
 *                 operation to shift by width. If word is unsigned
 *                 then, return 0. Or if negative, return ~0.       
 *****************************************************************/
int64_t right_shifts(int64_t word, unsigned width)
{
    assert(width <= 64);

    if (width <= 63)
    {
        return word >> width;
    }

    if (word >= 0)
    {
        return 0;
    }
    
    else
    {
        return ~0;
    }
}