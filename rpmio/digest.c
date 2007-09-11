/** \ingroup signature
 * \file rpmio/digest.c
 */

#include "system.h"
#include "rpmio_internal.h"
#include "debug.h"

#ifdef	SHA_DEBUG
#define	DPRINTF(_a)	fprintf _a
#else
#define	DPRINTF(_a)
#endif


/**
 * MD5/SHA1 digest private data.
 */
struct DIGEST_CTX_s {
    rpmDigestFlags flags;	/*!< Bit(s) to control digest operation. */
    uint32_t datalen;		/*!< No. bytes in block of plaintext data. */
    uint32_t paramlen;		/*!< No. bytes of digest parameters. */
    uint32_t digestlen;		/*!< No. bytes of digest. */
    void * param;		/*!< Digest parameters. */
    int (*Reset) (void * param);	/*!< Digest initialize. */
    int (*Update) (void * param, const byte * data, size_t size);	/*!< Digest transform. */
    int (*Digest) (void * param, byte * digest);	/*!< Digest finish. */
};

DIGEST_CTX
rpmDigestDup(DIGEST_CTX octx)
{
    DIGEST_CTX nctx;
    nctx = memcpy(xcalloc(1, sizeof(*nctx)), octx, sizeof(*nctx));
    nctx->param = memcpy(xcalloc(1, nctx->paramlen), octx->param, nctx->paramlen);
    return nctx;
}

DIGEST_CTX
rpmDigestInit(pgpHashAlgo hashalgo, rpmDigestFlags flags)
{
    DIGEST_CTX ctx = xcalloc(1, sizeof(*ctx));
    int xx;

    ctx->flags = flags;

    switch (hashalgo) {
    case PGPHASHALGO_MD5:
	ctx->digestlen = 16;
	ctx->datalen = 64;
/* FIX: union, not void pointer */
	ctx->paramlen = sizeof(md5Param);
	ctx->param = xcalloc(1, ctx->paramlen);
/* FIX: cast? */
	ctx->Reset = (void *) md5Reset;
	ctx->Update = (void *) md5Update;
	ctx->Digest = (void *) md5Digest;
	break;
    case PGPHASHALGO_SHA1:
	ctx->digestlen = 20;
	ctx->datalen = 64;
/* FIX: union, not void pointer */
	ctx->paramlen = sizeof(sha1Param);
	ctx->param = xcalloc(1, ctx->paramlen);
/* FIX: cast? */
	ctx->Reset = (void *) sha1Reset;
	ctx->Update = (void *) sha1Update;
	ctx->Digest = (void *) sha1Digest;
	break;
#if HAVE_BEECRYPT_API_H
    case PGPHASHALGO_SHA256:
	ctx->digestlen = 32;
	ctx->datalen = 64;
/* FIX: union, not void pointer */
	ctx->paramlen = sizeof(sha256Param);
	ctx->param = xcalloc(1, ctx->paramlen);
/* FIX: cast? */
	ctx->Reset = (void *) sha256Reset;
	ctx->Update = (void *) sha256Update;
	ctx->Digest = (void *) sha256Digest;
	break;
    case PGPHASHALGO_SHA384:
	ctx->digestlen = 48;
	ctx->datalen = 128;
/* FIX: union, not void pointer */
	ctx->paramlen = sizeof(sha384Param);
	ctx->param = xcalloc(1, ctx->paramlen);
/* FIX: cast? */
	ctx->Reset = (void *) sha384Reset;
	ctx->Update = (void *) sha384Update;
	ctx->Digest = (void *) sha384Digest;
	break;
    case PGPHASHALGO_SHA512:
	ctx->digestlen = 64;
	ctx->datalen = 128;
/* FIX: union, not void pointer */
	ctx->paramlen = sizeof(sha512Param);
	ctx->param = xcalloc(1, ctx->paramlen);
/* FIX: cast? */
	ctx->Reset = (void *) sha512Reset;
	ctx->Update = (void *) sha512Update;
	ctx->Digest = (void *) sha512Digest;
	break;
#endif
    case PGPHASHALGO_RIPEMD160:
    case PGPHASHALGO_MD2:
    case PGPHASHALGO_TIGER192:
    case PGPHASHALGO_HAVAL_5_160:
    default:
	free(ctx);
	return NULL;
	break;
    }

    xx = (*ctx->Reset) (ctx->param);

DPRINTF((stderr, "*** Init(%x) ctx %p param %p\n", flags, ctx, ctx->param));
    return ctx;
}

/* LCL: ctx->param may be modified, but ctx is abstract @*/
int
rpmDigestUpdate(DIGEST_CTX ctx, const void * data, size_t len)
{
    if (ctx == NULL)
	return -1;

DPRINTF((stderr, "*** Update(%p,%p,%d) param %p \"%s\"\n", ctx, data, len, ctx->param, ((char *)data)));
    return (*ctx->Update) (ctx->param, data, len);
}

int
rpmDigestFinal(DIGEST_CTX ctx, void ** datap, size_t *lenp, int asAscii)
{
    byte * digest;
    char * t;
    int i;

    if (ctx == NULL)
	return -1;
    digest = xmalloc(ctx->digestlen);

DPRINTF((stderr, "*** Final(%p,%p,%p,%d) param %p digest %p\n", ctx, datap, lenp, asAscii, ctx->param, digest));
/* FIX: check rc */
    (void) (*ctx->Digest) (ctx->param, digest);

    /* Return final digest. */
    if (!asAscii) {
	if (lenp) *lenp = ctx->digestlen;
	if (datap) {
	    *datap = digest;
	    digest = NULL;
	}
    } else {
	if (lenp) *lenp = (2*ctx->digestlen) + 1;
	if (datap) {
	    const byte * s = (const byte *) digest;
	    static const char hex[] = "0123456789abcdef";

	    *datap = t = xmalloc((2*ctx->digestlen) + 1);
	    for (i = 0 ; i < ctx->digestlen; i++) {
		*t++ = hex[ (unsigned)((*s >> 4) & 0x0f) ];
		*t++ = hex[ (unsigned)((*s++   ) & 0x0f) ];
	    }
	    *t = '\0';
	}
    }
    if (digest) {
	memset(digest, 0, ctx->digestlen);	/* In case it's sensitive */
	free(digest);
    }
    memset(ctx->param, 0, ctx->paramlen);	/* In case it's sensitive */
    free(ctx->param);
    memset(ctx, 0, sizeof(*ctx));	/* In case it's sensitive */
    free(ctx);
    return 0;
}
