/*************************************************************************
*                            RGBCVconvert.h
* 
* 
*      Authors: Jae Hyun Cheigh (jcheig01), Suyu Lui (sliu21)
*
*      Fall 2020 - COMP40
*      HW 4
* 
* 
*      Summary: Header file for RGBCVconvert.c
*     
**************************************************************************/

#ifndef RGBCVCONVERT_INCLUDED
#define RGBCVCONVERT_INCLUDED

#include "pnm.h"
#include "a2methods.h"
#include "a2blocked.h"
#include "uarray2b.h"

/* struct holding info of component video */
typedef struct
{
    float y;
    float pb;
    float pr;
} CV;

/* converts RGB values to component video values in the array */
A2Methods_UArray2 RGBtoCV(Pnm_ppm image, int blocksize);
void RGBtoCV_apply(int i, int j, A2Methods_UArray2 array,
                   void *elem, void *cl);

/* check if rgb value is between 0 and 1 */
float rgb_check(float value);

/* converts component video values to RGB values in the array */
void CVtoRGB(A2Methods_UArray2 array, Pnm_ppm pixmap,
                int blocksize);
void CVtoRGB_apply(int i, int j, A2Methods_UArray2 array,
                   void *elem, void *cl);

#endif