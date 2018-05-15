/*
 * This file is:
 * Copyright Daniel Silverstone <dsilvers@digital-scurf.org> 2006
 * Copyright Simon McVittie <smcv pseudorandom co uk> 2006
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include <stdio.h>

/* Construct and write out the tables for the gfshare code */

int
main(int argc, char** argv)
{
  unsigned char logs[256];
  unsigned char exps[255];
  unsigned int x;
  unsigned int i;
  
  x = 1;
  for( i = 0; i < 255; ++i ) {
    exps[i] = x;
    logs[x] = i;
    x <<= 1;
    if( x & 0x100 )
      x ^= 0x11d; /* Unset the 8th bit and mix in 0x1d */
  }
  logs[0] = 0; /* can't log(0) so just set it neatly to 0 */
  
  /* The above generation algorithm clearly demonstrates that
   * logs[exps[i]] == i for 0 <= i <= 254
   * exps[logs[i]] == i for 1 <= i <= 255
   */
  
  /* Spew out the tables */
  
  fprintf(stdout, "\
/*\n\
 * This file is autogenerated by gfshare_maketable.\n\
 */\n\
\n\
static unsigned char logs[256] = {\n  ");
  for( i = 0; i < 256; ++i ) {
    fprintf(stdout, "0x%02x", logs[i]);
    if( i == 255 )
      fprintf(stdout, " };\n");
    else if( (i % 8) == 7 )
      fprintf(stdout, ",\n  ");
    else
      fprintf(stdout, ", ");
  }
  
  /* The exp table we output from 0 to 509 because that way when we
   * do the lagrange interpolation we don't have to be quite so strict
   * with staying inside the field which makes it quicker
   */
  
  fprintf(stdout, "\
\n\
static unsigned char exps[510] = {\n  ");
  for( i = 0; i < 510; ++i ) {
    fprintf(stdout, "0x%02x", exps[i % 255]); /* exps[255]==exps[0] */
    if( i == 509 )
      fprintf(stdout, " };\n");
    else if( (i % 8) == 7)
      fprintf(stdout, ",\n  ");
    else
      fprintf(stdout, ", ");
  }
  
  return 0;
}
