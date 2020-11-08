/*************************************************************************
*                             ppmdiff.c
* 
* 
*      Authors: Jae Hyun Cheigh (jcheig01), Suyu Lui (sliu21)
*
*      Fall 2020 - COMP40
*      HW 4
* 
* 
*      Summary: Implementation file that handles identifying ppm
*               files' difference rate
*               
**************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "a2methods.h"
#include "a2plain.h"
#include "pnm.h"

/* handles command line input */
FILE *process_file(char *argv);
/* perform difference operation */
void diff(Pnm_ppm ppm1, Pnm_ppm ppm2);

int main(int argc, char *argv[])
{
    /* make sure 2 image files are inputted */
    if (argc != 3)
    {
        fprintf(stderr, "Need 2 image files\n");
        exit(EXIT_FAILURE);
    }

    FILE *fp1 = process_file(argv[1]);
    FILE *fp2 = process_file(argv[2]);

    A2Methods_T methods = uarray2_methods_plain;

    Pnm_ppm ppm1 = Pnm_ppmread(fp1, methods);
    Pnm_ppm ppm2 = Pnm_ppmread(fp2, methods);

    if (abs((int) ppm1->height - (int) ppm2->height) > 1 ||
        abs((int) ppm1->height - (int) ppm2->height) > 1)
    {
        fprintf(stderr, "Dimension doesn't match\n");
        fprintf(stdout, "1.0\n");
        exit(EXIT_FAILURE);
    }

    else
    {
        diff(ppm1, ppm2);
    }

    Pnm_ppmfree(&ppm1);
    Pnm_ppmfree(&ppm2);
    fclose(fp1);
    fclose(fp2);

    exit(EXIT_SUCCESS);
}

/****************************************************************
 * process_file
 * Description: Handles command line input and read in files
 * Inputs: 1) Char pointer to command line argument
 * Output: FILE pointer to files
 * Implementation: If - is input, read in file from stdin or else,
 *                 open the file in command line argument.
 *****************************************************************/
FILE *process_file(char *argv)
{
    FILE *fp;

    if (argv[0] == '-')
    {
        fp = stdin;
    }
    else
    {
        fp = fopen(argv, "rb");
    }

    return fp;
}

/****************************************************************
 * diff
 * Description: Perform difference operation
 * Inputs: 1) PPM image 1
 *         2) PPM image 2
 * Output: Void
 * Implementation: Iterate through each pixel by the width and
 *                 height of whichever is smaller in dimension.
 *                 Perform RGB difference for each pixel and
 *                 add their squared values to use to compute
 *                 difference E value.
 *****************************************************************/
void diff(Pnm_ppm ppm1, Pnm_ppm ppm2)
{
    int small_width = ppm1->width;
    int small_height = ppm1->height;
    int denom1 = ppm1->denominator;
    int denom2 = ppm2->denominator;

    float sum = 0;
    /* get smaller width and height */
    if (ppm1->width > ppm2->width)
    {
        small_width = ppm2->width;
    }
    if (ppm1->height > ppm2->height)
    {
        small_height = ppm2->height;
    }

    for (int j = 0; j < small_height; j++)
    {
        for (int i = 0; i < small_width; i++)
        {
            Pnm_rgb pix1 = ppm1->methods->at(ppm1->pixels, i, j);
            Pnm_rgb pix2 = ppm2->methods->at(ppm2->pixels, i, j);

            float r_diff = (((int) pix1->red / (float) denom1) - 
                            ((int) pix2->red) / (float) denom2);
            float g_diff = (((int) pix1->green / (float) denom1) -
                            ((int) pix2->green) / (float) denom2);
            float b_diff = (((int) pix1->blue / (float) denom1) -
                            ((int) pix2->blue) / (float) denom2);

            sum += pow(r_diff, 2) + pow(g_diff, 2) + pow(b_diff, 2);
        }
    }

    float E = sqrt(sum/(3*small_width*small_height));

    fprintf(stdout, "%1.4f\n", E);
}