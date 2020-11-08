/*************************************************************************
*                              compress.c
* 
* 
*      Authors: Jae Hyun Cheigh (jcheig01), Suyu Lui (sliu21)
*
*      Fall 2020 - COMP40
*      HW 4
* 
* 
*      Summary: Implementation file for compress and decompress options
*               in 40image program.
*     
**************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "a2methods.h"
#include "a2blocked.h"
#include "uarray2b.h"
#include "compress40.h"
#include "RGBCVconvert.h"
#include "wordpack.h"
#include "pnm.h"
#include "assert.h"

#define BLOCKSIZE 2

/* struct holding info of 2D array's blocks,
 * array of codewords, and counter for each
 * pixel iterated during map process */
typedef struct
{
    UArray_T block;
    UArray_T codewords;
    unsigned pix_count;
} codewords_cl;

/* obtain codewords from 2D blocked array of component video value */
UArray_T codewords(A2Methods_UArray2 CVarray, Pnm_ppm image);
void codewords_apply(int i, int j, A2Methods_UArray2 array,
                     void *elem, void *cl);
/* obtain 2D blocked array of component video value from codewords */
A2Methods_UArray2 words_to_cv(UArray_T words, Pnm_ppm image);
void word_to_cv_apply(int i, int j, A2Methods_UArray2 array,
                      void *elem, void *cl);

/****************************************************************
 * compress40
 * Description: Compress operation done when user inputs
 *              -c in command line.
 * Inputs: 1) File pointer to image file
 * Output: Void
 * Implementation: Read in file, check for dimensions of images,
 *                 convert RGB values to component video values,
 *                 get coded words from each blocks, and print out
 *                 result.
 *****************************************************************/
void compress40(FILE *input)
{
    assert(input != NULL);

    A2Methods_T methods = uarray2_methods_blocked;
    assert(methods != NULL);

    Pnm_ppm image = Pnm_ppmread(input, methods);
    assert(image != NULL);

    /* trim odd length image */
    if (image->width % 2 != 0)
    {
        (image->width)--;
    }
    if (image->height % 2 != 0)
    {
        (image->height)--;
    }

    /* convert RGB values to component video */
    A2Methods_UArray2 CVarray = RGBtoCV(image, BLOCKSIZE);
    /* get list of codewords */
    UArray_T words = codewords(CVarray, image);
    /* print out in specific format */
    print_compressed(words, image);

    UArray_free(&words);
    methods->free(&CVarray);
    Pnm_ppmfree(&image);
}

/****************************************************************
 * decompress40
 * Description: Decompress operation done when user inputs
 *              -c in command line.
 * Inputs: 1) File pointer to image file
 * Output: Void
 * Implementation: Read in file, extract coded words, convert
 *                 them to component video values, then convert
 *                 them to RGB values for each pixel, and print
 *                 out the result image.
 *****************************************************************/
void decompress40(FILE *input)
{
    assert(input != NULL);

    unsigned height, width;
    int read = fscanf(input, "COMP40 Compressed image format 2\n%u %u", 
                      &width, &height);
    assert(read == 2);
    int c =getc(input);
    assert(c == '\n');

    A2Methods_T methods = uarray2_methods_blocked;
    assert(methods != NULL);

    struct Pnm_ppm pixmap = { .width = width, .height = height,
                              .denominator = 255, .pixels = NULL,
                              .methods = methods
                            };

    /* read compressed file and extract codewords */
    UArray_T words = read_compressed(input, &pixmap, BLOCKSIZE);
    /* convert codewords to coefficients values of component video */
    A2Methods_UArray2 CVarray = words_to_cv(words, &pixmap);
    /* convert component video to RGB values and store in pixmap pixel */
    CVtoRGB(CVarray, &pixmap, BLOCKSIZE);

    Pnm_ppmwrite(stdout, &pixmap);

    UArray_free(&words);
    methods->free(&CVarray);
    methods->free(&(pixmap.pixels));

}

/****************************************************************
 * codewords
 * Description: Get coded words from each block of component
 *              video array.
 * Inputs: 1) 2D unboxed array of with component video values
 *         2) PPM image file
 * Output: UArray_T unboxed array of coded words
 * Implementation: Allocate memory for block of component video
 *                 array and for coded words that will be stored
 *                 for each block. Map through the array to get
 *                 the desired coded words via block-major way.
 *****************************************************************/
UArray_T codewords(A2Methods_UArray2 CVarray, Pnm_ppm image)
{

    codewords_cl cl;

    cl.block = UArray_new(BLOCKSIZE * BLOCKSIZE, sizeof(CV));
    assert(cl.block != NULL);
    cl.codewords = UArray_new((image->width * image->height) /
                              (BLOCKSIZE * BLOCKSIZE), sizeof(uint64_t));
    assert(cl.codewords != NULL);
    cl.pix_count = 1;
    /* get codewords for each block through block-major mapping */
    image->methods->map_block_major(CVarray, codewords_apply, &cl);

    UArray_free(&(cl.block));
    return cl.codewords;
}

/****************************************************************
 * codewords_apply
 * Description: Apply function for codewords
 * Inputs: 1) Column index
 *         2) Row index
 *         3) 2D array that is mapped
 *         4) Value at current index of 2D array
 *         5) Pointer to closure
 * Output: Void
 * Implementation: At the beginning of each block, pack values
 *                 computed by discrete cosine transform operation
 *                 and pack them to get coded words. Then, store
 *                 the coded words into the array.
 *****************************************************************/
void codewords_apply(int i, int j, A2Methods_UArray2 array,
                     void *elem, void *cl)
{
    (void) array;

    codewords_cl *closure = cl;
    unsigned pix_count = closure->pix_count;
    unsigned block_id = BLOCKSIZE * (i % BLOCKSIZE) + (j % BLOCKSIZE);
    *(CV *) UArray_at(closure->block, block_id) = *(CV *) elem;
    /* perform dct and wordpack operation in the beginning
     * of the block */
    if (pix_count % (BLOCKSIZE * BLOCKSIZE) == 0)
    {
        /* codeword for the block */
        uint64_t word = wordpack(dct(closure->block));

        unsigned codewords_id = (pix_count / (BLOCKSIZE * BLOCKSIZE)) - 1;
        *(uint64_t *) UArray_at(closure->codewords, codewords_id) = word;
    }

    (closure->pix_count)++;
}

/****************************************************************
 * words_to_cv
 * Description: Convery coded words to component video values
 * Inputs: 1) Unboxed array of coded words
 *         2) PPM image
 * Output: 2D unboxed array with component video values
 * Implementation: Allocate memory for component video array
 *                 and map through it to get appropriate values
 *                 in by block-major way.
 *****************************************************************/
A2Methods_UArray2 words_to_cv(UArray_T words, Pnm_ppm pixmap)
{
    A2Methods_UArray2 CVarray = pixmap->methods->
                                new_with_blocksize(pixmap->width,
                                                   pixmap->height,
                                                   sizeof(CV), BLOCKSIZE);
    assert(CVarray != NULL);

    codewords_cl cl;
    cl.codewords = words;
    cl.pix_count = 0;

    pixmap->methods->map_block_major(CVarray, word_to_cv_apply, &cl);

    return CVarray;
}

/****************************************************************
 * words_to_cv_apply
 * Description: Apply function for words_to_cv_apply
 * Inputs: 1) Column index
 *         2) Row index
 *         3) 2D array that is mapped
 *         4) Value at current index of 2D array
 *         5) Pointer to closure
 * Output: Void
 * Implementation: At the beginning of block, perform unpacking
 *                 word to get coefficients and use them for
 *                 inverse discrete transformation. The array of
 *                 component video values will be stored in each
 *                 block.
 *****************************************************************/
void word_to_cv_apply(int i, int j, A2Methods_UArray2 array,
                      void *elem, void *cl)
{
    (void) array;
    codewords_cl *closure = cl;
    unsigned pix_count = closure->pix_count;
    unsigned block_id = BLOCKSIZE * (i % BLOCKSIZE) + (j % BLOCKSIZE);

    /* perform unpack and inverse_dct operation in the beginning
     * of the block */
    if (pix_count % (BLOCKSIZE * BLOCKSIZE) == 0)
    {
        unsigned codewords_id = closure->pix_count / 4;

        uint64_t packword = *(uint64_t *) UArray_at(closure->codewords,
                                                    codewords_id);
        closure->block = inverse_dct(unpack(packword), BLOCKSIZE);
    }

    *(CV *) elem = *(CV *) UArray_at(closure->block, block_id);

    /* deallocate memory at the end of the block */
    if (pix_count % (BLOCKSIZE * BLOCKSIZE) == 3)
    {
        UArray_free(&(closure->block));
    }

    (closure->pix_count)++;
}

