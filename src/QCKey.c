//
// Created by Isaac on 2018/1/11.
//

#include "QCKey.h"
#include "QCKeyPrivate.h"
#include "QCRandom.h"
#include "QCArrayPrivate.h"
#include <tomcrypt.h>
#include <math.h>

QCKeyConfig kQCDefaultKeyConfig = {
        .length = 4801,
        .weight = 45,
        .error = 42
};

static void QCKeyDeallocate(QCKeyRef key);
static QCKeyRef QCKeyCopy(QCKeyRef key);
static void QCKeyPrint(QCKeyRef key);
static bool QCKeyEqual(QCKeyRef key1, QCKeyRef key2);

static struct QCClass kQCKeyClass = {
        .name = "QCKey",
        .allocator = QCAllocator,
        .size = sizeof(struct QCKey),
        .deallocate = QCKeyDeallocate,
        .copy = QCKeyCopy,
        .print = QCKeyPrint,
        .equal = QCKeyEqual
};

QCKeyRef QCKeyCreatePrivate(QCArrayRef h0, QCArrayRef h1, QCArrayRef h1inv, QCKeyConfig config) {
    QCKeyRef key = QCAllocate(&kQCKeyClass);
    key->h0 = QCRetain(h0);
    key->h1 = QCRetain(h1);
    key->h1inv = QCRetain(h1inv);
    key->length = config.length;
    key->weight = config.weight;
    key->error = config.error;
    return key;
}

QCKeyRef QCKeyCreatePublic(QCArrayRef g, QCKeyConfig config) {
    QCKeyRef key = QCAllocate(&kQCKeyClass);
    key->g = QCRetain(g);
    key->length = config.length;
    key->weight = config.weight;
    key->error = config.error;
    return key;
}

static QCKeyRef QCKeyCopy(QCKeyRef key) {
    if (key) {
        QCKeyRef copy = key->isa->allocator(sizeof(*key));
        copy->length = key->length;
        copy->weight = key->weight;
        copy->error = key->error;
        copy->h0 = (QCArrayRef)QCObjectCopy(key->h0);
        copy->h1 = (QCArrayRef)QCObjectCopy(key->h1);
        copy->h1inv = (QCArrayRef)QCObjectCopy(key->h1inv);
        copy->g = (QCArrayRef)QCObjectCopy(key->g);
        return copy;
    }
    return NULL;
}

static void QCKeyDeallocate(QCKeyRef key) {
    if (key) {
        QCRelease(key->h0);
        QCRelease(key->h1);
        QCRelease(key->h1inv);
        QCRelease(key->g);
    }
}

static void QCKeyPrint(QCKeyRef key) {
    if (key) {
        if (key->g) {
            printf("<QCPublicKey length: %d weight: %d error: %d>", key->length, key->weight, key->error);
        } else {
            printf("<QCPrivateKey length: %d weight: %d error: %d>", key->length, key->weight, key->error);
        }
    }
}

static bool QCKeyEqual(QCKeyRef key1, QCKeyRef key2) {
    if (key1 && key2 && key1->isa == key2->isa) {
        return key1->length == key2->length
               && key1->weight == key2->weight
               && key1->error == key2->error
               && QCObjectEqual(key1->h0, key2->h0)
               && QCObjectEqual(key1->h1, key2->h1)
               && QCObjectEqual(key1->h1inv, key2->h1inv)
               && QCObjectEqual(key1->g, key2->g);
    }
    return false;
}

void QCKeyGeneratePair(QCKeyConfig config, QCKeyRef *privateKey, QCKeyRef *publicKey) {
    QCArrayRef h0 = QCRandomWeightVector(config.length, config.weight);
    QCArrayRef h1 = QCRandomWeightVector(config.length, config.weight);

    BN_CTX *bnCTX = BN_CTX_new();
    BIGNUM *base = NULL;
    BIGNUM *exp = NULL;
    BIGNUM *n = BN_new();

    BN_dec2bn(&base, "2");
    BN_dec2bn(&exp, "1200");
    BN_exp(n, base, exp, bnCTX);
    BN_sub(n, n, base);

    printf("%s", BN_bn2dec(n));

    QCArrayRef h1inv = QCArrayExpPoly(h1, n);

    BN_free(base);
    BN_free(exp);
    BN_free(n);
    BN_CTX_free(bnCTX);

    QCKeyRef privKey = QCKeyCreatePrivate(h0, h1, h1inv, config);

    QCRelease(h0);
    QCRelease(h1);
    QCRelease(h1inv);

    *privateKey = privKey;

    QCArrayRef g = QCArrayMulPoly(h0, h1inv);
    QCKeyRef pubKey = QCKeyCreatePublic(g, config);

    QCRelease(g);

    *publicKey = pubKey;
}

static void _der_tests_print_flexi(ltc_asn1_list* l, unsigned int level)
{
    char buf[1024];
    const char* name = NULL;
    const char* text = NULL;
    ltc_asn1_list* ostring = NULL;
    unsigned int n;

    switch (l->type)
    {
        case LTC_ASN1_BIT_STRING:
            name = "BIT STRING";
            const char *out = malloc(sizeof(char) * l->size);
            size_t ol = 0;
            int ret = der_decode_bit_string(l->data, l->size, out, &ol);
            if (ret == CRYPT_OK) {
                printf("parse ok");
            }
            break;
        case LTC_ASN1_OCTET_STRING:
            name = "OCTET STRING";
            {
                unsigned long ostring_l = l->size;
                /* sometimes there's another sequence in an octet string...
                 * try to decode that... if it fails print out the octet string
                 */
                if (der_decode_sequence_flexi(l->data, &ostring_l, &ostring) == CRYPT_OK) {
                    text = "";
                }
                else {
                    int r;
                    char* s = buf;
                    int sz = sizeof(buf);
                    for (n = 0; n < l->size; ++n) {
                        r = snprintf(s, sz, "%02X", ((unsigned char*)l->data)[n]);
                        if (r < 0 || r >= sz) {
                            fprintf(stderr, "%s boom\n", name);
                            exit(EXIT_FAILURE);
                        }
                        s += r;
                        sz -= r;
                    }
                    text = buf;
                }
            }
            break;
        case LTC_ASN1_SEQUENCE:
            name = "SEQUENCE";
            text = "";
            break;
        case LTC_ASN1_RAW_BIT_STRING:
            name = "RAW BIT STRING";
            break;
    }

    for (n = 0; n < level; ++n) {
        fprintf(stderr, "    ");
    }
    if (name) {
        if (text)
            fprintf(stderr, "%s %s\n", name, text);
        else
            fprintf(stderr, "%s <missing decoding>\n", name);
    } else
        fprintf(stderr, "WTF type=%i\n", l->type);

    if (ostring) {
        _der_tests_print_flexi(ostring, level + 1);
        der_free_sequence_flexi(ostring);
    }

    if (l->child)
        _der_tests_print_flexi(l->child, level + 1);

    if (l->next)
        _der_tests_print_flexi(l->next, level);
}

static QCKeyRef _parsePrivateKeyFile(const QCByte *data, size_t length) {

    size_t bs = kQCDefaultKeyConfig.length;

    ltc_asn1_list *decoded_list;
    size_t len;
    size_t bufLength;
    QCByte *buf = QCAllocator(sizeof(QCByte) * bs);
    int ret = der_decode_sequence_flexi(data, &len, &decoded_list);
    QCArrayRef h0 = NULL;
    QCArrayRef h1 = NULL;
    QCArrayRef h1inv = NULL;

    if (ret != CRYPT_OK) {
        return NULL;
    }
    if (decoded_list->type == LTC_ASN1_SEQUENCE) {
        _der_tests_print_flexi(decoded_list, 0);

        ltc_asn1_list *node = decoded_list->child;
        ret = der_decode_bit_string(node->data, node->size, buf, &bufLength);
        if (ret == CRYPT_OK) {
            h0 = QCArrayCreateWithByte(buf, bufLength, true);

            node = node->next;
            ret = der_decode_bit_string(node->data, node->size, buf, &bufLength);
            if (ret == CRYPT_OK) {
                h1 = QCArrayCreateWithByte(buf, bufLength, true);

                node = node->next;
                ret = der_decode_bit_string(node->data, node->size, buf, &bufLength);
                if (ret == CRYPT_OK) {
                    h1inv = QCArrayCreateWithByte(buf, bufLength, true);
                }
            }
        }

        QCKeyRef privateKey = QCKeyCreatePrivate(h0, h1, h1inv, kQCDefaultKeyConfig);

        QCRelease(h0);
        QCRelease(h1);
        QCRelease(h1inv);
        return privateKey;
    }
}

static QCKeyRef _parsePublicKeyFile(const QCByte *data, size_t length) {

    size_t bs = kQCDefaultKeyConfig.length;

    ltc_asn1_list *decoded_list;
    size_t len;
    size_t bufLength;
    QCByte *buf = QCAllocator(sizeof(QCByte) * bs);
    int ret = der_decode_sequence_flexi(data, &len, &decoded_list);
    QCArrayRef g = NULL;

    if (ret != CRYPT_OK) {
        return NULL;
    }
    if (decoded_list->type == LTC_ASN1_SEQUENCE) {
        ltc_asn1_list *node = decoded_list->child;
        ret = der_decode_bit_string(node->data, node->size, buf, &bufLength);
        if (ret == CRYPT_OK) {
            g = QCArrayCreateWithByte(buf, bufLength, true);
        }
    }

    QCKeyRef key = QCKeyCreatePublic(g, kQCDefaultKeyConfig);
    QCRelease(g);

    return key;
}

#define kBeginTemplate "-----BEGIN %s-----"
#define kEndTemplate "-----END %s-----"
#define kPrivateKeyLabel "PQP PRIVATE KEY"
#define kPublicKeyLabel "PQP PUBLIC KEY"

bool _isKindOfFile(const char *fileContent, const char *label) {
    return strstr(fileContent, label) != NULL;
}

QCByte *_trimFileContent(const char *fileContent, size_t fileLength, size_t *outLength, const char *label) {
    char begin[64] = {'\0'};
    sprintf(begin, kBeginTemplate, label);
    char end[64] = {'\0'};
    sprintf(end, kEndTemplate, label);

    size_t bufferSize = (fileLength - strlen(begin) - strlen(end));
    size_t resultSize = sizeof(QCByte) * bufferSize;
    QCByte *result = QCAllocator(resultSize + 1);
    result[resultSize] = '\0';
    memcpy(result, fileContent + strlen(begin), resultSize);

    if (outLength) {
        *outLength = bufferSize;
    }
    return result;
}

QCByte *_readFileContent(const char *path, size_t *outLength) {

    FILE *fileptr = fopen(path, "rb");  // Open the file in binary mode
    fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
    size_t filelen = (size_t)ftell(fileptr);             // Get the current byte offset in the file
    rewind(fileptr);                      // Jump back to the beginning of the file

    QCByte *buffer = QCAllocator((filelen + 1) * sizeof(QCByte)); // Enough memory for file + \0

    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    ssize_t total = 0;
    while ((read = getline(&line, &len, fileptr)) != -1) {
        // ignore '\n'
        memcpy(buffer + total, line, read - 1);
        total += read - 1;
    }
    if (line) {
        QCDeallocate(line);
    }

    if (outLength) {
        *outLength = total;
    }

    fclose(fileptr); // Close the file

    return buffer;
}

QCKeyRef QCKeyCreateFromPEMFile(const char* filePath) {
    size_t length = 0;
    QCByte *data = _readFileContent(filePath, &length);

    if (_isKindOfFile(data, kPrivateKeyLabel)) {
        QCByte *trimmed = _trimFileContent(data, length, &length, kPrivateKeyLabel);
        QCArrayRef array = QCArrayCreateWithBase64(trimmed, length);

        QCDeallocate(data);
        QCDeallocate(trimmed);

        QCObjectPrint(array);

        QCKeyRef key =_parsePrivateKeyFile(array->data, array->count);
        QCRelease(array);
        return key;
    } else if (_isKindOfFile(data, kPublicKeyLabel)) {
        QCByte *trimmed = _trimFileContent(data, length, &length, kPublicKeyLabel);
        QCArrayRef array = QCArrayCreateWithBase64(trimmed, length);

        QCDeallocate(data);
        QCDeallocate(trimmed);

        QCKeyRef key =_parsePublicKeyFile(array->data, array->count);
        QCRelease(array);
        return key;
    }
    return NULL;
}


static void _savePrivateKeyToPath(QCKeyRef key, const char *path) {
    size_t count = key->h0->count + key->h1->count + key->h1inv->count;
    QCByte *buffer = _QCMallocData(QCDTByte, count, NULL);
    size_t idx = 0;
    size_t total = 0;
    der_encode_bit_string(key->h0->data, key->h0->count, buffer, &idx);
    total += idx;
    der_encode_bit_string(key->h1->data, key->h1->count, buffer + total, &idx);
    total += idx;
    der_encode_bit_string(key->h1inv->data, key->h1inv->count, buffer + total, &idx);
    total += idx;
    const char *b64string = QCEncodeBase64(buffer, total);

    FILE * fp;
    fp = fopen (path, "a");

    fprintf(fp, kBeginTemplate, kPrivateKeyLabel);
    fprintf(fp, "\n");
    fprintf(fp, b64string);
    fprintf(fp, kEndTemplate, kPrivateKeyLabel);
    fprintf(fp, "\n");

    /* close the file*/
    fclose (fp);

    QCDeallocate(buffer);
}


static void _savePublicKeyToPath(QCKeyRef key, const char *path) {
    size_t count = key->h0->count + key->h1->count + key->h1inv->count;
    QCByte *buffer = _QCMallocData(QCDTByte, count, NULL);
    size_t idx = 0;
    size_t total = 0;
    der_encode_bit_string(key->h0->data, key->h0->count, buffer, &idx);
    total += idx;
    der_encode_bit_string(key->h1->data, key->h1->count, buffer + total, &idx);
    total += idx;
    der_encode_bit_string(key->h1inv->data, key->h1inv->count, buffer + total, &idx);
    total += idx;
    const char *b64string = QCEncodeBase64(buffer, total);

    FILE * fp;
    fp = fopen (path, "a");

    fprintf(fp, kBeginTemplate, kPublicKeyLabel);
    fprintf(fp, "\n");
    fprintf(fp, b64string);
    fprintf(fp, kEndTemplate, kPublicKeyLabel);
    fprintf(fp, "\n");

    /* close the file*/
    fclose (fp);

    QCDeallocate(buffer);
}

void QCKeySaveToFile(QCKeyRef key, const char *path) {
    if (key && path) {
        if (key->h0) {
            // is private key
            _savePrivateKeyToPath(key, path);
        } else if (key->g) {
            // is public key
            _savePublicKeyToPath(key, path);
        }
    }
}