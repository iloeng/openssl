/*
 * Copyright 1995-2023 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/*
 * DES low level APIs are deprecated for public use, but still ok for internal
 * use.
 */
#include "internal/deprecated.h"

#include <openssl/e_os2.h>
#include <string.h>

#include "testutil.h"
#include "internal/nelem.h"

#ifndef OPENSSL_NO_DES
# include <openssl/des.h>

/* In case any platform doesn't use unsigned int for its checksums */
# define TEST_cs_eq  TEST_uint_eq

# define DATA_BUF_SIZE      20

/* tisk tisk - the test keys don't all have odd parity :-( */
/* test data */
# define NUM_TESTS 34
static unsigned char key_data[NUM_TESTS][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11},
    {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF},
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10},
    {0x7C, 0xA1, 0x10, 0x45, 0x4A, 0x1A, 0x6E, 0x57},
    {0x01, 0x31, 0xD9, 0x61, 0x9D, 0xC1, 0x37, 0x6E},
    {0x07, 0xA1, 0x13, 0x3E, 0x4A, 0x0B, 0x26, 0x86},
    {0x38, 0x49, 0x67, 0x4C, 0x26, 0x02, 0x31, 0x9E},
    {0x04, 0xB9, 0x15, 0xBA, 0x43, 0xFE, 0xB5, 0xB6},
    {0x01, 0x13, 0xB9, 0x70, 0xFD, 0x34, 0xF2, 0xCE},
    {0x01, 0x70, 0xF1, 0x75, 0x46, 0x8F, 0xB5, 0xE6},
    {0x43, 0x29, 0x7F, 0xAD, 0x38, 0xE3, 0x73, 0xFE},
    {0x07, 0xA7, 0x13, 0x70, 0x45, 0xDA, 0x2A, 0x16},
    {0x04, 0x68, 0x91, 0x04, 0xC2, 0xFD, 0x3B, 0x2F},
    {0x37, 0xD0, 0x6B, 0xB5, 0x16, 0xCB, 0x75, 0x46},
    {0x1F, 0x08, 0x26, 0x0D, 0x1A, 0xC2, 0x46, 0x5E},
    {0x58, 0x40, 0x23, 0x64, 0x1A, 0xBA, 0x61, 0x76},
    {0x02, 0x58, 0x16, 0x16, 0x46, 0x29, 0xB0, 0x07},
    {0x49, 0x79, 0x3E, 0xBC, 0x79, 0xB3, 0x25, 0x8F},
    {0x4F, 0xB0, 0x5E, 0x15, 0x15, 0xAB, 0x73, 0xA7},
    {0x49, 0xE9, 0x5D, 0x6D, 0x4C, 0xA2, 0x29, 0xBF},
    {0x01, 0x83, 0x10, 0xDC, 0x40, 0x9B, 0x26, 0xD6},
    {0x1C, 0x58, 0x7F, 0x1C, 0x13, 0x92, 0x4F, 0xEF},
    {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01},
    {0x1F, 0x1F, 0x1F, 0x1F, 0x0E, 0x0E, 0x0E, 0x0E},
    {0xE0, 0xFE, 0xE0, 0xFE, 0xF1, 0xFE, 0xF1, 0xFE},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF},
    {0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10}
};

static unsigned char plain_data[NUM_TESTS][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11},
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11},
    {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF},
    {0x01, 0xA1, 0xD6, 0xD0, 0x39, 0x77, 0x67, 0x42},
    {0x5C, 0xD5, 0x4C, 0xA8, 0x3D, 0xEF, 0x57, 0xDA},
    {0x02, 0x48, 0xD4, 0x38, 0x06, 0xF6, 0x71, 0x72},
    {0x51, 0x45, 0x4B, 0x58, 0x2D, 0xDF, 0x44, 0x0A},
    {0x42, 0xFD, 0x44, 0x30, 0x59, 0x57, 0x7F, 0xA2},
    {0x05, 0x9B, 0x5E, 0x08, 0x51, 0xCF, 0x14, 0x3A},
    {0x07, 0x56, 0xD8, 0xE0, 0x77, 0x47, 0x61, 0xD2},
    {0x76, 0x25, 0x14, 0xB8, 0x29, 0xBF, 0x48, 0x6A},
    {0x3B, 0xDD, 0x11, 0x90, 0x49, 0x37, 0x28, 0x02},
    {0x26, 0x95, 0x5F, 0x68, 0x35, 0xAF, 0x60, 0x9A},
    {0x16, 0x4D, 0x5E, 0x40, 0x4F, 0x27, 0x52, 0x32},
    {0x6B, 0x05, 0x6E, 0x18, 0x75, 0x9F, 0x5C, 0xCA},
    {0x00, 0x4B, 0xD6, 0xEF, 0x09, 0x17, 0x60, 0x62},
    {0x48, 0x0D, 0x39, 0x00, 0x6E, 0xE7, 0x62, 0xF2},
    {0x43, 0x75, 0x40, 0xC8, 0x69, 0x8F, 0x3C, 0xFA},
    {0x07, 0x2D, 0x43, 0xA0, 0x77, 0x07, 0x52, 0x92},
    {0x02, 0xFE, 0x55, 0x77, 0x81, 0x17, 0xF1, 0x2A},
    {0x1D, 0x9D, 0x5C, 0x50, 0x18, 0xF7, 0x28, 0xC2},
    {0x30, 0x55, 0x32, 0x28, 0x6D, 0x6F, 0x29, 0x5A},
    {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF},
    {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF},
    {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
};

static unsigned char cipher_data[NUM_TESTS][8] = {
    {0x8C, 0xA6, 0x4D, 0xE9, 0xC1, 0xB1, 0x23, 0xA7},
    {0x73, 0x59, 0xB2, 0x16, 0x3E, 0x4E, 0xDC, 0x58},
    {0x95, 0x8E, 0x6E, 0x62, 0x7A, 0x05, 0x55, 0x7B},
    {0xF4, 0x03, 0x79, 0xAB, 0x9E, 0x0E, 0xC5, 0x33},
    {0x17, 0x66, 0x8D, 0xFC, 0x72, 0x92, 0x53, 0x2D},
    {0x8A, 0x5A, 0xE1, 0xF8, 0x1A, 0xB8, 0xF2, 0xDD},
    {0x8C, 0xA6, 0x4D, 0xE9, 0xC1, 0xB1, 0x23, 0xA7},
    {0xED, 0x39, 0xD9, 0x50, 0xFA, 0x74, 0xBC, 0xC4},
    {0x69, 0x0F, 0x5B, 0x0D, 0x9A, 0x26, 0x93, 0x9B},
    {0x7A, 0x38, 0x9D, 0x10, 0x35, 0x4B, 0xD2, 0x71},
    {0x86, 0x8E, 0xBB, 0x51, 0xCA, 0xB4, 0x59, 0x9A},
    {0x71, 0x78, 0x87, 0x6E, 0x01, 0xF1, 0x9B, 0x2A},
    {0xAF, 0x37, 0xFB, 0x42, 0x1F, 0x8C, 0x40, 0x95},
    {0x86, 0xA5, 0x60, 0xF1, 0x0E, 0xC6, 0xD8, 0x5B},
    {0x0C, 0xD3, 0xDA, 0x02, 0x00, 0x21, 0xDC, 0x09},
    {0xEA, 0x67, 0x6B, 0x2C, 0xB7, 0xDB, 0x2B, 0x7A},
    {0xDF, 0xD6, 0x4A, 0x81, 0x5C, 0xAF, 0x1A, 0x0F},
    {0x5C, 0x51, 0x3C, 0x9C, 0x48, 0x86, 0xC0, 0x88},
    {0x0A, 0x2A, 0xEE, 0xAE, 0x3F, 0xF4, 0xAB, 0x77},
    {0xEF, 0x1B, 0xF0, 0x3E, 0x5D, 0xFA, 0x57, 0x5A},
    {0x88, 0xBF, 0x0D, 0xB6, 0xD7, 0x0D, 0xEE, 0x56},
    {0xA1, 0xF9, 0x91, 0x55, 0x41, 0x02, 0x0B, 0x56},
    {0x6F, 0xBF, 0x1C, 0xAF, 0xCF, 0xFD, 0x05, 0x56},
    {0x2F, 0x22, 0xE4, 0x9B, 0xAB, 0x7C, 0xA1, 0xAC},
    {0x5A, 0x6B, 0x61, 0x2C, 0xC2, 0x6C, 0xCE, 0x4A},
    {0x5F, 0x4C, 0x03, 0x8E, 0xD1, 0x2B, 0x2E, 0x41},
    {0x63, 0xFA, 0xC0, 0xD0, 0x34, 0xD9, 0xF7, 0x93},
    {0x61, 0x7B, 0x3A, 0x0C, 0xE8, 0xF0, 0x71, 0x00},
    {0xDB, 0x95, 0x86, 0x05, 0xF8, 0xC8, 0xC6, 0x06},
    {0xED, 0xBF, 0xD1, 0xC6, 0x6C, 0x29, 0xCC, 0xC7},
    {0x35, 0x55, 0x50, 0xB2, 0x15, 0x0E, 0x24, 0x51},
    {0xCA, 0xAA, 0xAF, 0x4D, 0xEA, 0xF1, 0xDB, 0xAE},
    {0xD5, 0xD4, 0x4F, 0xF7, 0x20, 0x68, 0x3D, 0x0D},
    {0x2A, 0x2B, 0xB0, 0x08, 0xDF, 0x97, 0xC2, 0xF2}
};

static unsigned char cipher_ecb2[NUM_TESTS - 1][8] = {
    {0x92, 0x95, 0xB5, 0x9B, 0xB3, 0x84, 0x73, 0x6E},
    {0x19, 0x9E, 0x9D, 0x6D, 0xF3, 0x9A, 0xA8, 0x16},
    {0x2A, 0x4B, 0x4D, 0x24, 0x52, 0x43, 0x84, 0x27},
    {0x35, 0x84, 0x3C, 0x01, 0x9D, 0x18, 0xC5, 0xB6},
    {0x4A, 0x5B, 0x2F, 0x42, 0xAA, 0x77, 0x19, 0x25},
    {0xA0, 0x6B, 0xA9, 0xB8, 0xCA, 0x5B, 0x17, 0x8A},
    {0xAB, 0x9D, 0xB7, 0xFB, 0xED, 0x95, 0xF2, 0x74},
    {0x3D, 0x25, 0x6C, 0x23, 0xA7, 0x25, 0x2F, 0xD6},
    {0xB7, 0x6F, 0xAB, 0x4F, 0xBD, 0xBD, 0xB7, 0x67},
    {0x8F, 0x68, 0x27, 0xD6, 0x9C, 0xF4, 0x1A, 0x10},
    {0x82, 0x57, 0xA1, 0xD6, 0x50, 0x5E, 0x81, 0x85},
    {0xA2, 0x0F, 0x0A, 0xCD, 0x80, 0x89, 0x7D, 0xFA},
    {0xCD, 0x2A, 0x53, 0x3A, 0xDB, 0x0D, 0x7E, 0xF3},
    {0xD2, 0xC2, 0xBE, 0x27, 0xE8, 0x1B, 0x68, 0xE3},
    {0xE9, 0x24, 0xCF, 0x4F, 0x89, 0x3C, 0x5B, 0x0A},
    {0xA7, 0x18, 0xC3, 0x9F, 0xFA, 0x9F, 0xD7, 0x69},
    {0x77, 0x2C, 0x79, 0xB1, 0xD2, 0x31, 0x7E, 0xB1},
    {0x49, 0xAB, 0x92, 0x7F, 0xD0, 0x22, 0x00, 0xB7},
    {0xCE, 0x1C, 0x6C, 0x7D, 0x85, 0xE3, 0x4A, 0x6F},
    {0xBE, 0x91, 0xD6, 0xE1, 0x27, 0xB2, 0xE9, 0x87},
    {0x70, 0x28, 0xAE, 0x8F, 0xD1, 0xF5, 0x74, 0x1A},
    {0xAA, 0x37, 0x80, 0xBB, 0xF3, 0x22, 0x1D, 0xDE},
    {0xA6, 0xC4, 0xD2, 0x5E, 0x28, 0x93, 0xAC, 0xB3},
    {0x22, 0x07, 0x81, 0x5A, 0xE4, 0xB7, 0x1A, 0xAD},
    {0xDC, 0xCE, 0x05, 0xE7, 0x07, 0xBD, 0xF5, 0x84},
    {0x26, 0x1D, 0x39, 0x2C, 0xB3, 0xBA, 0xA5, 0x85},
    {0xB4, 0xF7, 0x0F, 0x72, 0xFB, 0x04, 0xF0, 0xDC},
    {0x95, 0xBA, 0xA9, 0x4E, 0x87, 0x36, 0xF2, 0x89},
    {0xD4, 0x07, 0x3A, 0xF1, 0x5A, 0x17, 0x82, 0x0E},
    {0xEF, 0x6F, 0xAF, 0xA7, 0x66, 0x1A, 0x7E, 0x89},
    {0xC1, 0x97, 0xF5, 0x58, 0x74, 0x8A, 0x20, 0xE7},
    {0x43, 0x34, 0xCF, 0xDA, 0x22, 0xC4, 0x86, 0xC8},
    {0x08, 0xD7, 0xB4, 0xFB, 0x62, 0x9D, 0x08, 0x85}
};

static unsigned char cbc_key[8] = {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef
};
static unsigned char cbc2_key[8] = {
    0xf1, 0xe0, 0xd3, 0xc2, 0xb5, 0xa4, 0x97, 0x86
};
static unsigned char cbc3_key[8] = {
    0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10
};
static unsigned char cbc_iv[8] = {
    0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10
};
/*
 * Changed the following text constant to binary so it will work on ebcdic
 * machines :-)
 */
/* static char cbc_data[40]="7654321 Now is the time for \0001"; */
static unsigned char cbc_data[40] = {
    0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31, 0x20,
    0x4E, 0x6F, 0x77, 0x20, 0x69, 0x73, 0x20, 0x74,
    0x68, 0x65, 0x20, 0x74, 0x69, 0x6D, 0x65, 0x20,
    0x66, 0x6F, 0x72, 0x20, 0x00, 0x31, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static unsigned char cbc_ok[32] = {
    0xcc, 0xd1, 0x73, 0xff, 0xab, 0x20, 0x39, 0xf4,
    0xac, 0xd8, 0xae, 0xfd, 0xdf, 0xd8, 0xa1, 0xeb,
    0x46, 0x8e, 0x91, 0x15, 0x78, 0x88, 0xba, 0x68,
    0x1d, 0x26, 0x93, 0x97, 0xf7, 0xfe, 0x62, 0xb4
};

# ifdef SCREW_THE_PARITY
#  error "SCREW_THE_PARITY is not meant to be defined."
#  error "Original vectors are preserved for reference only."
static unsigned char cbc2_key[8] = {
    0xf0, 0xe1, 0xd2, 0xc3, 0xb4, 0xa5, 0x96, 0x87
};
static unsigned char xcbc_ok[32] = {
    0x86, 0x74, 0x81, 0x0D, 0x61, 0xA4, 0xA5, 0x48,
    0xB9, 0x93, 0x03, 0xE1, 0xB8, 0xBB, 0xBD, 0xBD,
    0x64, 0x30, 0x0B, 0xB9, 0x06, 0x65, 0x81, 0x76,
    0x04, 0x1D, 0x77, 0x62, 0x17, 0xCA, 0x2B, 0xD2,
};
# else
static unsigned char xcbc_ok[32] = {
    0x84, 0x6B, 0x29, 0x14, 0x85, 0x1E, 0x9A, 0x29,
    0x54, 0x73, 0x2F, 0x8A, 0xA0, 0xA6, 0x11, 0xC1,
    0x15, 0xCD, 0xC2, 0xD7, 0x95, 0x1B, 0x10, 0x53,
    0xA6, 0x3C, 0x5E, 0x03, 0xB2, 0x1A, 0xA3, 0xC4,
};
# endif

static unsigned char cbc3_ok[32] = {
    0x3F, 0xE3, 0x01, 0xC9, 0x62, 0xAC, 0x01, 0xD0,
    0x22, 0x13, 0x76, 0x3C, 0x1C, 0xBD, 0x4C, 0xDC,
    0x79, 0x96, 0x57, 0xC0, 0x64, 0xEC, 0xF5, 0xD4,
    0x1C, 0x67, 0x38, 0x12, 0xCF, 0xDE, 0x96, 0x75
};

static unsigned char pcbc_ok[32] = {
    0xcc, 0xd1, 0x73, 0xff, 0xab, 0x20, 0x39, 0xf4,
    0x6d, 0xec, 0xb4, 0x70, 0xa0, 0xe5, 0x6b, 0x15,
    0xae, 0xa6, 0xbf, 0x61, 0xed, 0x7d, 0x9c, 0x9f,
    0xf7, 0x17, 0x46, 0x3b, 0x8a, 0xb3, 0xcc, 0x88
};

static unsigned char cfb_key[8] = {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef
};
static unsigned char cfb_iv[8] = {
    0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef
};
static unsigned char cfb_buf1[40], cfb_buf2[40], cfb_tmp[8];
static unsigned char plain[24] = {
    0x4e, 0x6f, 0x77, 0x20, 0x69, 0x73,
    0x20, 0x74, 0x68, 0x65, 0x20, 0x74,
    0x69, 0x6d, 0x65, 0x20, 0x66, 0x6f,
    0x72, 0x20, 0x61, 0x6c, 0x6c, 0x20
};

static unsigned char cfb_cipher8[24] = {
    0xf3, 0x1f, 0xda, 0x07, 0x01, 0x14, 0x62, 0xee, 0x18, 0x7f, 0x43, 0xd8,
    0x0a, 0x7c, 0xd9, 0xb5, 0xb0, 0xd2, 0x90, 0xda, 0x6e, 0x5b, 0x9a, 0x87
};

static unsigned char cfb_cipher16[24] = {
    0xF3, 0x09, 0x87, 0x87, 0x7F, 0x57, 0xF7, 0x3C, 0x36, 0xB6, 0xDB, 0x70,
    0xD8, 0xD5, 0x34, 0x19, 0xD3, 0x86, 0xB2, 0x23, 0xB7, 0xB2, 0xAD, 0x1B
};

static unsigned char cfb_cipher32[24] = {
    0xF3, 0x09, 0x62, 0x49, 0xA4, 0xDF, 0xA4, 0x9F, 0x33, 0xDC, 0x7B, 0xAD,
    0x4C, 0xC8, 0x9F, 0x64, 0xE4, 0x53, 0xE5, 0xEC, 0x67, 0x20, 0xDA, 0xB6
};

static unsigned char cfb_cipher48[24] = {
    0xF3, 0x09, 0x62, 0x49, 0xC7, 0xF4, 0x30, 0xB5, 0x15, 0xEC, 0xBB, 0x85,
    0x97, 0x5A, 0x13, 0x8C, 0x68, 0x60, 0xE2, 0x38, 0x34, 0x3C, 0xDC, 0x1F
};

static unsigned char cfb_cipher64[24] = {
    0xF3, 0x09, 0x62, 0x49, 0xC7, 0xF4, 0x6E, 0x51, 0xA6, 0x9E, 0x83, 0x9B,
    0x1A, 0x92, 0xF7, 0x84, 0x03, 0x46, 0x71, 0x33, 0x89, 0x8E, 0xA6, 0x22
};

static unsigned char ofb_key[8] = {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef
};
static unsigned char ofb_iv[8] = {
    0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef
};
static unsigned char ofb_buf1[24], ofb_buf2[24], ofb_tmp[8];
static unsigned char ofb_cipher[24] = {
    0xf3, 0x09, 0x62, 0x49, 0xc7, 0xf4, 0x6e, 0x51,
    0x35, 0xf2, 0x4a, 0x24, 0x2e, 0xeb, 0x3d, 0x3f,
    0x3d, 0x6d, 0x5b, 0xe3, 0x25, 0x5a, 0xf8, 0xc3
};
static DES_LONG cbc_cksum_ret = 0xF7FE62B4L;
static unsigned char cbc_cksum_data[8] = {
    0x1D, 0x26, 0x93, 0x97, 0xf7, 0xfe, 0x62, 0xb4
};

static char *pt(const unsigned char *p, char buf[DATA_BUF_SIZE])
{
    char *ret;
    int i;
    static const char *f = "0123456789ABCDEF";

    ret = &(buf[0]);
    for (i = 0; i < 8; i++) {
        ret[i * 2] = f[(p[i] >> 4) & 0xf];
        ret[i * 2 + 1] = f[p[i] & 0xf];
    }
    ret[16] = '\0';
    return ret;
}

static int test_des_ecb(int i)
{
    DES_key_schedule ks;
    DES_cblock in, out, outin;
    char b1[DATA_BUF_SIZE], b2[DATA_BUF_SIZE];

    DES_set_key_unchecked(&key_data[i], &ks);
    memcpy(in, plain_data[i], 8);
    memset(out, 0, 8);
    memset(outin, 0, 8);
    DES_ecb_encrypt(&in, &out, &ks, DES_ENCRYPT);
    DES_ecb_encrypt(&out, &outin, &ks, DES_DECRYPT);

    if (!TEST_mem_eq(out, 8, cipher_data[i], 8)) {
        TEST_info("Encryption error %2d k=%s p=%s", i + 1,
                  pt(key_data[i], b1), pt(in, b2));
        return 0;
    }
    if (!TEST_mem_eq(in, 8, outin, 8)) {
        TEST_info("Decryption error %2d k=%s p=%s", i + 1,
                  pt(key_data[i], b1), pt(out, b2));
        return 0;
    }
    return 1;
}

static int test_des_ede_ecb(int i)
{
    DES_cblock in, out, outin;
    DES_key_schedule ks, ks2, ks3;
    char b1[DATA_BUF_SIZE], b2[DATA_BUF_SIZE];

    DES_set_key_unchecked(&key_data[i], &ks);
    DES_set_key_unchecked(&key_data[i + 1], &ks2);
    DES_set_key_unchecked(&key_data[i + 2], &ks3);
    memcpy(in, plain_data[i], 8);
    memset(out, 0, 8);
    memset(outin, 0, 8);
    DES_ecb3_encrypt(&in, &out, &ks, &ks2, &ks, DES_ENCRYPT);
    DES_ecb3_encrypt(&out, &outin, &ks, &ks2, &ks, DES_DECRYPT);

    if (!TEST_mem_eq(out, 8, cipher_ecb2[i], 8)) {
        TEST_info("Encryption error %2d k=%s p=%s", i + 1,
                  pt(key_data[i], b1), pt(in, b2));
        return 0;
    }
    if (!TEST_mem_eq(in, 8, outin, 8)) {
        TEST_info("Decryption error %2d k=%s p=%s ", i + 1,
                  pt(key_data[i], b1), pt(out, b2));
        return 0;
    }
    return 1;
}

static int test_des_cbc(void)
{
    unsigned char cbc_in[40];
    unsigned char cbc_out[40];
    DES_cblock iv3;
    DES_key_schedule ks;
    const size_t cbc_data_len = strlen((char *)cbc_data);

    if (!TEST_int_eq(DES_set_key_checked(&cbc_key, &ks), 0))
        return 0;
    memset(cbc_out, 0, sizeof(cbc_out));
    memset(cbc_in, 0, sizeof(cbc_in));
    memcpy(iv3, cbc_iv, sizeof(cbc_iv));
    DES_ncbc_encrypt(cbc_data, cbc_out, cbc_data_len + 1, &ks,
                     &iv3, DES_ENCRYPT);
    if (!TEST_mem_eq(cbc_out, 32, cbc_ok, 32))
        return 0;

    memcpy(iv3, cbc_iv, sizeof(cbc_iv));
    DES_ncbc_encrypt(cbc_out, cbc_in, cbc_data_len + 1, &ks,
                     &iv3, DES_DECRYPT);
    return TEST_mem_eq(cbc_in, cbc_data_len, cbc_data, cbc_data_len);
}

static int test_des_ede_cbc(void)
{
    DES_cblock iv3;
    DES_key_schedule ks;
    unsigned char cbc_in[40];
    unsigned char cbc_out[40];
    const size_t n = strlen((char *)cbc_data) + 1;

    if (!TEST_int_eq(DES_set_key_checked(&cbc_key, &ks), 0))
        return 0;
    memset(cbc_out, 0, sizeof(cbc_out));
    memset(cbc_in, 0, sizeof(cbc_in));
    memcpy(iv3, cbc_iv, sizeof(cbc_iv));
    DES_xcbc_encrypt(cbc_data, cbc_out, n, &ks, &iv3, &cbc2_key, &cbc3_key,
                     DES_ENCRYPT);
    if (!TEST_mem_eq(cbc_out, sizeof(xcbc_ok), xcbc_ok, sizeof(xcbc_ok)))
        return 0;
    memcpy(iv3, cbc_iv, sizeof(cbc_iv));
    DES_xcbc_encrypt(cbc_out, cbc_in, n, &ks, &iv3, &cbc2_key, &cbc3_key,
                     DES_DECRYPT);
    return TEST_mem_eq(cbc_data, n, cbc_data, n);
}

static int test_ede_cbc(void)
{
    DES_cblock iv3;
    DES_key_schedule ks, ks2, ks3;
    unsigned char cbc_in[40];
    unsigned char cbc_out[40];
    const size_t i = strlen((char *)cbc_data) + 1;
    const size_t n = (i + 7) / 8 * 8;

    if (!TEST_int_eq(DES_set_key_checked(&cbc_key, &ks), 0))
        return 0;
    if (!TEST_int_eq(DES_set_key_checked(&cbc2_key, &ks2), 0))
        return 0;
    if (!TEST_int_eq(DES_set_key_checked(&cbc3_key, &ks3), 0))
        return 0;
    memset(cbc_out, 0, sizeof(cbc_out));
    memset(cbc_in, 0, sizeof(cbc_in));
    memcpy(iv3, cbc_iv, sizeof(cbc_iv));

    DES_ede3_cbc_encrypt(cbc_data, cbc_out, 16L, &ks, &ks2, &ks3, &iv3,
                         DES_ENCRYPT);
    DES_ede3_cbc_encrypt(&cbc_data[16], &cbc_out[16], i - 16, &ks, &ks2,
                         &ks3, &iv3, DES_ENCRYPT);
    if (!TEST_mem_eq(cbc_out, n, cbc3_ok, n))
        return 0;

    memcpy(iv3, cbc_iv, sizeof(cbc_iv));
    DES_ede3_cbc_encrypt(cbc_out, cbc_in, i, &ks, &ks2, &ks3, &iv3,
                         DES_DECRYPT);
    return TEST_mem_eq(cbc_in, i, cbc_data, i);
}

static int test_input_align(int i)
{
    unsigned char cbc_out[40];
    DES_cblock iv;
    DES_key_schedule ks;
    const size_t n = strlen(i + (char *)cbc_data) + 1;

    memset(cbc_out, 0, sizeof(cbc_out));
    memcpy(iv, cbc_iv, sizeof(cbc_iv));
    if (!TEST_int_eq(DES_set_key_checked(&cbc_key, &ks), 0))
        return 0;
    DES_ncbc_encrypt(&cbc_data[i], cbc_out, n, &ks, &iv, DES_ENCRYPT);
    return 1;
}

static int test_output_align(int i)
{
    unsigned char cbc_out[40];
    DES_cblock iv;
    DES_key_schedule ks;
    const size_t n = strlen((char *)cbc_data) + 1;

    memset(cbc_out, 0, sizeof(cbc_out));
    memcpy(iv, cbc_iv, sizeof(cbc_iv));
    if (!TEST_int_eq(DES_set_key_checked(&cbc_key, &ks), 0))
        return 0;
    DES_ncbc_encrypt(cbc_data, &cbc_out[i], n, &ks, &iv, DES_ENCRYPT);
    return 1;
}

static int test_des_crypt(void)
{
    if (!TEST_str_eq("efGnQx2725bI2", DES_crypt("testing", "ef")))
        return 0;
    if (!TEST_str_eq("yA1Rp/1hZXIJk", DES_crypt("bca76;23", "yA")))
        return 0;

    if (!TEST_ptr_null(DES_crypt("testing", "y\202")))
        return 0;
    if (!TEST_ptr_null(DES_crypt("testing", "\0A")))
        return 0;
    if (!TEST_ptr_null(DES_crypt("testing", "A")))
        return 0;
    return 1;
}

static int test_des_pcbc(void)
{
    unsigned char cbc_in[40];
    unsigned char cbc_out[40];
    DES_key_schedule ks;
    const int n = strlen((char *)cbc_data) + 1;

    if (!TEST_int_eq(DES_set_key_checked(&cbc_key, &ks), 0))
        return 0;
    memset(cbc_out, 0, sizeof(cbc_out));
    memset(cbc_in, 0, sizeof(cbc_in));
    DES_pcbc_encrypt(cbc_data, cbc_out, n, &ks,
                     &cbc_iv, DES_ENCRYPT);
    if (!TEST_mem_eq(cbc_out, sizeof(pcbc_ok), pcbc_ok, sizeof(pcbc_ok)))
        return 0;
    DES_pcbc_encrypt(cbc_out, cbc_in, n, &ks,
                     &cbc_iv, DES_DECRYPT);
    return TEST_mem_eq(cbc_in, n, cbc_data, n);
}

static int cfb_test(int bits, unsigned char *cfb_cipher)
{
    DES_key_schedule ks;

    DES_set_key_checked(&cfb_key, &ks);
    memcpy(cfb_tmp, cfb_iv, sizeof(cfb_iv));
    DES_cfb_encrypt(plain, cfb_buf1, bits, sizeof(plain), &ks, &cfb_tmp,
                    DES_ENCRYPT);
    if (!TEST_mem_eq(cfb_cipher, sizeof(plain), cfb_buf1, sizeof(plain)))
        return 0;
    memcpy(cfb_tmp, cfb_iv, sizeof(cfb_iv));
    DES_cfb_encrypt(cfb_buf1, cfb_buf2, bits, sizeof(plain), &ks, &cfb_tmp,
                    DES_DECRYPT);
    return TEST_mem_eq(plain, sizeof(plain), cfb_buf2, sizeof(plain));
}

static int test_des_cfb8(void)
{
    return cfb_test(8, cfb_cipher8);
}

static int test_des_cfb16(void)
{
    return cfb_test(16, cfb_cipher16);
}

static int test_des_cfb32(void)
{
    return cfb_test(32, cfb_cipher32);
}

static int test_des_cfb48(void)
{
    return cfb_test(48, cfb_cipher48);
}

static int test_des_cfb64(void)
{
    DES_key_schedule ks;
    int n;
    size_t i;

    if (!cfb_test(64, cfb_cipher64))
        return 0;

    DES_set_key_checked(&cfb_key, &ks);
    memcpy(cfb_tmp, cfb_iv, sizeof(cfb_iv));
    n = 0;
    DES_cfb64_encrypt(plain, cfb_buf1, 12, &ks, &cfb_tmp, &n, DES_ENCRYPT);
    DES_cfb64_encrypt(&plain[12], &cfb_buf1[12], sizeof(plain) - 12, &ks,
                      &cfb_tmp, &n, DES_ENCRYPT);
    if (!TEST_mem_eq(cfb_cipher64, sizeof(plain), cfb_buf1, sizeof(plain)))
        return 0;
    memcpy(cfb_tmp, cfb_iv, sizeof(cfb_iv));
    n = 0;
    DES_cfb64_encrypt(cfb_buf1, cfb_buf2, 17, &ks, &cfb_tmp, &n, DES_DECRYPT);
    DES_cfb64_encrypt(&cfb_buf1[17], &cfb_buf2[17],
                      sizeof(plain) - 17, &ks, &cfb_tmp, &n, DES_DECRYPT);
    if (!TEST_mem_eq(plain, sizeof(plain), cfb_buf2, sizeof(plain)))
        return 0;

    memcpy(cfb_tmp, cfb_iv, sizeof(cfb_iv));
    for (i = 0; i < sizeof(plain); i++)
        DES_cfb_encrypt(&plain[i], &cfb_buf1[i], 8, 1, &ks, &cfb_tmp,
                        DES_ENCRYPT);
    if (!TEST_mem_eq(cfb_cipher8, sizeof(plain), cfb_buf1, sizeof(plain)))
        return 0;

    memcpy(cfb_tmp, cfb_iv, sizeof(cfb_iv));
    for (i = 0; i < sizeof(plain); i++)
        DES_cfb_encrypt(&cfb_buf1[i], &cfb_buf2[i], 8, 1, &ks, &cfb_tmp,
                        DES_DECRYPT);
    return TEST_mem_eq(plain, sizeof(plain), cfb_buf2, sizeof(plain));
}

static int test_des_ede_cfb64(void)
{
    DES_key_schedule ks;
    int n;

    DES_set_key_checked(&cfb_key, &ks);
    memcpy(cfb_tmp, cfb_iv, sizeof(cfb_iv));
    n = 0;
    DES_ede3_cfb64_encrypt(plain, cfb_buf1, 12, &ks, &ks, &ks, &cfb_tmp, &n,
                           DES_ENCRYPT);
    DES_ede3_cfb64_encrypt(&plain[12], &cfb_buf1[12], sizeof(plain) - 12, &ks,
                           &ks, &ks, &cfb_tmp, &n, DES_ENCRYPT);
    if (!TEST_mem_eq(cfb_cipher64, sizeof(plain), cfb_buf1, sizeof(plain)))
        return 0;
    memcpy(cfb_tmp, cfb_iv, sizeof(cfb_iv));
    n = 0;
    DES_ede3_cfb64_encrypt(cfb_buf1, cfb_buf2, (long)17, &ks, &ks, &ks,
                           &cfb_tmp, &n, DES_DECRYPT);
    DES_ede3_cfb64_encrypt(&cfb_buf1[17], &cfb_buf2[17], sizeof(plain) - 17,
                           &ks, &ks, &ks, &cfb_tmp, &n, DES_DECRYPT);
    return TEST_mem_eq(plain, sizeof(plain), cfb_buf2, sizeof(plain));
}

static int test_des_ofb(void)
{
    DES_key_schedule ks;

    DES_set_key_checked(&ofb_key, &ks);
    memcpy(ofb_tmp, ofb_iv, sizeof(ofb_iv));
    DES_ofb_encrypt(plain, ofb_buf1, 64, sizeof(plain) / 8, &ks, &ofb_tmp);
    if (!TEST_mem_eq(ofb_cipher, sizeof(ofb_buf1), ofb_buf1, sizeof(ofb_buf1)))
        return 0;

    memcpy(ofb_tmp, ofb_iv, sizeof(ofb_iv));
    DES_ofb_encrypt(ofb_buf1, ofb_buf2, 64, sizeof(ofb_buf1) / 8, &ks,
                    &ofb_tmp);
    return TEST_mem_eq(plain, sizeof(ofb_buf2), ofb_buf2, sizeof(ofb_buf2));
}

static int test_des_ofb64(void)
{
    DES_key_schedule ks;
    int num;
    size_t i;

    DES_set_key_checked(&ofb_key, &ks);
    memcpy(ofb_tmp, ofb_iv, sizeof(ofb_iv));
    memset(ofb_buf1, 0, sizeof(ofb_buf1));
    memset(ofb_buf2, 0, sizeof(ofb_buf1));
    num = 0;
    for (i = 0; i < sizeof(plain); i++) {
        DES_ofb64_encrypt(&plain[i], &ofb_buf1[i], 1, &ks, &ofb_tmp, &num);
    }
    if (!TEST_mem_eq(ofb_cipher, sizeof(ofb_buf1), ofb_buf1, sizeof(ofb_buf1)))
        return 0;
    memcpy(ofb_tmp, ofb_iv, sizeof(ofb_iv));
    num = 0;
    DES_ofb64_encrypt(ofb_buf1, ofb_buf2, sizeof(ofb_buf1), &ks, &ofb_tmp,
                      &num);
    return TEST_mem_eq(plain, sizeof(ofb_buf2), ofb_buf2, sizeof(ofb_buf2));
}

static int test_des_ede_ofb64(void)
{
    DES_key_schedule ks;
    int num;
    size_t i;

    DES_set_key_checked(&ofb_key, &ks);
    memcpy(ofb_tmp, ofb_iv, sizeof(ofb_iv));
    memset(ofb_buf1, 0, sizeof(ofb_buf1));
    memset(ofb_buf2, 0, sizeof(ofb_buf1));
    num = 0;
    for (i = 0; i < sizeof(plain); i++) {
        DES_ede3_ofb64_encrypt(&plain[i], &ofb_buf1[i], 1, &ks, &ks,
                               &ks, &ofb_tmp, &num);
    }
    if (!TEST_mem_eq(ofb_cipher, sizeof(ofb_buf1), ofb_buf1, sizeof(ofb_buf1)))
        return 0;
    memcpy(ofb_tmp, ofb_iv, sizeof(ofb_iv));
    num = 0;
    DES_ede3_ofb64_encrypt(ofb_buf1, ofb_buf2, sizeof(ofb_buf1), &ks, &ks, &ks,
                           &ofb_tmp, &num);
    return TEST_mem_eq(plain, sizeof(ofb_buf2), ofb_buf2, sizeof(ofb_buf2));
}

static int test_des_cbc_cksum(void)
{
    DES_LONG cs;
    DES_key_schedule ks;
    unsigned char cret[8];

    DES_set_key_checked(&cbc_key, &ks);
    cs = DES_cbc_cksum(cbc_data, &cret, strlen((char *)cbc_data), &ks,
                       &cbc_iv);
    if (!TEST_cs_eq(cs, cbc_cksum_ret))
        return 0;
    return TEST_mem_eq(cret, 8, cbc_cksum_data, 8);
}

static int test_des_quad_cksum(void)
{
    DES_LONG cs, lqret[4];

    cs = DES_quad_cksum(cbc_data, (DES_cblock *)lqret,
                        (long)strlen((char *)cbc_data), 2,
                        (DES_cblock *)cbc_iv);
    if (!TEST_cs_eq(cs, 0x70d7a63aL))
        return 0;
    if (!TEST_cs_eq(lqret[0], 0x327eba8dL))
        return 0;
    if (!TEST_cs_eq(lqret[1], 0x201a49ccL))
        return 0;
    if (!TEST_cs_eq(lqret[2], 0x70d7a63aL))
        return 0;
    if (!TEST_cs_eq(lqret[3], 0x501c2c26L))
        return 0;
    return 1;
}

/*
 * Test TDES based key wrapping.
 * The wrapping process uses a randomly generated IV so it is difficult to
 * undertake KATs.  End to end testing is performed instead.
 */
static const int test_des_key_wrap_sizes[] = {
    8, 16, 24, 32, 64, 80
};

static int test_des_key_wrap(int idx)
{
    int in_bytes = test_des_key_wrap_sizes[idx];
    unsigned char in[100], c_txt[200], p_txt[200], key[24];
    int clen, clen_upd, clen_fin, plen, plen_upd, plen_fin, expect, bs, i;
    EVP_CIPHER *cipher = NULL;
    EVP_CIPHER_CTX *ctx = NULL;
    int res = 0;

    /* Some sanity checks and cipher loading */
    if (!TEST_size_t_le(in_bytes, sizeof(in))
            || !TEST_ptr(cipher = EVP_CIPHER_fetch(NULL, "DES3-WRAP", NULL))
            || !TEST_int_eq(bs = EVP_CIPHER_get_block_size(cipher), 8)
            || !TEST_size_t_eq(bs * 3u, sizeof(key))
            || !TEST_true(in_bytes % bs == 0)
            || !TEST_ptr(ctx = EVP_CIPHER_CTX_new()))
        goto err;

    /* Create random data to end to end test */
    for (i = 0; i < in_bytes; i++)
        in[i] = test_random();

    /* Build the key */
    memcpy(key, cbc_key, sizeof(cbc_key));
    memcpy(key + sizeof(cbc_key), cbc2_key, sizeof(cbc2_key));
    memcpy(key + sizeof(cbc_key) + sizeof(cbc3_key), cbc_key, sizeof(cbc3_key));

    /* Wrap / encrypt the key */
    clen_upd = sizeof(c_txt);
    if (!TEST_true(EVP_EncryptInit(ctx, cipher, key, NULL))
            || !TEST_true(EVP_EncryptUpdate(ctx, c_txt, &clen_upd,
                                            in, in_bytes)))
        goto err;

    expect = (in_bytes + (bs - 1)) / bs * bs + 2 * bs;
    if (!TEST_int_eq(clen_upd, expect))
        goto err;

    clen_fin = sizeof(c_txt) - clen_upd;
    if (!TEST_true(EVP_EncryptFinal(ctx, c_txt + clen_upd, &clen_fin))
            || !TEST_int_eq(clen_fin, 0))
        goto err;
    clen = clen_upd + clen_fin;

    /* Decrypt the wrapped key */
    plen_upd = sizeof(p_txt);
    if (!TEST_true(EVP_DecryptInit(ctx, cipher, key, NULL))
            || !TEST_true(EVP_DecryptUpdate(ctx, p_txt, &plen_upd,
                                            c_txt, clen)))
        goto err;
    plen_fin = sizeof(p_txt) - plen_upd;
    if (!TEST_true(EVP_DecryptFinal(ctx, p_txt + plen_upd, &plen_fin)))
        goto err;
    plen = plen_upd + plen_fin;

    if (!TEST_mem_eq(in, in_bytes, p_txt, plen))
        goto err;
    res = 1;
 err:
    EVP_CIPHER_free(cipher);
    EVP_CIPHER_CTX_free(ctx);
    return res;
}

/*-
 * Weak and semi weak keys as taken from
 * %A D.W. Davies
 * %A W.L. Price
 * %T Security for Computer Networks
 * %I John Wiley & Sons
 * %D 1984
 */
static struct {
    const DES_cblock key;
    int expect;
} weak_keys[] = {
    /* weak keys */
    {{0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01}, 1 },
    {{0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE}, 1 },
    {{0x1F, 0x1F, 0x1F, 0x1F, 0x0E, 0x0E, 0x0E, 0x0E}, 1 },
    {{0xE0, 0xE0, 0xE0, 0xE0, 0xF1, 0xF1, 0xF1, 0xF1}, 1 },
    /* semi-weak keys */
    {{0x01, 0xFE, 0x01, 0xFE, 0x01, 0xFE, 0x01, 0xFE}, 1 },
    {{0xFE, 0x01, 0xFE, 0x01, 0xFE, 0x01, 0xFE, 0x01}, 1 },
    {{0x1F, 0xE0, 0x1F, 0xE0, 0x0E, 0xF1, 0x0E, 0xF1}, 1 },
    {{0xE0, 0x1F, 0xE0, 0x1F, 0xF1, 0x0E, 0xF1, 0x0E}, 1 },
    {{0x01, 0xE0, 0x01, 0xE0, 0x01, 0xF1, 0x01, 0xF1}, 1 },
    {{0xE0, 0x01, 0xE0, 0x01, 0xF1, 0x01, 0xF1, 0x01}, 1 },
    {{0x1F, 0xFE, 0x1F, 0xFE, 0x0E, 0xFE, 0x0E, 0xFE}, 1 },
    {{0xFE, 0x1F, 0xFE, 0x1F, 0xFE, 0x0E, 0xFE, 0x0E}, 1 },
    {{0x01, 0x1F, 0x01, 0x1F, 0x01, 0x0E, 0x01, 0x0E}, 1 },
    {{0x1F, 0x01, 0x1F, 0x01, 0x0E, 0x01, 0x0E, 0x01}, 1 },
    {{0xE0, 0xFE, 0xE0, 0xFE, 0xF1, 0xFE, 0xF1, 0xFE}, 1 },
    {{0xFE, 0xE0, 0xFE, 0xE0, 0xFE, 0xF1, 0xFE, 0xF1}, 1 },
    /* good key */
    {{0x49, 0xE9, 0x5D, 0x6D, 0x4C, 0xA2, 0x29, 0xBF}, 0 }
};

static int test_des_weak_keys(int n)
{
    const_DES_cblock *key = (unsigned char (*)[8])weak_keys[n].key;

    return TEST_int_eq(DES_is_weak_key(key), weak_keys[n].expect);
}

static struct {
    const DES_cblock key;
    int expect;
} bad_parity_keys[] = {
    {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0 },
    {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 0 },
    /* Perturb each byte in turn to create even parity */
    {{0x48, 0xE9, 0x5D, 0x6D, 0x4C, 0xA2, 0x29, 0xBF}, 0 },
    {{0x49, 0xE8, 0x5D, 0x6D, 0x4C, 0xA2, 0x29, 0xBF}, 0 },
    {{0x49, 0xE9, 0x5C, 0x6D, 0x4C, 0xA2, 0x29, 0xBF}, 0 },
    {{0x49, 0xE9, 0x5D, 0x7D, 0x4C, 0xA2, 0x29, 0xBF}, 0 },
    {{0x49, 0xE9, 0x5D, 0x6D, 0x5C, 0xA2, 0x29, 0xBF}, 0 },
    {{0x49, 0xE9, 0x5D, 0x6D, 0x4C, 0xA3, 0x29, 0xBF}, 0 },
    {{0x49, 0xE9, 0x5D, 0x6D, 0x4C, 0xA2, 0x39, 0xBF}, 0 },
    {{0x49, 0xE9, 0x5D, 0x6D, 0x4C, 0xA2, 0x29, 0xBE}, 0 },
    /* Odd parity version of above */
    {{0x49, 0xE9, 0x5D, 0x6D, 0x4C, 0xA2, 0x29, 0xBF}, 1 }
};

static int test_des_check_bad_parity(int n)
{
    const_DES_cblock *key = (unsigned char (*)[8])bad_parity_keys[n].key;

    return TEST_int_eq(DES_check_key_parity(key), bad_parity_keys[n].expect);
}

/* Test that two key 3DES can generate a random key without error */
static int test_des_two_key(void)
{
    int res = 0;
    EVP_CIPHER *cipher = NULL;
    EVP_CIPHER_CTX *ctx = NULL;
    unsigned char key[16];

    if (!TEST_ptr(cipher = EVP_CIPHER_fetch(NULL, "DES-EDE-ECB", NULL))
            || !TEST_ptr(ctx = EVP_CIPHER_CTX_new())
            || !EVP_CipherInit_ex(ctx, cipher, NULL, NULL, NULL, 1)
            || !EVP_CIPHER_CTX_set_key_length(ctx, sizeof(key))
            || !EVP_CIPHER_CTX_rand_key(ctx, key))
        goto err;

    res = 1;
 err:
    EVP_CIPHER_free(cipher);
    EVP_CIPHER_CTX_free(ctx);
    return res;
}

#endif

int setup_tests(void)
{
#ifndef OPENSSL_NO_DES
    ADD_ALL_TESTS(test_des_ecb, NUM_TESTS);
    ADD_TEST(test_des_cbc);
    ADD_TEST(test_ede_cbc);
    ADD_ALL_TESTS(test_des_ede_ecb, NUM_TESTS - 2);
    ADD_TEST(test_des_ede_cbc);
    ADD_TEST(test_des_pcbc);
    ADD_TEST(test_des_cfb8);
    ADD_TEST(test_des_cfb16);
    ADD_TEST(test_des_cfb32);
    ADD_TEST(test_des_cfb48);
    ADD_TEST(test_des_cfb64);
    ADD_TEST(test_des_ede_cfb64);
    ADD_TEST(test_des_ofb);
    ADD_TEST(test_des_ofb64);
    ADD_TEST(test_des_ede_ofb64);
    ADD_TEST(test_des_cbc_cksum);
    ADD_TEST(test_des_quad_cksum);
    ADD_TEST(test_des_crypt);
    ADD_ALL_TESTS(test_input_align, 4);
    ADD_ALL_TESTS(test_output_align, 4);
    ADD_ALL_TESTS(test_des_key_wrap, OSSL_NELEM(test_des_key_wrap_sizes));
    ADD_ALL_TESTS(test_des_weak_keys, OSSL_NELEM(weak_keys));
    ADD_ALL_TESTS(test_des_check_bad_parity, OSSL_NELEM(bad_parity_keys));
    ADD_TEST(test_des_two_key);
#endif
    return 1;
}
