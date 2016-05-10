#ifndef HTTPS_CERTIFICATE_EXT_H
#define HTTPS_CERTIFICATE_EXT_H
/**
 * This file implements extension for client certificates for HTTPS protocol.
 * Certificate and key need to be in unencrypted (empty passphrase) DER format.
 *
 * Example for making keys:
 * generate the private key (a 1024 bit key is used in this example)
 * > openssl genrsa -out my_private_key.pem 1024 
 *
 * convert the private key into DER format
 * > openssl rsa -in ./my_private_key.pem -out ./my_private_key -outform DER
 *
 * generate the certificate request using the previously generated private key
 * > openssl req -new -key my_private_key.pem -out my_cert_req.pem
 *
 * get the certificate from Certificate Authority
 * (see other guides if you are managing your own CA)
 *
 * convert the certificate into DER format 
 * > openssl x509 -in my_cert.pem -out my_cert -outform DER
 *
 * Then you can use SPIFFS or convert the keys to C code header with
 * > xxd -i client.crt > MyCertificates.h
 * > xxd -i client.key >> MyCertificates.h
 *
 * Then you can use these classes to do the job:
 * #include "MyCertificates.h"
 * HttpsCertificateClient http;
 * http.begin(url, fingerprint, client_crt, client_crt_len, client_key, client_key_len);
 */
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <memory>

class TransportTraits
{
    public:
        virtual ~TransportTraits() { }

        virtual std::unique_ptr<WiFiClient> create() {
            return std::unique_ptr<WiFiClient>(new WiFiClient()); 
        } 
        virtual bool verify(WiFiClient& client, const char* host) {
            return true; 
        } 
};


class TLSTraits : public TransportTraits
{
    public:
        TLSTraits(const String& fingerprint) : _fingerprint(fingerprint) {}
    protected:
        String _fingerprint;
};

class ClientCertificateTLSTraits : public TLSTraits {
public:
    ClientCertificateTLSTraits(
            const String& fingerprint,
            const uint8_t* certificate,
            const uint16_t cert_len,
            const uint8_t* private_key,
            const uint16_t key_len)
    : TLSTraits (fingerprint)
      , _cert(certificate)
      , _certLen(cert_len)
      , _key(private_key)
      , _keyLen(key_len)
    { };

    std::unique_ptr<WiFiClient> create() override
    {
        auto espClient = new WiFiClientSecure();

        espClient->setCertificate(_cert, _certLen);
        espClient->setPrivateKey(_key, _keyLen);

        return std::unique_ptr<WiFiClient>(espClient);
    };
protected:
    const uint8_t* _cert;
    const uint16_t _certLen;
    const uint8_t* _key;
    const uint16_t _keyLen;
};

class HttpsCertificateClient : public HTTPClient {
public:
    bool begin(String url,
            String httpsFingerprint,
            const uint8_t* certificate,
            const uint16_t cert_len,
            const uint8_t* private_key,
            const uint16_t key_len)
    {
        _transportTraits.reset(nullptr);
        _port = 443;
        if (httpsFingerprint.length() == 0
                || cert_len < 100
                || key_len < 100) {
            return false;
        }
        if (!beginInternal(url, "https")) {
            return false;

        }
        _transportTraits = TransportTraitsPtr(
                new ClientCertificateTLSTraits(
                    httpsFingerprint, certificate, cert_len, private_key, key_len));
        //DEBUG_HTTPCLIENT("[HTTP-Client][begin] httpsFingerprint: %s\n", httpsFingerprint.c_str());
        return true;

    };

};

class ESPCertificateUpdate : ESP8266HTTPUpdate {
    public:
    t_httpUpdate_return update(
            const String& url,
            const String& currentVersion,
            const String& fingerprint,
            const uint8_t* certificate,
            const uint16_t cert_len,
            const uint8_t* private_key,
            const uint16_t key_len) {

        HttpsCertificateClient http;
        http.begin(url, fingerprint, certificate, cert_len, private_key, key_len);
        return handleUpdate(http, currentVersion);
    };
};

#endif
