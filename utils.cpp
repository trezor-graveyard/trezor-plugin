#include "utils.h"

#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>

static const char PUBLIC_KEY[] = "XXX";

bool check_signature(const uint8_t *data, size_t datalen,
                     const uint8_t *sig, size_t siglen)
{
    bool ret;

    BIO *bufio = BIO_new_mem_buf(PUBLIC_KEY, sizeof(PUBLIC_KEY));
    if (!bufio) {
        FBLOG_FATAL("check_signature()", "PEM_PKEY_new failed");
        ret = false;
        goto ret;
    }
    RSA *rsa = PEM_read_bio_RSA_PUBKEY(bufio, 0, 0, 0);
    if (!rsa) {
        FBLOG_FATAL("check_signature()", "PEM_read_bio_RSA_PUBKEY failed");
        ret = false;
        goto err0;
    }
    EVP_PKEY *key = EVP_PKEY_new();
    if (!key) {
        FBLOG_FATAL("check_signature()", "PEM_PKEY_new failed");
        ret = false;
        goto err1;
    }
    if (1 != EVP_PKEY_set_RSA(key, rsa)) {
        FBLOG_FATAL("check_signature()", "EVK_PKEY_assign_RSA failed");
        goto err2;
    }

    EVP_MD_CTX ctx;
    EVP_MD_CTX_init(&ctx);
    if (1 != EVP_DigestVerifyInit(&ctx, NULL, EVP_sha256(), NULL, key)) {
        FBLOG_FATAL("check_signature()", "EVP_DigestVerifyInit failed");
        ret = false;
        goto err3:
    }
	if (1 != EVP_DigestVerifyUpdate(&ctx, data, datalen)) {
        FBLOG_FATAL("check_signature()", "EVP_DigestVerifyUpdate failed");
        ret = false;
        goto err3:
    }
    ret = EVP_DigestVerifyFinal(&ctx, sig, siglen);

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
