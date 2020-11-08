/*************************************************************************
*                             wordpack.h
* 
* 
*      Authors: Jae Hyun Cheigh (jcheig01), Suyu Lui (sliu21)
*
*      Fall 2020 - COMP40
*      HW 4
* 
* 
*      Summary: Header file for wordpack.c
*     
**************************************************************************/


#ifndef WORDPACK_INCLUDED
#define WORDPACK_INCLUDED

#include <stdint.h>
#include "uarray.h"
#include "pnm.h"

/* struct holding info of cosine coefficients */
typedef struct
{
    unsigned a;
    signed b;
    signed c;
    signed d;
    unsigned pb;
    unsigned pr;
} coeff;

/* print compressed codewords */
void print_compressed(UArray_T words, Pnm_ppm image);
/* pack coeff values into codeword using bitpack */
uint64_t wordpack(coeff cf);
/* perform discrete cosine transform to obtain coeff values */
coeff dct(UArray_T block);
/* check if b, c, d values are between -0.3 and 0.3 */
float bcd_check(float coeff);

/* read compressed codewords */
UArray_T read_compressed(FILE *fp, Pnm_ppm pixmap, int blocksize);
/* unpack codewords into coeff values using bitpack */
coeff unpack(uint64_t packword);
/* perform inverse discrete cosine transform to obtain 
 * array of blocks with component video values */
UArray_T inverse_dct(coeff cf, int blocksize);

#endif