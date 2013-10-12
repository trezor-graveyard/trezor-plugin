#include "utils.h"

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>

#include "logging.h"

static const char PUBLIC_KEY[] = "XXX";

bool check_signature(const uint8_t *data, size_t datalen,
                     const uint8_t *sig, size_t siglen)
{
    bool ret = false;
    BIO *bufio;
    RSA *rsa;
    EVP_PKEY *key;
    EVP_MD_CTX ctx;

    bufio = BIO_new_mem_buf((void*)PUBLIC_KEY, sizeof(PUBLIC_KEY));
    if (!bufio) {
        FBLOG_FATAL("check_signature()", "PEM_PKEY_new failed");
        goto ret;
    }
    rsa = PEM_read_bio_RSA_PUBKEY(bufio, 0, 0, 0);
    if (!rsa) {
        FBLOG_FATAL("check_signature()", "PEM_read_bio_RSA_PUBKEY failed");
        goto err0;
    }
    key = EVP_PKEY_new();
    if (!key) {
        FBLOG_FATAL("check_signature()", "PEM_PKEY_new failed");
        goto err1;
    }
    if (1 != EVP_PKEY_set1_RSA(key, rsa)) {
        FBLOG_FATAL("check_signature()", "EVK_PKEY_assign_RSA failed");
        goto err2;
    }

    EVP_MD_CTX_init(&ctx);
    if (1 != EVP_VerifyInit(&ctx, EVP_sha256())) {
        FBLOG_FATAL("check_signature()", "EVP_DigestVerifyInit failed");
        goto err3;
    }
	if (1 != EVP_VerifyUpdate(&ctx, data, datalen)) {
        FBLOG_FATAL("check_signature()", "EVP_DigestVerifyUpdate failed");
        goto err3;
    }
    ret = EVP_VerifyFinal(&ctx, sig, siglen, key);

err3:
    EVP_MD_CTX_cleanup(&ctx);
err2:
    EVP_PKEY_free(key);
err1:
    RSA_free(rsa);
err0:
    BIO_free(bufio);
ret:
    return ret;
}
