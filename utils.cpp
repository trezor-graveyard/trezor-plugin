#include "utils.h"

#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/ssl.h>

#include <boost/algorithm/hex.hpp>

#include "logging.h"

static const char X509_CERT[] =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIBajCCARmgAwIBAgIJAIvilCV4UmbIMAkGByqGSM49BAEwDTELMAkGA1UEBhMC\n"
    "Q1MwHhcNMTMxMDEzMDkzNzEzWhcNMTMxMTEyMDkzNzEzWjANMQswCQYDVQQGEwJD\n"
    "UzBOMBAGByqGSM49AgEGBSuBBAAhAzoABG+bgXlu3LkwqRi/H+ZQ+xZ7gVsunnhK\n"
    "5OnY5miFqHPq/kTfgzeiDAh6Qpvwf6uHj1jIbgRuZbRgo24wbDAdBgNVHQ4EFgQU\n"
    "WQUM9OJ9RhD8H/TndbmEuNhS/2QwPQYDVR0jBDYwNIAUWQUM9OJ9RhD8H/TndbmE\n"
    "uNhS/2ShEaQPMA0xCzAJBgNVBAYTAkNTggkAi+KUJXhSZsgwDAYDVR0TBAUwAwEB\n"
    "/zAJBgcqhkjOPQQBA0AAMD0CHQCU6Rll+Cq00jzY4/gnT0k1L44d1ZvlOvRgED/s\n"
    "Ahxb1AEOZf5Ls8v8Pfzd63KgswFxdPCz5KuVlk0p\n"
    "-----END CERTIFICATE-----\n";

// openssl marked as deprecated since osx 10.7
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
bool check_signature(const uint8_t *data, size_t datalen,
                     const uint8_t *sig, size_t siglen)
{
    bool ret = false;
    BIO *bufio;
    EVP_PKEY *evpkey;
    EC_KEY *eckey;
    X509 *cert;

    bufio = BIO_new_mem_buf((void*)X509_CERT, sizeof(X509_CERT));
    if (!bufio) {
        FBLOG_FATAL("check_signature()", "BIO_new_mem_buf failed");
        goto ret;
    }
    cert = PEM_read_bio_X509(bufio, 0, 0, 0);
    if (!cert) {
        FBLOG_FATAL("check_signature()", "PEM_read_bio_X509 failed");
        goto err0;
    }
    evpkey = X509_get_pubkey(cert);
    if (!evpkey) {
        FBLOG_FATAL("check_signature()", "X509_get_pubkey failed");
        goto err1;
    }
    eckey = EVP_PKEY_get1_EC_KEY(evpkey);
    if (!eckey) {
        FBLOG_FATAL("check_signature()", "EVP_PKEY_get1_EC_KEY failed");
        goto err2;
    }

    // TODO: compute the data digest?
    ret = ECDSA_verify(0, data, datalen, sig, siglen, eckey);

    EC_KEY_free(eckey);
err2:
    EVP_PKEY_free(evpkey);
err1:
    X509_free(cert);
err0:
    BIO_free(bufio);
ret:
    return ret;
}
#pragma clang diagnostic pop

template<>
std::string hex_encode<std::string>(const std::string &str)
{
    std::ostringstream stream;
    std::ostream_iterator<char> iterator(stream);
    boost::algorithm::hex(str, iterator);
    return stream.str();
}

std::string hex_decode(const std::string &hex)
{
    std::ostringstream stream;
    std::ostream_iterator<char> iterator(stream);
    boost::algorithm::unhex(hex, iterator);
    return stream.str();
}
