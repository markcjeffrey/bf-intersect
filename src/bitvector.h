/* Bit Vector Definitions
 * copied from ECE540s Optimizing Compilers
 */

#ifndef BITVECTOR_H
#define BITVECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/*  Bit vectors are stored in arrays of unsigned integers.  The bits are
    numbered beginning with 0.  */

typedef struct bit_vector_struct {
    int num_bits;
    unsigned *bits;
} bit_vector;

#ifndef boolean
#define boolean char
#endif

typedef void (*bitprint_f)(FILE *fd, int bit);

bit_vector *new_bit_vector(int num_bits);
void set_bit(bit_vector *b, int ndx, boolean value);
boolean get_bit(const bit_vector *b, int ndx);
void set_all_bits(bit_vector *b, boolean value);
void subtract_bits(bit_vector *b, const bit_vector *c);
void and_bits(bit_vector *b, const bit_vector *c);
void or_bits(bit_vector *b, const bit_vector *c);
void copy_bits(bit_vector *b, const bit_vector *c);
boolean bits_are_equal(bit_vector *b, bit_vector *c);
boolean bits_are_false(bit_vector *b);
boolean bits_are_false_range(bit_vector *b, int from, int to);
unsigned count_bits(bit_vector *b, boolean v);
unsigned first_bit(bit_vector *b, boolean v);
void downsample_bits(bit_vector *b, int nfactor);
void fprint_bits(FILE *fd, bit_vector *b, bitprint_f bitprint);
void fprint_allbits(FILE *fd, bit_vector *b, const char * delimeter);
void fprint_bit(FILE *fd, int bit);
void free_bit_vector(bit_vector *b);

#ifdef __cplusplus
}
#endif

#endif /* BITVECTOR_H */
