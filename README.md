# 40image

Homework 4(Arith)
Date: 10/26/2020

Name: Jae Hyun Cheigh(jcheig01) SuYu Liu(sliu21)

IMPLEMENTATION & ARCHITECTURE

40image:
  In this part we have successfully implemented funtions that can perform
  compress and decompress that are called in 40image.c. First, for compress,
  we use functions in pnm to read input image. We then use A2Methods's 2D
  blocked uarray to hold the transformed data from RGB to component video.
  Each of the elements holds a struct of coefficients value from CV. To 
  implement RGB to CV conversion, we use blocked uarray functions such as 
  new_with_blocksize and map_block_major with blocksize of two in order to 
  map through an array by blocks and perform discrete cosine transform to 
  obtain coefficient values. Then, we use bitpack module's Bitpack_new 
  functions to pack the coefficient values to codewords. We used uarray model
  to hold coded words which are sent to print function that prints the 
  codewords in big-endian order.

  For decompress, we use bitpack module's Bitpack_new to get codewords read 
  by fgetc functions and store in uarray. The coded words are then unpacked by
  Bitpack_get functions to get coefficient values for each block which are
  sent to perform inverse discrete cosine transformation to get component 
  video values for each pixel. Therefore, struct holding information of
  coefficient values from CV are stored in 2D blocked uarray by performing
  map_block_major with blocksize of two. Then we perform necessary
  computation in each block to convert CV values to RGB values which are 
  stored in pixels attributes of PPM struct. Then, we print out the PPM 
  image with Pnm_ppmwrite function.

  In total, our architecture heavily relied on uarray, uarray2b, and
  pnm modules.

bitpack:
  In this part, we have successfully implemented functions that can perform
  bit packing operations. We built Bitpack_fit for signed and unsigned 
  integers to see if the given value fits in the given width of bits,
  Bitpack_get for signed and unsigned integers to extract value from a given
  word at given width and least significant bit, and Bitpack_new for signed
  and unsigned integers to update values of given word at given width and 
  least significant bits with given replacing value. In order to help these
  functions, we separately created left shift function performed on both 
  signed and unsigned bits, right shift for unsigned bits, and right shift
  for signed bits. We used uint64_t, int64_t datatypes and shift operations 
  from c library to implement the these operations for signed and unsigned 
  values. In total, our architecture heavily relies on the standard integer 
  datatype and shift operation in c library.