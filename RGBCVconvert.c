/*************************************************************************
*                            RGBCVconvert.c
* 
* 
*      Authors: Jae Hyun Cheigh (jcheig01), Suyu Lui (sliu21)
*
*      Fall 2020 - COMP40
*      HW 4
* 
* 
*      Summary: Implementation file with functions that convert RGB values
*               of an image to component video values, and vice-versa.
*     
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "RGBCVconvert.h"
#include "assert.h"

/****************************************************************
 * RGBtoCV
 * Description: Convert RGB values to component video values
 * Inputs: 1) PPM image
 *         2) blocksize used for array
 * Output: 2D unboxed array with component video values stored
 * Implementation: Allocate memory for 2D unboxed array and
 *                 use map_block_major to get component video
 *                 values from RGB values of an image
 *****************************************************************/
A2Methods_UArray2 RGBtoCV(Pnm_ppm image, int blocksize)
{
    A2Methods_UArray2 CVarray = image->methods->
                                new_with_blocksize(image->width,
                                                   image->height,
                                                   sizeof(CV),
                                                   blocksize);
    assert(CVarray != NULL);
    /* for each pixel perform computation to convert RGB values
     * to component video values */
    image->methods->map_block_major(CVarray, RGBtoCV_apply, image);

    return CVarray;
}

/****************************************************************
 * RGBtoCV_apply
 * Description: Apply function for RGBtoCV_apply
 * Inputs: 1) Column index
 *         2) Row index
 *         3) 2D array that is mapped
 *         4) Value at current index of 2D array
 *         5) Pointer to closure
 * Output: Void
 * Implementation: Get RGB values from a pixel of an image and
 *                 perform calculation to derive component
 *                 video values y, pb, pr, and store them
 *                 in the 2D array.
 *****************************************************************/
void RGBtoCV_apply(int i, int j, A2Methods_UArray2 array,
                   void *elem, void *cl)
{
    (void) array;

    Pnm_ppm image = (Pnm_ppm) cl;
    Pnm_rgb pixel = (Pnm_rgb) image->methods->at(image->pixels, i, j);

    float r = (float) pixel->red / (float) image->denominator;
    float g = (float) pixel->green / (float) image->denominator;
    float b = (float) pixel->blue / (float) image->denominator;

    float y = 0.299 * r + 0.587 * g + 0.114 * b;
    float pb = -0.168736 * r - 0.331264 * g + 0.5 * b;
    float pr = 0.5 * r - 0.418688 * g - 0.081312 * b;

    /* store values into struct and assign to current pixel */
    CV cv = { y, pb, pr };
    *(CV *) elem = cv;
}

/****************************************************************
 * CVtoRGB
 * Description: Convert component video values to RGB values
 * Inputs: 1) 2D unboxed array (component video array in this case)
 *         2) PPM image
 *         3) blocksize used for array
 * Output: Void
 * Implementation: Allocate memory for pixmap ppm and map through
 *                 component video array to store RGB values in
 *                 pixmap.
 *****************************************************************/
void CVtoRGB(A2Methods_UArray2 array, Pnm_ppm pixmap, int blocksize)
{
    pixmap->pixels = pixmap->methods->
                    new_with_blocksize(pixmap->width, pixmap->height,
                                       sizeof(struct Pnm_rgb), blocksize);
    assert(pixmap->pixels != NULL);

    /* for each pixel perform computation to convert component 
     * video values to RGB values */
    pixmap->methods->map_block_major(array, CVtoRGB_apply, pixmap);
}

/****************************************************************
 * CVtoRGB_apply
 * Description: Apply function for CVtoRGB_apply
 * Inputs: 1) Column index
 *         2) Row index
 *         3) 2D array that is mapped
 *         4) Value at current index of 2D array
 *         5) Pointer to closure
 * Output: Void
 * Implementation: Get component video values from current index
 *                 of the array and perform calculation to derive
 *                 RGB values that is stored in pixmap pixel.
 *****************************************************************/
void CVtoRGB_apply(int i, int j, A2Methods_UArray2 array,
                   void *elem, void *cl)
{
    (void) array;

    Pnm_ppm pixmap = (Pnm_ppm) cl;
    struct Pnm_rgb pixel;

    float y = ((CV *) elem)->y;
    float pb = ((CV *) elem)->pb;
    float pr = ((CV *) elem)->pr;

    float r = rgb_check((1.0 * y) + (0.0 * pb) + (1.402 * pr));
    float g = rgb_check((1.0 * y) - (0.344136 * pb) - (0.714136 * pr));
    float b = rgb_check((1.0 * y) + (1.772 * pb) + (0.0 * pr));

    /* store values into struct and assign to current pixel of pixmap */
    pixel.red = (unsigned) (r * pixmap->denominator);
    pixel.green = (unsigned) (g * pixmap->denominator);
    pixel.blue = (unsigned) (b * pixmap->denominator);
    *((struct Pnm_rgb *) pixmap->methods->at(pixmap->pixels, i, j)) = pixel;
}

/****************************************************************
 * rgb_check
 * Description: Check if RGB is between 0 and 1
 * Inputs: 1) RGB initial value
 * Output: Float type of check value
 * Implementation: Return value unless value is over 1 (return 1)
 *                 and less than 0 (return 0)
 *****************************************************************/
float rgb_check(float value)
{
    if (value > 1.0)
    {
        return 1.0;
    }

    else if (value < 0.0)
    {
        return 0.0;
    }

    return value;
}