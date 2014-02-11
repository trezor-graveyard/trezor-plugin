#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/ssl.h>

#include <boost/algorithm/hex.hpp>

#include "logging.h"
#include "utf8_tools.h"

#include "utils.h"
#include "utils_keys.h" // defines signature_keys and signature_keys_length

// openssl marked as deprecated since osx 10.7
#ifdef __APPLE__
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace utils {

static void make_digest(const EVP_MD *type, const uint8_t *data, size_t datalen,
                        uint8_t *dig, size_t *diglen)
{
    EVP_MD_CTX ctx;
    unsigned int digleni;

    EVP_MD_CTX_init(&ctx);
    EVP_DigestInit(&ctx, type);
    EVP_DigestUpdate(&ctx, data, datalen);
    EVP_DigestFinal(&ctx, dig, &digleni); 
    EVP_MD_CTX_cleanup(&ctx);
    *diglen = digleni;
}

static EC_KEY *read_eckey(const char *buf, size_t buflen)
{
    BIO *bio;
    EVP_PKEY *evpkey;
    EC_KEY *eckey = 0;

    bio = BIO_new_mem_buf((void*)buf, buflen);
    if (!bio) {
        FBLOG_FATAL("check_signature()", "BIO_new_mem_buf failed");
        goto ret;
    }
    evpkey = PEM_read_bio_PUBKEY(bio, 0, 0, 0);
    if (!evpkey) {
        FBLOG_FATAL("check_signature()", "PEM_read_bio_PUBKEY failed");
        goto err0;
    }
    eckey = EVP_PKEY_get1_EC_KEY(evpkey);
    if (!eckey) {
        FBLOG_FATAL("check_signature()", "EVP_PKEY_get1_EC_KEY failed");
        goto err1;
    }

err1:
    EVP_PKEY_free(evpkey);
err0:
    BIO_free(bio);
ret:
    return eckey;
}

bool signature_verify(const uint8_t *sig, const uint8_t *data, size_t datalen)
{
    ECDSA_SIG ecsig;
    const uint8_t *sig_r = sig;
    const uint8_t *sig_s = sig + SIGNATURE_LENGTH / 2;
    ecsig.r = BN_bin2bn(sig_r, SIGNATURE_LENGTH / 2, 0);
    ecsig.s = BN_bin2bn(sig_s, SIGNATURE_LENGTH / 2, 0);

    size_t diglen;
    uint8_t dig[EVP_MAX_MD_SIZE];
    make_digest(EVP_sha256(), data, datalen, dig, &diglen);

    for (size_t i = 0; i < signature_keys_count; i++) {
        EC_KEY *eckey = read_eckey(signature_keys[i], strlen(signature_keys[i]));
        if (!eckey) continue;

        int ret = ECDSA_do_verify(dig, diglen, &ecsig, eckey);
        EC_KEY_free(eckey);
        if (ret > 0) return true;
    }

    return false;
}

std::string utf8_encode(const std::wstring &str)
{
    return FB::wstring_to_utf8(str);
}

std::wstring utf8_decode(const std::string &str)
{
    return FB::utf8_to_wstring(str);
}

template<>
std::string hex_encode<std::string>(const std::string &str)
{
    try {
        std::ostringstream stream;
        std::ostream_iterator<char> iterator(stream);
        boost::algorithm::hex(str, iterator);
        return stream.str();
    } catch (const std::exception &e) {
        throw std::invalid_argument("Cannot encode to HEX");
    }
    
}

std::string hex_decode(const std::string &hex)
{
    try {
        std::ostringstream stream;
        std::ostream_iterator<char> iterator(stream);
        boost::algorithm::unhex(hex, iterator);
        return stream.str();
    } catch (const std::exception &e) {
        throw std::invalid_argument("Cannot decode from HEX");
    }
}

}
