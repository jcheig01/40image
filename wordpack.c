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
*      Summary: Implementation file with functions that handle packing
*               and unpacking 32bit codewords for the 40image program.
*     
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "assert.h"
#include "wordpack.h"
#include "RGBCVconvert.h"
#include "bitpack.h"
#include "arith40.h"

/*
static const int A_COEFF = 511;
static const int BCD_COEFF = 50;

static const unsigned A_WIDTH = 9;
static const unsigned BCD_WIDTH = 5;
static const unsigned PB_WIDTH = 4;
static const unsigned PR_WIDTH = 4;

static const unsigned A_LSB = 23;
static const unsigned B_LSB = 18;
static const unsigned C_LSB = 13;
static const unsigned D_LSB = 8;
static const unsigned PB_LSB = 4;
static const unsigned PR_LSB = 0;
*/

/* challenge values */
/* Max value of bits with width of */
/* 6 is (2^6) - 1 = 63 */
static const int A_COEFF = 63;
static const int BCD_COEFF = 50;

static const unsigned A_WIDTH = 6;
static const unsigned BCD_WIDTH = 6;
static const unsigned PB_WIDTH = 4;
static const unsigned PR_WIDTH = 4;

static const unsigned A_LSB = 26;
static const unsigned B_LSB = 20;
static const unsigned C_LSB = 14;
static const unsigned D_LSB = 8;
static const unsigned PB_LSB = 4;
static const unsigned PR_LSB = 0;


/****************************************************************
 * print_compressed
 * Description: Print out coded words in big-endian
 * Inputs: 1) Unboxed array containg coded words
 *         2) PPM image
 * Output: Void
 * Implementation: For each 32-bit packed word, print out from
 *                 most significant byte by using putchar.
 *****************************************************************/
void print_compressed(UArray_T words, Pnm_ppm image)
{
    unsigned width = image->width;
    unsigned height = image->height;

    printf("COMP40 Compressed image format 2\n%u %u", width, height);
    printf("\n");
        
    for (int i = 0; i < UArray_length(words); i++)
    {
        uint64_t word = *(uint64_t *) UArray_at(words, i);
        /* Print in big-endian by looping through 32bit word
         * from index 24 and decrementing by 8 */
        for (int j = 24; j >= 0; j -= 8)
        {
            putchar(Bitpack_getu(word, 8, j));
        }
    }
}

/****************************************************************
 * wordpack
 * Description: Pack coefficient values into 32bit word.
 * Inputs: 1) Struct holding coeffcient values
 * Output: Unsigned type of packed word
 * Implementation: Perform bitpack new operation to assign values
 *                 given width and least significant bit values
 *                 for each coefficient values.
 *****************************************************************/
uint64_t wordpack(coeff cf)
{
    uint64_t packword = 0;
    
    packword = Bitpack_newu(packword, A_WIDTH, A_LSB, cf.a);
    packword = Bitpack_news(packword, BCD_WIDTH, B_LSB, cf.b);
    packword = Bitpack_news(packword, BCD_WIDTH, C_LSB, cf.c);
    packword = Bitpack_news(packword, BCD_WIDTH, D_LSB, cf.d);
    packword = Bitpack_newu(packword, PB_WIDTH, PB_LSB, cf.pb);
    packword = Bitpack_newu(packword, PR_WIDTH, PR_LSB, cf.pr);

    return packword;
}

/****************************************************************
 * dct
 * Description: Perform discrete cosine transformation.
 * Inputs: 1) Unboxed array of block 2D array with component
 *            video values
 * Output: Computed coefficient values
 * Implementation: Get component video values from the block
 *                 and perform discrete cosine transformation
 *                 to a,b,c,d coefficient values and perform
 *                 quantization for pb and pr coefficient values.
 *****************************************************************/
coeff dct(UArray_T block)
{
    /* get y from component video block */
    float y1 = ((CV *) UArray_at(block, 0))->y;
    float y2 = ((CV *) UArray_at(block, 1))->y;
    float y3 = ((CV *) UArray_at(block, 2))->y;
    float y4 = ((CV *) UArray_at(block, 3))->y;

    float a = (y4 + y3 + y2 + y1) / 4.0;
    float b = bcd_check((y4 + y3 - y2 - y1) / 4.0);
    float c = bcd_check((y4 - y3 + y2 - y1) / 4.0);
    float d = bcd_check((y4 - y3 - y2 + y1) / 4.0);

    unsigned cfa = (unsigned) round(a * A_COEFF);
    signed cfb= (signed) round(b * BCD_COEFF);
    signed cfc = (signed) round(c * BCD_COEFF);
    signed cfd = (signed) round(d * BCD_COEFF);

    float pb = 0.0;
    float pr = 0.0;

    /* sum pb, pr values of each elements of the block */
    for (int i = 0; i < UArray_length(block); i++)
    {
        pb += ((CV *) UArray_at(block, i))->pb;
        pr += ((CV *) UArray_at(block, i))->pr;
    }

    float avgpb = pb / (float) UArray_length(block);
    float avgpr = pr / (float) UArray_length(block);

    unsigned cfpb = Arith40_index_of_chroma(avgpb);
    unsigned cfpr = Arith40_index_of_chroma(avgpr);

    /* store coefficient values to struct */
    coeff cf = { cfa, cfb, cfc, cfd, cfpb, cfpr };

    return cf;
}

/****************************************************************
 * bcd_check
 * Description: Check if b,c,d coefficient value is
 *              between -0.3 and 0.3
 * Inputs: 1) b,c,d coefficient value
 * Output: Modified b,c,d coefficient value
 * Implementation: Return coefficient value unless it's less than
 *                 -0.3 (return -0.3) or greater than 0.3 (return
 *                 0.3).
 *****************************************************************/
float bcd_check(float coeff)
{
    if (coeff < -0.3)
    {
        return -0.3;
    }

    else if (coeff > 0.3)
    {
        return 0.3;
    }

    return coeff;
}

/****************************************************************
 * read_compressed
 * Description: Read compressed file and put them into array
 * Inputs: 1) File pointer
 *         2) PPM pixmap
 *         3) blocksize used for 2D array
 * Output: Unboxed array of coded words
 * Implementation: Allocate array for packed word that will be
 *                 inserted when the 32bit word is read in
 *                 big-endian way from the file.
 *****************************************************************/
UArray_T read_compressed(FILE *fp, Pnm_ppm pixmap, int blocksize)
{
    unsigned width = pixmap->width;
    unsigned height = pixmap->height;

    int word_len = (width * height) / (blocksize * blocksize);

    UArray_T words = UArray_new(word_len, sizeof(uint64_t));
    assert(words != NULL);

    for (int i = 0; i < word_len; i++)
    {
        uint64_t word = 0;
        /* Get codewords in big-endian by looping through 32bit word
         * from index 24 and decrementing by 8 */
        for (int j = 24; j >= 0; j -= 8)
        {
            word = Bitpack_newu(word, 8, j, (uint64_t) fgetc(fp));
        }
        *(uint64_t *) UArray_at(words, i) = word;
    }

    return words;
}

/****************************************************************
 * unpack
 * Description: Unpack word to extract coefficient values.
 * Inputs: 1) Unsigned packed word
 * Output: Struct of coefficient values
 * Implementation: Extract coefficient values using Bitpack_get
 *                 operations.
 *****************************************************************/
coeff unpack(uint64_t packword)
{
    unsigned pr = Bitpack_getu(packword, PR_WIDTH, PR_LSB);
    unsigned pb = Bitpack_getu(packword, PB_WIDTH, PB_LSB);
    signed d = Bitpack_gets(packword, BCD_WIDTH, D_LSB);
    signed c = Bitpack_gets(packword, BCD_WIDTH, C_LSB);
    signed b = Bitpack_gets(packword, BCD_WIDTH, B_LSB);
    signed a = Bitpack_getu(packword, A_WIDTH, A_LSB);

    coeff cf = { a, b, c, d, pb, pr };

    return cf;
}

/****************************************************************
 * inverse_dct
 * Description: Perform inverse discrete cosine transformation to
 *              store component video values in each block.
 * Inputs: 1) Struct holding coefficient values
 *         2) Blocksize used for 2D array
 * Output: Unboxed array of component video values for each block
 * Implementation: Convert chroma-coded pb and pr into pb and pr.
 *                 Perform inverse dct operation listed from spec
 *                 on coefficient values to get component video
 *                 values that is stored in newly allocated block
 *                 array.
 *****************************************************************/
UArray_T inverse_dct(coeff cf, int blocksize)
{
    UArray_T block = UArray_new(blocksize * blocksize,
                                sizeof(CV));
    assert(block != NULL);

    float pb = Arith40_chroma_of_index(cf.pb);
    float pr = Arith40_chroma_of_index(cf.pr);

    /* assign pb,pr for each elements of the block */
    for (int i = 0; i < UArray_length(block); i++)
    {
        ((CV *) UArray_at(block, i))->pb = pb;
        ((CV *) UArray_at(block, i))->pr = pr;
    }

    float a = (float) cf.a / (float) A_COEFF;
    float b = (float) cf.b / (float) BCD_COEFF;
    float c = (float) cf.c / (float) BCD_COEFF;
    float d = (float) cf.d / (float) BCD_COEFF;

    float y1 = a - b - c + d;
    float y2 = a - b + c - d;
    float y3 = a + b - c - d;
    float y4 = a + b + c + d;

    /* assign y component video values to each
     * element of the block */
    ((CV *) UArray_at(block, 0))->y = y1;
    ((CV *) UArray_at(block, 1))->y = y2;
    ((CV *) UArray_at(block, 2))->y = y3;
    ((CV *) UArray_at(block, 3))->y = y4;

    return block;
}

