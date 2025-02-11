// required header files
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define toRead 64
//isnt it toRead 32 , cuz the above line is used to define size for reading input data , set to 64 bytes which is 512 bytes

// state context for BLAKE-256
typedef struct
{
    // chain variables, salt, counter
    uint32_t h[8], s[4], t[2]; //chain variables(initial hash values), salt , counter to keep track of bits processed
    int buflen, nullt; //buffer lenght and null counter 
    uint8_t buf[64]; //buffer for input data
} state256;

// 8-bit to 32-bit conversion - big-endian
//converts 4 8 bit values to a single 32 bit integer in big endian format
#define U8TO32_BIG(p)                                          \
    (((uint32_t)((p)[0]) << 24) | ((uint32_t)((p)[1]) << 16) | \
     ((uint32_t)((p)[2]) << 8) | ((uint32_t)((p)[3])))

// 32-bit to 8-bit conversion - big-endian
//does the reverse of the above function 
#define U32TO8_BIG(p, v)           \
    (p)[0] = (uint8_t)((v) >> 24); \
    (p)[1] = (uint8_t)((v) >> 16); \
    (p)[2] = (uint8_t)((v) >> 8);  \
    (p)[3] = (uint8_t)((v));

// right circular shift
//performs right circular shift of 32 bit integer x by n bits
#define ROT(x, n) (((x) << (32 - n)) | ((x) >> (n)))
// left circular shift
//performs a left circular shift of 32 bit ineteger x by n bits.
#define ROTL(x, n) (((x) >> (32 - n)) | ((x) << (n)))

// permutation table
const uint8_t sigma[][16] =
    {
        {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
        {14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3},
        {11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4},
        {7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8},
        {9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13},
        {2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9},
        {12, 5, 1, 15, 14, 13, 4, 10, 0, 7, 6, 3, 9, 2, 8, 11},
        {13, 11, 7, 14, 12, 1, 3, 9, 5, 0, 15, 4, 8, 6, 2, 10},
        {6, 15, 14, 9, 11, 3, 0, 8, 12, 2, 13, 7, 1, 4, 10, 5},
        {10, 2, 8, 4, 7, 6, 1, 5, 15, 11, 9, 14, 3, 12, 13, 0},
        {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
        {14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3},
        {11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4},
        {7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8},
        {9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13},
        {2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9}};

// constants
//Contains 16 constant values derived from the fractional parts of square roots of prime numbers, similar to how constants are chosen in SHA-256.
const uint32_t constant[16] =
    {
        0x243f6a88, 0x85a308d3, 0x13198a2e, 0x03707344,
        0xa4093822, 0x299f31d0, 0x082efa98, 0xec4e6c89,
        0x452821e6, 0x38d01377, 0xbe5466cf, 0x34e90c6c,
        0xc0ac29b7, 0xc97c50dd, 0x3f84d5b5, 0xb5470917};

// padding
static const uint8_t padding[129] =
    {
        0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// original core function for BLAKE-256
//takes a working vector v , message block m , round index r , postions a,b,c,d,e as inputs 
void G(uint32_t v[], uint32_t m[], uint32_t r, int a, int b, int c, int d, int e)
{
    v[a] += (m[sigma[r][e]] ^ constant[sigma[r][e + 1]]) + v[b];
    v[d] = ROT(v[d] ^ v[a], 16);
    v[c] += v[d];
    v[b] = ROT(v[b] ^ v[c], 12);
    v[a] += (m[sigma[r][e + 1]] ^ constant[sigma[r][e]]) + v[b];
    v[d] = ROT(v[d] ^ v[a], 8);
    v[c] += v[d];
    v[b] = ROT(v[b] ^ v[c], 7);
}

// modified core function - without the use of constants
void G_mod(uint32_t v[], uint32_t m[], uint32_t r, int a, int b, int c, int d, int e)
{
    v[a] += m[sigma[r][e]] + v[b];
    v[d] = ROT(v[d] ^ v[a], 16);
    v[c] += v[d];
    v[b] = ROT(v[b] ^ v[c], 12);
    v[a] += m[sigma[r][e + 1]] + v[b];
    v[d] = ROT(v[d] ^ v[a], 8);
    v[c] += v[d];
    v[b] = ROT(v[b] ^ v[c], 7);
}