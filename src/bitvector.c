/*  Bit Vector Implementation */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitvector.h"

#define WORDBITS (sizeof(unsigned) * 8)
#define NUM_WORDS(b) (((b) + (WORDBITS-1)) / WORDBITS)

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/*  Allocate a new bit vector and set all of the bits to FALSE.  */

bit_vector *
new_bit_vector (int num_bits)
{
    int num_words = NUM_WORDS(num_bits);
    bit_vector *result = (bit_vector *)malloc(sizeof(bit_vector));
    assert(num_bits >= 0);
    result->num_bits = num_bits;
    result->bits = (unsigned *)calloc(num_words, sizeof(unsigned));
    return result;
}



/*  Change the value of one bit in a vector.  The specified bit is set to
    the given boolean value.  */
inline
void
set_bit (bit_vector *b, int ndx, boolean value)
{
    unsigned mask, old_bits;
    int word = ndx / WORDBITS;
#ifdef DEBUG
    assert(ndx < b->num_bits);
#endif

    mask = 1 << (ndx % WORDBITS);
    old_bits = b->bits[word];
    if (value == FALSE) {
        mask = ~mask;
        b->bits[word] = old_bits & mask;
    } else {
        b->bits[word] = old_bits | mask;
    }
}



/*  Retrieve the value of a bit in a vector.  */

boolean
get_bit (const bit_vector *b, int ndx)
{
    unsigned mask;
    int word = ndx / WORDBITS;

    assert(ndx < b->num_bits);

    mask = 1 << (ndx % WORDBITS);
    return ((b->bits[word] & mask) != 0);
}



/*  Set all of the bits in a vector to one value.  */

void
set_all_bits (bit_vector *b, boolean value)
{
    unsigned new_bits = 0;
    int num_words = NUM_WORDS(b->num_bits);

    if (value) new_bits = ~new_bits;

    unsigned * bbits = b->bits;

    memset(bbits, new_bits, num_words * sizeof(bbits[0]));
}



/*  Subtract bit vectors.  The result replaces the first vector (b = b - c).  */

void
subtract_bits (bit_vector *b, const bit_vector *c)
{
    int i;
    unsigned old_bits, mask;
    int num_words = NUM_WORDS(b->num_bits);

    assert(b->num_bits == c->num_bits);

    for (i = 0; i < num_words; i++) {
        old_bits = b->bits[i];
        mask = ~(c->bits[i]);
        b->bits[i] = old_bits & mask;
    }
}



/*  Boolean AND of two bit vectors.  The result replaces the first vector.  */

void
and_bits (bit_vector *b, const bit_vector *c)
{
    int num_words = NUM_WORDS(b->num_bits);
    int i;

    assert(b->num_bits == c->num_bits);

    unsigned * bbits = b->bits;
    unsigned * cbits = c->bits;

    for (i = 0; i < num_words; i++) {
        bbits[i] = bbits[i] & cbits[i];
    }
}



/*  Boolean OR of two bit vectors.  The result replaces the first vector.  */

void
or_bits (bit_vector *b, const bit_vector *c)
{
    int num_words = NUM_WORDS(b->num_bits);
    int i;

    assert(b->num_bits == c->num_bits);

    for (i = 0; i < num_words; i++) {
        b->bits[i] = (b->bits[i] | c->bits[i]);
    }
}



/*  Copy bit vector "c" to bit vector "b".  */

void
copy_bits (bit_vector *b, const bit_vector *c)
{
    int num_words = NUM_WORDS(b->num_bits);
    int i;

    assert(b->num_bits == c->num_bits);

    for (i = 0; i < num_words; i++) {
        b->bits[i] = c->bits[i];
    }

}



/*  Test if two bit vectors are equal.  */

boolean
bits_are_equal (bit_vector *b, bit_vector *c)
{
    int full_words = b->num_bits / WORDBITS;
    int excess_bits = b->num_bits % WORDBITS;
    unsigned mask = (((unsigned)1) << excess_bits) - 1;
    int i;

    assert(b->num_bits == c->num_bits);

    /* check everything but the last word */
    for (i = 0; i < full_words; i++) {
        if (b->bits[i] != c->bits[i]) return FALSE;
    }

    /* check the remaining bits */
    if (excess_bits) {
        if ((b->bits[full_words] & mask) != (c->bits[full_words] & mask)) {
            return FALSE;
        }
    }

    return TRUE;
}



boolean
bits_are_false (bit_vector *b)
{
    int full_words = b->num_bits / WORDBITS;
    int excess_bits = b->num_bits % WORDBITS;
    unsigned mask = (((unsigned)1) << excess_bits) - 1;
    int i;

    /* check everything but the last word */
    for (i = 0; i < full_words; i++) {
        if (b->bits[i] != 0) return FALSE;
    }

    /* check the remaining bits */
    if (excess_bits) {
        if ((b->bits[full_words] & mask) != 0) {
            return FALSE;
        }
    }

    return TRUE;
}

boolean
bits_are_false_range (bit_vector *b, int from, int to)
{
    int fromword = from / WORDBITS;
    int range_bits = to-from + 1;
    int pre_excess_bits = (WORDBITS - (from % WORDBITS)) % WORDBITS;
    int full_words = (range_bits - pre_excess_bits) / WORDBITS;
    int post_excess_bits = (range_bits - pre_excess_bits) % WORDBITS;
    unsigned pre_mask = ((unsigned)-1) << (WORDBITS - pre_excess_bits);
    unsigned post_mask = ((((unsigned)1) << post_excess_bits)
                          &((unsigned)~0x1)) - 1;
    int i;

    //assert( range_bits <= b->num_bits && from >= 0 && to > from );
    if (!( range_bits <= b->num_bits && from >= 0 && to > from ))
    {
        fprintf(stderr, "<bits_are_false_range> Invalid input\n"
                "range_bits=%d <= b->num_bits=%d from=%d to=%d\n",
                range_bits, b->num_bits, from, to);
        exit(-1);
    }
    else if ( full_words < -1 ) {
        fprintf(stderr, "<bits_are_false_range> full_words has taken\n"
                "a negative value=%d range_bits=%d pre_excess_bits=%d\n"
                "from=%d to=%d pre_mask=0x%x post_mask=0x%x\n",
                   full_words, range_bits, pre_excess_bits, from,to,
                   pre_mask, post_mask);
        exit(-1);
    }

    /* check if range even exceeds a full word */
    if (full_words > 0)
    {

        /* check the bits before the full word */
        if (pre_excess_bits)
        {
            if ((b->bits[fromword] & pre_mask) != 0) {
                return FALSE;
            }
            fromword++;
        }

        /* check everything but the last word */
        for (i = fromword; i < fromword + full_words; i++) {
            if (b->bits[i] != 0) return FALSE;
        }

        /* check the remaining bits */
        if (post_excess_bits) {
            if ((b->bits[full_words] & post_mask) != 0) {
                return FALSE;
            }
        }
    }
    /* if the range is contained within a word */
    else
    {
        pre_mask = pre_mask & post_mask;
        if ((b->bits[fromword] & pre_mask) != 0) {
            return FALSE;
        }
    }

    return TRUE;
}



/* count bits of given value */

unsigned
count_bits (bit_vector *b, boolean v)
{
  unsigned int full_words = b->num_bits / WORDBITS;
  unsigned int excess_bits = b->num_bits % WORDBITS;
  unsigned mask = (((unsigned)1) << excess_bits) - 1;
  unsigned i,x;
  unsigned count = 0;

  for (i=0; i<full_words;i++) {
    if (b->bits[i]) {
      for (x=0;x < WORDBITS;x++)
        if (((b->bits[i] & (1<<x)) >> x) == v)
          count++;
    }
  }

  /* check remaining bits */
  if (excess_bits) {
    if ((b->bits[full_words] & mask) != 0) {
      for (x=0;x < excess_bits;x++)
        if (((b->bits[i] & (1<<x)) >> x) == v)
          count++;
    }
  }

  return count;
}

unsigned
first_bit(bit_vector *b, boolean v)
{
  unsigned int full_words = b->num_bits / WORDBITS;
  unsigned int excess_bits = b->num_bits % WORDBITS;
  unsigned mask = (((unsigned)1) << excess_bits) - 1;
  unsigned i,x;
  unsigned ndx = b->num_bits;

  for (i=0; i<full_words;i++) {
    if (b->bits[i]) {
      for (x=0;x < WORDBITS;x++)
        if (((b->bits[i] & (1<<x)) >> x) == v)
        {
          ndx = i*WORDBITS + x;
          return ndx;
        }
    }
  }

  /* check remaining bits */
  if (excess_bits) {
    if ((b->bits[full_words] & mask) != 0) {
      for (x=0;x < excess_bits;x++)
        if (((b->bits[i] & (1<<x)) >> x) == v)
        {
          ndx = full_words * WORDBITS + x;
          return ndx;
        }
    }
  }

  return ndx;
}

void
downsample_bits (bit_vector *b, int nfactor)
{
    int rdbit, wrbit, wrndx;


    rdbit = 0;
    wrbit = 0;
    wrndx = 0;
    for (rdbit = 0; rdbit < b->num_bits; rdbit += nfactor)
    {
        set_bit(b, wrbit++, get_bit(b, rdbit));
    }

    for ( ; wrbit < b->num_bits; wrbit++)
    {
        set_bit(b, wrbit, FALSE);
    }
}

/*  Print the elements of a bit vector that are TRUE.  The bitprint argument
    specifies a function to print a single bit.  This allows you to print
    sensible output when the bits represent something other than integers.
    The fprint_bit function may be used to print them as integers.  */

void
fprint_bits (FILE *fd, bit_vector *b, bitprint_f bitprint)
{
    int i;

    for (i = 0; i < b->num_bits; i++) {
        if (get_bit(b, i)) {
            fputc(' ', fd);
            bitprint(fd, i);
        }
    }
}

void
fprint_allbits(FILE *fd, bit_vector *b, const char * delimeter)
{
    int i;

    for (i = 0; i < b->num_bits; i++) {
        fprintf(fd, "%d%s", get_bit(b, i) ? (1) : (0), delimeter);
    }

}


void
fprint_bit (FILE *fd, int bit)
{
    fprintf(fd, "%d", bit);
}



/*  Deallocate a bit vector.  */

void
free_bit_vector (bit_vector *b)
{
    free(b->bits);
    free(b);
}



