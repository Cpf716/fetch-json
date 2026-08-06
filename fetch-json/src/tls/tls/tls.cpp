//
//  tls.cpp
//  fetch-json
//
//  Created by Corey Ferguson on 7/10/26.
//

#include "tls.h"

namespace tls {
    // Non-Member Fields

    class logger _logger;

    // Non-Member Functions

    std::string ca_path() {
        return "/etc/ssl/cert.pem";
    }

    void set_logging(logging value) {
        _logger.level() = value;
    }

    error::error(const std::string what) : fpp::error(what) { }

    // Constructors

    tls_client::tls_client(const std::string hostname, const int port) {
        const char *pers = "mbedtls_client";
        
        mbedtls_net_init(&server_fd);
        mbedtls_ssl_init(&ssl);
        mbedtls_ssl_config_init(&conf);
        mbedtls_x509_crt_init(&cacert);
        mbedtls_entropy_init(&entropy);
        mbedtls_ctr_drbg_init(&ctr_drbg);

        // 1: Error
        mbedtls_debug_set_threshold(1);

        if (_logger.level() == LOG_MORE) {
            // 2: State Change
            mbedtls_debug_set_threshold(2);
        }

        // Define TLS logger callback
        mbedtls_ssl_conf_dbg(&conf, [](void* ctx, int level, const char* file, int line, const char* str) {
            _logger.more(trim_end(str));
        }, NULL);

        // Initialize seed for the RNG
        if ((mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *) pers, strlen(pers)))) {
            // MBEDTLS_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED
            throw tls::error("Entropy source failed");
        }

        // Read ca-bundle.crt
        if (mbedtls_x509_crt_parse_file(&cacert, ca_path().c_str())) {
            throw tls::error("Failed to read cert.pem");
        }

        // Configure SSL
        if (mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) {
            // MBEDTLS_ERR_XXX_ALLOC_FAILED
            throw tls::error("Memory allocation failed");
        }

        // Request HTTP/1.1 connection
        mbedtls_ssl_conf_alpn_protocols(&conf, (const char*[]) { "http/1.1", NULL });

        // Require valid CA certificate
        mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_REQUIRED);
        mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);

        mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);

        // Initialize SSL session
        if (mbedtls_ssl_setup(&ssl, &conf)) {
            // PSA_ERROR_INSUFFICIENT_MEMORY
            throw tls::error("Memory allocation failed");
        }

        int errnum;

        // Set hostname
        if ((errnum = mbedtls_ssl_set_hostname(&ssl, hostname.c_str()))) {
            throw tls::error([errnum] {
                switch (errnum) {
                    case MBEDTLS_ERR_SSL_ALLOC_FAILED:
                        return "Memory allocation failed";
                    case MBEDTLS_ERR_SSL_BAD_INPUT_DATA:
                        return "Bad hostname";
                    default:
                        return "";
                }
            }());
        }
        
        std::vector<dns::host> hosts;

        try {
            hosts = dns::lookup(hostname);

            // Connect to the server
            if ((errnum = mbedtls_net_connect(&server_fd, hosts[0].ip().c_str(), std::to_string(port).c_str(), MBEDTLS_NET_PROTO_TCP))) {
                throw tls::error([errnum] {
                    switch (errnum) {
                        case MBEDTLS_ERR_NET_SOCKET_FAILED:
                            return "Socket failed";
                        case MBEDTLS_ERR_NET_UNKNOWN_HOST:
                            return "Unknown host";
                        case MBEDTLS_ERR_NET_CONNECT_FAILED:
                            return "Connection failed";
                        default:
                            return "";
                    }
                }());
            }

            // Bind socket to the SSL context
            mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

            // Perform SSL handshake
            while ((errnum = mbedtls_ssl_handshake(&ssl)))
                if (!(errnum == MBEDTLS_ERR_SSL_WANT_READ || errnum == MBEDTLS_ERR_SSL_WANT_WRITE))
                    throw tls::error("SSL handshake failed");

            _logger.more("* Connected to " + hostname + "(" + hosts[0].ip() + ") port " + std::to_string(443));
        } catch (dns::error& e) {
            throw tls::error(e.what());
        }
    }

    tls_client::~tls_client() { }

    // Member Functions

    void tls_client::close() {
        mbedtls_net_free(&this->server_fd);
        mbedtls_x509_crt_free(&this->cacert);
        mbedtls_ssl_free(&this->ssl);
        mbedtls_ssl_config_free(&this->conf);
        mbedtls_ctr_drbg_free(&this->ctr_drbg);
        mbedtls_entropy_free(&this->entropy);
    }

    std::string tls_client::recv() {
        int           len;
        unsigned char buf[BUFF_LEN];

        if ((len = mbedtls_ssl_read(&this->ssl, buf, sizeof(buf))) <= 0)
            throw tls::error("Receive failed");
        
        return std::string(reinterpret_cast<const char*>(buf), len);
    }

    int tls_client::send(std::string message) {
        int len = 0,
             total = 0;

        // Repeat until all bytes have been sent
        while (total != message.length()) {
            len = mbedtls_ssl_write(&this->ssl, (const unsigned char *) message.substr(total).c_str(), message.length() - total);

            if (len <= 0)
                throw tls::error("Write failed");

            total += len;
        }

        return total;
    }
}
