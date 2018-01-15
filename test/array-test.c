//
// Created by Isaac on 2018/1/13.
//

#include <printf.h>
#include <memory.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "array-test.h"
#include "src/QCArray.h"
#include "src/vendor/sha256.h"
#include "src/vendor/sha512.h"
#include "fft-test.h"

static void sha256_test() {

    QCByte text1[] = {"abc"};
    QCByte text2[] = {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"};

    QCByte hash1[SHA256_BLOCK_SIZE] = {0xba,0x78,0x16,0xbf,0x8f,0x01,0xcf,0xea,0x41,0x41,0x40,0xde,0x5d,0xae,0x22,0x23,
                                       0xb0,0x03,0x61,0xa3,0x96,0x17,0x7a,0x9c,0xb4,0x10,0xff,0x61,0xf2,0x00,0x15,0xad};
    QCByte hash2[SHA256_BLOCK_SIZE] = {0x24,0x8d,0x6a,0x61,0xd2,0x06,0x38,0xb8,0xe5,0xc0,0x26,0x93,0x0c,0x3e,0x60,0x39,
                                       0xa3,0x3c,0xe4,0x59,0x64,0xff,0x21,0x67,0xf6,0xec,0xed,0xd4,0x19,0xdb,0x06,0xc1};

    QCArrayRef a1 = QCArrayCreateWithByte(text1, strlen(text1), false);
    QCArrayRef s1 = QCArraySHA256(a1);
    QCObjectPrint(s1);
    if (QCArrayCompareRaw(s1, hash1, QCDTByte)) {
        printf("equal\n");
    }

    QCArrayRef a2 = QCArrayCreateWithByte(text2, strlen(text2), false);
    QCArrayRef s2 = QCArraySHA256(a2);
    QCObjectPrint(s2);
    if (QCArrayCompareRaw(s2, hash2, QCDTByte)) {
        printf("equal\n");
    }

    QCRelease(a1);
    QCRelease(a2);

    QCRelease(s1);
    QCRelease(s2);
}


#define MAX_PLAINTEXT_SIZE      100

typedef struct
{
    char            PlainText [MAX_PLAINTEXT_SIZE];
    uint32_t        PlainTextSize;                      // 0 to use (uint32_t)strlen
    SHA512_HASH     Sha512Hash;
} TestVector;

static TestVector gTestVectors [] =
        {
                {
                        "", 0,
                        {{0xcf,0x83,0xe1,0x35,0x7e,0xef,0xb8,0xbd,0xf1,0x54,0x28,0x50,0xd6,0x6d,0x80,0x07,
                                 0xd6,0x20,0xe4,0x05,0x0b,0x57,0x15,0xdc,0x83,0xf4,0xa9,0x21,0xd3,0x6c,0xe9,0xce,
                                 0x47,0xd0,0xd1,0x3c,0x5d,0x85,0xf2,0xb0,0xff,0x83,0x18,0xd2,0x87,0x7e,0xec,0x2f,
                                 0x63,0xb9,0x31,0xbd,0x47,0x41,0x7a,0x81,0xa5,0x38,0x32,0x7a,0xf9,0x27,0xda,0x3e}},  // sha512
                },
                {
                        "a", 0,
                        {{0x1f,0x40,0xfc,0x92,0xda,0x24,0x16,0x94,0x75,0x09,0x79,0xee,0x6c,0xf5,0x82,0xf2,
                                 0xd5,0xd7,0xd2,0x8e,0x18,0x33,0x5d,0xe0,0x5a,0xbc,0x54,0xd0,0x56,0x0e,0x0f,0x53,
                                 0x02,0x86,0x0c,0x65,0x2b,0xf0,0x8d,0x56,0x02,0x52,0xaa,0x5e,0x74,0x21,0x05,0x46,
                                 0xf3,0x69,0xfb,0xbb,0xce,0x8c,0x12,0xcf,0xc7,0x95,0x7b,0x26,0x52,0xfe,0x9a,0x75}},  // sha512
                },
                {
                        "aaa", 0,
                        {{0xd6,0xf6,0x44,0xb1,0x98,0x12,0xe9,0x7b,0x5d,0x87,0x16,0x58,0xd6,0xd3,0x40,0x0e,
                                 0xcd,0x47,0x87,0xfa,0xeb,0x9b,0x89,0x90,0xc1,0xe7,0x60,0x82,0x88,0x66,0x4b,0xe7,
                                 0x72,0x57,0x10,0x4a,0x58,0xd0,0x33,0xbc,0xf1,0xa0,0xe0,0x94,0x5f,0xf0,0x64,0x68,
                                 0xeb,0xe5,0x3e,0x2d,0xff,0x36,0xe2,0x48,0x42,0x4c,0x72,0x73,0x11,0x7d,0xac,0x09}},  // sha512
                },
                {
                        "abc", 0,
                        {{0xdd,0xaf,0x35,0xa1,0x93,0x61,0x7a,0xba,0xcc,0x41,0x73,0x49,0xae,0x20,0x41,0x31,
                                 0x12,0xe6,0xfa,0x4e,0x89,0xa9,0x7e,0xa2,0x0a,0x9e,0xee,0xe6,0x4b,0x55,0xd3,0x9a,
                                 0x21,0x92,0x99,0x2a,0x27,0x4f,0xc1,0xa8,0x36,0xba,0x3c,0x23,0xa3,0xfe,0xeb,0xbd,
                                 0x45,0x4d,0x44,0x23,0x64,0x3c,0xe8,0x0e,0x2a,0x9a,0xc9,0x4f,0xa5,0x4c,0xa4,0x9f}},  // sha512
                },
                {
                        "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", 0,
                        {{0x20,0x4a,0x8f,0xc6,0xdd,0xa8,0x2f,0x0a,0x0c,0xed,0x7b,0xeb,0x8e,0x08,0xa4,0x16,
                                 0x57,0xc1,0x6e,0xf4,0x68,0xb2,0x28,0xa8,0x27,0x9b,0xe3,0x31,0xa7,0x03,0xc3,0x35,
                                 0x96,0xfd,0x15,0xc1,0x3b,0x1b,0x07,0xf9,0xaa,0x1d,0x3b,0xea,0x57,0x78,0x9c,0xa0,
                                 0x31,0xad,0x85,0xc7,0xa7,0x1d,0xd7,0x03,0x54,0xec,0x63,0x12,0x38,0xca,0x34,0x45}},  // sha512
                },
                {
                        "The quick brown fox jumps over the lazy dog", 0,
                        {{0x07,0xe5,0x47,0xd9,0x58,0x6f,0x6a,0x73,0xf7,0x3f,0xba,0xc0,0x43,0x5e,0xd7,0x69,
                                 0x51,0x21,0x8f,0xb7,0xd0,0xc8,0xd7,0x88,0xa3,0x09,0xd7,0x85,0x43,0x6b,0xbb,0x64,
                                 0x2e,0x93,0xa2,0x52,0xa9,0x54,0xf2,0x39,0x12,0x54,0x7d,0x1e,0x8a,0x3b,0x5e,0xd6,
                                 0xe1,0xbf,0xd7,0x09,0x78,0x21,0x23,0x3f,0xa0,0x53,0x8f,0x3d,0xb8,0x54,0xfe,0xe6}},  // sha512
                },
                {
                        "The quick brown fox jumps over the lazy dog.", 0,
                        {{0x91,0xea,0x12,0x45,0xf2,0x0d,0x46,0xae,0x9a,0x03,0x7a,0x98,0x9f,0x54,0xf1,0xf7,
                                 0x90,0xf0,0xa4,0x76,0x07,0xee,0xb8,0xa1,0x4d,0x12,0x89,0x0c,0xea,0x77,0xa1,0xbb,
                                 0xc6,0xc7,0xed,0x9c,0xf2,0x05,0xe6,0x7b,0x7f,0x2b,0x8f,0xd4,0xc7,0xdf,0xd3,0xa7,
                                 0xa8,0x61,0x7e,0x45,0xf3,0xc4,0x63,0xd4,0x81,0xc7,0xe5,0x86,0xc3,0x9a,0xc1,0xed}},  // sha512
                },
                {
                        "message digest", 0,
                        {{0x10,0x7d,0xbf,0x38,0x9d,0x9e,0x9f,0x71,0xa3,0xa9,0x5f,0x6c,0x05,0x5b,0x92,0x51,
                                 0xbc,0x52,0x68,0xc2,0xbe,0x16,0xd6,0xc1,0x34,0x92,0xea,0x45,0xb0,0x19,0x9f,0x33,
                                 0x09,0xe1,0x64,0x55,0xab,0x1e,0x96,0x11,0x8e,0x8a,0x90,0x5d,0x55,0x97,0xb7,0x20,
                                 0x38,0xdd,0xb3,0x72,0xa8,0x98,0x26,0x04,0x6d,0xe6,0x66,0x87,0xbb,0x42,0x0e,0x7c}},  // sha512
                },
                {
                        "abcdefghijklmnopqrstuvwxyz", 0,
                        {{0x4d,0xbf,0xf8,0x6c,0xc2,0xca,0x1b,0xae,0x1e,0x16,0x46,0x8a,0x05,0xcb,0x98,0x81,
                                 0xc9,0x7f,0x17,0x53,0xbc,0xe3,0x61,0x90,0x34,0x89,0x8f,0xaa,0x1a,0xab,0xe4,0x29,
                                 0x95,0x5a,0x1b,0xf8,0xec,0x48,0x3d,0x74,0x21,0xfe,0x3c,0x16,0x46,0x61,0x3a,0x59,
                                 0xed,0x54,0x41,0xfb,0x0f,0x32,0x13,0x89,0xf7,0x7f,0x48,0xa8,0x79,0xc7,0xb1,0xf1}},  // sha512
                },
                {
                        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", 0,
                        {{0x1e,0x07,0xbe,0x23,0xc2,0x6a,0x86,0xea,0x37,0xea,0x81,0x0c,0x8e,0xc7,0x80,0x93,
                                 0x52,0x51,0x5a,0x97,0x0e,0x92,0x53,0xc2,0x6f,0x53,0x6c,0xfc,0x7a,0x99,0x96,0xc4,
                                 0x5c,0x83,0x70,0x58,0x3e,0x0a,0x78,0xfa,0x4a,0x90,0x04,0x1d,0x71,0xa4,0xce,0xab,
                                 0x74,0x23,0xf1,0x9c,0x71,0xb9,0xd5,0xa3,0xe0,0x12,0x49,0xf0,0xbe,0xbd,0x58,0x94}},  // sha512
                },
                {   "12345678901234567890123456789012345678901234567890123456789012345678901234567890", 0,
                        {{0x72,0xec,0x1e,0xf1,0x12,0x4a,0x45,0xb0,0x47,0xe8,0xb7,0xc7,0x5a,0x93,0x21,0x95,
                                 0x13,0x5b,0xb6,0x1d,0xe2,0x4e,0xc0,0xd1,0x91,0x40,0x42,0x24,0x6e,0x0a,0xec,0x3a,
                                 0x23,0x54,0xe0,0x93,0xd7,0x6f,0x30,0x48,0xb4,0x56,0x76,0x43,0x46,0x90,0x0c,0xb1,
                                 0x30,0xd2,0xa4,0xfd,0x5d,0xd1,0x6a,0xbb,0x5e,0x30,0xbc,0xb8,0x50,0xde,0xe8,0x43}},  // sha512
                },

        };
#define NUM_TEST_VECTORS ( sizeof(gTestVectors) / sizeof(gTestVectors[0]) )

static void sha512_test() {
    bool            success = true;

    for(size_t i=0; i<NUM_TEST_VECTORS; i++ )
    {
        size_t len = (uint32_t) gTestVectors[i].PlainTextSize ? gTestVectors[i].PlainTextSize : (uint32_t)strlen( gTestVectors[i].PlainText );
        QCArrayRef array = QCArrayCreateWithByte(gTestVectors[i].PlainText, len, false);
        QCArrayRef s1 = QCArraySHA512(array);

        if( !QCArrayCompareRaw(s1, &gTestVectors[i].Sha512Hash, QCDTByte) ) {
            printf( "TestSha512 - Test vector %u failed\n", i );
            success = false;
        }

        QCRelease(array);
        QCRelease(s1);
    }

    if (success) {
        printf("SHA512 success\n");
    }
}

static void array_slice_test() {
    int d[] = {1, 2, 3, 4, 5, 6};
    QCArrayRef array = QCArrayCreateWithInt(d, sizeof(d) / sizeof(d[0]), false);

    QCArrayRef s1 = QCArraySlice(array, 0, 3);
    int expected[] = {1, 2, 3};
    QCArrayCompareRaw(s1, expected, QCDTInt);

    QCRelease(array);
    QCRelease(s1);
}

static void array_hex_test() {
    QCByte stream[] = {0x5e, 0xca, 0x49, 0x4f, 0x5a, 0xb1, 0xf3, 0xd4, 0x8e, 0x1a, 0x37, 0xcd, 0x32, 0x77, 0xc6, 0x92,
                       0x2f, 0x47, 0x6e, 0x50, 0x7c, 0xcc, 0xa2, 0x68, 0x08, 0x68, 0x94, 0x4a, 0x73, 0x31, 0x70, 0x1f,
                       0x91, 0xd8, 0x4e, 0x0a, 0x62, 0x8b, 0x51, 0x92, 0xe7, 0x9d, 0xb1, 0x18, 0x28, 0x99, 0x73, 0x6d};
    const char *hexString = "5eca494f5ab1f3d48e1a37cd3277c6922f476e507ccca2680868944a7331701f91d84e0a628b5192e79db1182899736d";

    QCArrayRef a1 = QCArrayCreateWithByte(stream, sizeof(stream) / sizeof(stream[0]), false);
    QCArrayRef a2 = QCArrayCreateWithHex(hexString, strlen(hexString));

    if(QCObjectEqual(a1, a2)) {
        printf("array hex test passed");
    }
    QCRelease(a1);
    QCRelease(a2);
}

static void base64_test() {
    //
    QCByte stream[] = {0x5e, 0xca, 0x49, 0x4f, 0x5a, 0xb1, 0xf3, 0xd4, 0x8e, 0x1a, 0x37, 0xcd, 0x32, 0x77, 0xc6, 0x92,
                       0x2f, 0x47, 0x6e, 0x50, 0x7c, 0xcc, 0xa2, 0x68, 0x08, 0x68, 0x94, 0x4a, 0x73, 0x31, 0x70, 0x1f,
                       0x91, 0xd8, 0x4e, 0x0a, 0x62, 0x8b, 0x51, 0x92, 0xe7, 0x9d, 0xb1, 0x18, 0x28, 0x99, 0x73, 0x6d};
    const char *baseString = "XspJT1qx89SOGjfNMnfGki9HblB8zKJoCGiUSnMxcB+R2E4KYotRkuedsRgomXNt";

    QCArrayRef a1 = QCArrayCreateWithByte(stream, sizeof(stream) / sizeof(stream[0]), false);
    QCArrayRef a2 = QCArrayCreateWithBase64(baseString, strlen(baseString));

    if(QCObjectEqual(a1, a2)) {
        printf("array base64 test passed");
    }
    QCRelease(a1);
    QCRelease(a2);
}

void array_test() {
    sha256_test();

    sha512_test();

    array_slice_test();

    array_hex_test();

    base64_test();
}
