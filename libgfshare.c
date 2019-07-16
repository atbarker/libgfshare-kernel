/*
 * This file is Copyright Daniel Silverstone <dsilvers@digital-scurf.org> 2006
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

//#include "config.h"
#include "libgfshare.h"
#include "libgfshare_tables.h"

#include <linux/slab.h>
#include <linux/random.h>
#include <linux/string.h>
#include <linux/timekeeping.h>

#define XMALLOC malloc

struct _gfshare_ctx {
  uint32_t sharecount;
  uint32_t threshold;
  uint32_t maxsize;
  uint32_t size;
  uint8_t* sharenrs;
  uint8_t* buffer;
  uint32_t buffersize;
};

//TODO see if there are any faster methods to get good random numbers, RDRAND if x86?
static void _gfshare_fill_rand_using_random_bytes(uint8_t* buffer, size_t count){
    get_random_bytes(buffer, count);
}

gfshare_rand_func_t gfshare_fill_rand = _gfshare_fill_rand_using_random_bytes;


/* ------------------------------------------------------[ Preparation ]---- */

static gfshare_ctx * _gfshare_ctx_init_core(const uint8_t *sharenrs, 
		                            uint32_t sharecount, 
					    uint32_t threshold, 
					    size_t maxsize ) 
{
  gfshare_ctx *ctx;

  /* Size must be nonzero, and 1 <= threshold <= sharecount */
  if( maxsize < 1 || threshold < 1 || threshold > sharecount ) {
    return NULL;
  }
  
  ctx = kmalloc( sizeof(struct _gfshare_ctx), GFP_KERNEL);
  if( ctx == NULL )
    return NULL; /* errno should still be set from XMALLOC() */
  
  ctx->sharecount = sharecount;
  ctx->threshold = threshold;
  ctx->maxsize = maxsize;
  ctx->size = maxsize;
  ctx->sharenrs = kmalloc( sharecount, GFP_KERNEL);
  
  if( ctx->sharenrs == NULL ) {
    kfree( ctx );
    return NULL;
  }
  
  memcpy( ctx->sharenrs, sharenrs, sharecount );
  ctx->buffer = kmalloc( sharecount * maxsize, GFP_KERNEL);
  
  if( ctx->buffer == NULL ) {
    kfree( ctx->sharenrs );
    kfree( ctx );
    return NULL;
  }
  
  return ctx;
}

/* Initialise a gfshare context for producing shares */
gfshare_ctx * gfshare_ctx_init_enc(const uint8_t* sharenrs,
                                   uint32_t sharecount,
                                   uint32_t threshold,
                                   size_t maxsize)
{
  int i;

  for (i = 0; i < sharecount; i++) {
    if (sharenrs[i] == 0) {
      /* can't have x[i] = 0 - that would just be a copy of the secret, in
       * theory (in fact, due to the way we use exp/log for multiplication and
       * treat log(0) as 0, it ends up as a copy of x[i] = 1) */
      return NULL;
    }
  }

  return _gfshare_ctx_init_core( sharenrs, sharecount, threshold, maxsize );
}

/* Initialise a gfshare context for recombining shares */
gfshare_ctx* gfshare_ctx_init_dec(const uint8_t* sharenrs,
                                  uint32_t sharecount,
                                  uint32_t threshold,
                                  size_t maxsize)
{
  return _gfshare_ctx_init_core( sharenrs, sharecount, threshold, maxsize );
}

/* Set the current processing size */
int gfshare_ctx_setsize(gfshare_ctx* ctx, size_t size) {
  if(size < 1 || size >= ctx->maxsize) {
    return 1;
  }
  ctx->size = size;
  return 0;
}

/* Free a share context's memory. */
void gfshare_ctx_free(gfshare_ctx* ctx) {
  gfshare_fill_rand( ctx->buffer, ctx->sharecount * ctx->maxsize );
  gfshare_fill_rand( ctx->sharenrs, ctx->sharecount );
  kfree( ctx->sharenrs );
  kfree( ctx->buffer );
  gfshare_fill_rand( (uint8_t*)ctx, sizeof(struct _gfshare_ctx) );
  kfree( ctx );
}

/* --------------------------------------------------------[ Splitting ]---- */
/* Provide a secret to the encoder. (this re-scrambles the coefficients) */
void gfshare_ctx_enc_setsecret( gfshare_ctx* ctx, const uint8_t* secret) {
  memcpy(ctx->buffer + ((ctx->threshold-1) * ctx->maxsize), secret, ctx->size);
  gfshare_fill_rand(ctx->buffer, (ctx->threshold-1) * ctx->maxsize);
}

/* Extract a share from the context. 
 * 'share' must be preallocated and at least 'size' bytes long.
 * 'sharenr' is the index into the 'sharenrs' array of the share you want.
 */
int gfshare_ctx_enc_getshares(const gfshare_ctx* ctx,
		              const uint8_t* secret,
                              uint8_t** shares)
{
  uint32_t pos, coefficient;
  uint8_t *share_ptr;
  uint64_t time;
  int i;

  time = ktime_get_ns();  
  memcpy(ctx->buffer + ((ctx->threshold-1) * ctx->maxsize), secret, ctx->size);
  gfshare_fill_rand(ctx->buffer, (ctx->threshold-1) * ctx->maxsize);
  printk(KERN_INFO "time to generate random bytes: %lld", ktime_get_ns() - time);

  for(i = 0; i < ctx->sharecount; i++) {
    uint32_t ilog = logs[ctx->sharenrs[i]];
    uint8_t *coefficient_ptr = ctx->buffer;

    time = ktime_get_ns();
    memcpy(shares[i], coefficient_ptr++, ctx->size);
    coefficient_ptr += ctx->size - 1;

    for(coefficient = 1; coefficient < ctx->threshold; ++coefficient) {
      share_ptr = shares[i];
      coefficient_ptr = ctx->buffer + coefficient * ctx->maxsize;
      for(pos = 0; pos < ctx->size; ++pos) {
        uint8_t share_byte = *share_ptr;
        if(share_byte) {
          share_byte = exps[ilog + logs[share_byte]];
        }
        *share_ptr++ = share_byte ^ *coefficient_ptr++;
      }
    }
    printk(KERN_INFO "time to generate share: %lld", ktime_get_ns() - time);
  }
  return 0;
}

/* ----------------------------------------------------[ Recombination ]---- */

/* Inform a recombination context of a change in share indexes */
void gfshare_ctx_dec_newshares( gfshare_ctx* ctx, const uint8_t* sharenrs) {
  memcpy(ctx->sharenrs, sharenrs, ctx->sharecount);
}

/* Provide a share context with one of the shares.
 * The 'sharenr' is the index into the 'sharenrs' array
 */
int gfshare_ctx_dec_giveshare(gfshare_ctx* ctx, uint8_t sharenr, const uint8_t* share) {
  if(sharenr >= ctx->sharecount) {
    return 1;
  }
  memcpy(ctx->buffer + (sharenr * ctx->maxsize), share, ctx->size);
  return 0;
}

/* Extract the secret by interpolation of the shares.
 * secretbuf must be allocated and at least 'size' bytes long
 */
void gfshare_ctx_dec_extract(const gfshare_ctx* ctx, uint8_t* secretbuf) {
  uint32_t i, j, n, jn;
  uint8_t *secret_ptr, *share_ptr;

  memset(secretbuf, 0, ctx->size);
  
  for(n = i = 0; n < ctx->threshold && i < ctx->sharecount; ++n, ++i) {
    /* Compute L(i) as per Lagrange Interpolation */
    unsigned Li_top = 0, Li_bottom = 0;
    
    if(ctx->sharenrs[i] == 0) {
      n--;
      continue; /* this share is not provided. */
    }
    
    for(jn = j = 0; jn < ctx->threshold && j < ctx->sharecount; ++jn, ++j) {
      if(i == j) {
	continue;
      }
      if(ctx->sharenrs[j] == 0) {
        jn--;
        continue; /* skip empty share */
      }
      Li_top += logs[ctx->sharenrs[j]];
      Li_bottom += logs[(ctx->sharenrs[i]) ^ (ctx->sharenrs[j])];
    }
    Li_bottom %= 0xff;
    Li_top += 0xff - Li_bottom;
    Li_top %= 0xff;
    /* Li_top is now log(L(i)) */
    
    secret_ptr = secretbuf; share_ptr = ctx->buffer + (ctx->maxsize * i);
    for(j = 0; j < ctx->size; ++j) {
      if(*share_ptr) {
        *secret_ptr ^= exps[Li_top + logs[*share_ptr]];
      }
      share_ptr++; secret_ptr++;
    }
  }
}

void gfshare_ctx_dec_decode(const gfshare_ctx* ctx, uint8_t* sharenrs, uint8_t** shares, uint8_t* secretbuf) {
  uint32_t i, j, n, jn;
  uint8_t *secret_ptr, *share_ptr;

  memcpy(ctx->sharenrs, sharenrs, ctx->sharecount);
  memset(secretbuf, 0, ctx->size);
  for(i = 0; i < ctx->sharecount; i++){
      memcpy(ctx->buffer + (i * ctx->maxsize), shares[i], ctx->size);
  }

  for(n = i = 0; n < ctx->threshold && i < ctx->sharecount; ++n, ++i) {
    /* Compute L(i) as per Lagrange Interpolation */
    unsigned Li_top = 0, Li_bottom = 0;

    if(ctx->sharenrs[i] == 0) {
      n--;
      continue; /* this share is not provided. */
    }

    for(jn = j = 0; jn < ctx->threshold && j < ctx->sharecount; ++jn, ++j) {
      if(i == j) {
        continue;
      }
      if(ctx->sharenrs[j] == 0) {
        jn--;
        continue; /* skip empty share */
      }
      Li_top += logs[ctx->sharenrs[j]];
      Li_bottom += logs[(ctx->sharenrs[i]) ^ (ctx->sharenrs[j])];
    }
    Li_bottom %= 0xff;
    Li_top += 0xff - Li_bottom;
    Li_top %= 0xff;
    /* Li_top is now log(L(i)) */

    secret_ptr = secretbuf; share_ptr = ctx->buffer + (ctx->maxsize * i);
    for(j = 0; j < ctx->size; ++j) {
      if(*share_ptr) {
        *secret_ptr ^= exps[Li_top + logs[*share_ptr]];
      }
      share_ptr++; secret_ptr++;
    }
  }
}
