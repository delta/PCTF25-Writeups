
#include "blake_header.h"

uint32_t *preimage_attack(uint32_t v0[], uint32_t v1_5[], uint32_t m8, uint32_t m10, uint32_t m11, uint32_t v10)
{
    uint32_t state0_5[16] = {0}, state1[16] = {0};
    static uint32_t pred_mess[16] = {0};

    state1[4] = ROTL(ROTL(v1_5[4], 7) ^ v1_5[8], 12) ^ (v1_5[8] - v1_5[12]);
    state1[5] = ROTL(ROTL(v1_5[5], 7) ^ v1_5[9], 12) ^ (v1_5[9] - v1_5[13]);
    state1[6] = ROTL(ROTL(v1_5[6], 7) ^ v1_5[10], 12) ^ (v1_5[10] - v1_5[14]);
    state1[7] = ROTL(ROTL(v1_5[7], 7) ^ v1_5[11], 12) ^ (v1_5[11] - v1_5[15]);

    state1[8] = v1_5[8] - v1_5[12] - (ROTL(v1_5[12], 8) ^ v1_5[0]);
    state1[9] = v1_5[9] - v1_5[13] - (ROTL(v1_5[13], 8) ^ v1_5[1]);
    state1[10] = v1_5[10] - v1_5[14] - (ROTL(v1_5[14], 8) ^ v1_5[2]);
    state1[11] = v1_5[11] - v1_5[15] - (ROTL(v1_5[15], 8) ^ v1_5[3]);

    state1[12] = ROTL((ROTL(v1_5[12], 8) ^ v1_5[0]), 16) ^ (v1_5[0] - (ROTL(v1_5[4], 7) ^ v1_5[8]) - m10);
    state1[13] = ROTL((ROTL(v1_5[13], 8) ^ v1_5[1]), 16) ^ (v1_5[1] - (ROTL(v1_5[5], 7) ^ v1_5[9]) - m8);

    state0_5[6] = ROTL(ROTL(state1[6], 7) ^ state1[11], 12) ^ (state1[11] - state1[12]);
    state0_5[7] = ROTL(ROTL(state1[7], 7) ^ state1[8], 12) ^ (state1[8] - state1[13]);

    pred_mess[4] = (ROTL(((ROTL((ROTL(state0_5[6], 7) ^ v10), 12) ^ v0[6]) - v0[10]), 16) ^ v0[14]) - v0[2] - v0[6];

    state1[1] = (ROTL(((ROTL((ROTL(v1_5[5], 7) ^ v1_5[9]), 12) ^ state1[5]) - state1[9]), 16) ^ state1[13]) - state1[5] - pred_mess[4];

    state0_5[14] = v10 - v0[10] - ROT((v0[14] ^ (v0[2] + v0[6] + pred_mess[4])), 16);

    state0_5[1] = state1[1] - (ROTL(state1[6], 7) ^ state1[11]) - m11 - state0_5[6] - m10;

    state0_5[11] = state1[11] - state1[12] - (ROTL(state1[12], 8) ^ state1[1]);

    state0_5[12] = ROTL(((ROTL((ROTL(state1[6], 7) ^ state1[11]), 12) ^ state0_5[6]) - state0_5[11]), 16) ^ (state0_5[1] + state0_5[6] + m10);

    state0_5[2] = (v10 - state0_5[14] - v0[10]) ^ ROTL(state0_5[14], 8);

    pred_mess[5] = (ROTL(((ROTL((ROTL(state0_5[6], 7) ^ v10), 12) ^ v0[6]) - v0[10]), 16) ^ v0[14]) + (ROTL(state0_5[6], 7) ^ v10) - state0_5[2];
    pred_mess[5] = -pred_mess[5];

    pred_mess[6] = (ROTL(((ROTL((ROTL(state0_5[7], 7) ^ state0_5[11]), 12) ^ v0[7]) - v0[11]), 16) ^ v0[15]) - v0[7] - v0[3];

    state1[15] = ROTL((ROTL(v1_5[15], 8) ^ v1_5[3]), 16) ^ (v1_5[3] - (ROTL(v1_5[7], 7) ^ v1_5[11]) - pred_mess[6]);

    state0_5[15] = state0_5[11] - v0[11] - ROT((v0[15] ^ (v0[3] + v0[7] + pred_mess[6])), 16);

    state0_5[5] = ROTL((ROTL(state1[5], 7) ^ state1[10]), 12) ^ (state1[10] - state1[15]);

    state1[0] = (state1[10] - state1[15] - v10) ^ ROTL(state1[15], 8);

    pred_mess[9] = (ROTL(((ROTL((ROTL(state1[5], 7) ^ state1[10]), 12) ^ state0_5[5]) - v10), 16) ^ state0_5[15]) + (ROTL(state1[5], 7) ^ state1[10]) - state1[0];
    pred_mess[9] = -pred_mess[9];

    pred_mess[14] = (ROTL(((ROTL((ROTL(v1_5[4], 7) ^ v1_5[8]), 12) ^ state1[4]) - state1[8]), 16) ^ state1[12]) - state1[0] - state1[4];

    state0_5[3] = (state0_5[11] - state0_5[15] - v0[11]) ^ ROTL(state0_5[15], 8);

    pred_mess[7] = (ROTL(((ROTL((ROTL(state0_5[7], 7) ^ state0_5[11]), 12) ^ v0[7]) - v0[11]), 16) ^ v0[15]) + (ROTL(state0_5[7], 7) ^ state0_5[11]) - state0_5[3];
    pred_mess[7] = -pred_mess[7];

    state0_5[0] = (ROTL(((ROTL((ROTL(state1[5], 7) ^ state1[10]), 12) ^ state0_5[5]) - v10), 16) ^ state0_5[15]) - m8 - state0_5[5];

    state0_5[8] = v0[8] + state0_5[12] + (ROTL(state0_5[12], 8) ^ state0_5[0]);

    pred_mess[0] = (ROTL((ROTL(state0_5[12], 8) ^ state0_5[0]), 16) ^ v0[12]) - v0[4] - v0[0];

    state1[2] = (state1[8] - state1[13] - state0_5[8]) ^ ROTL(state1[13], 8);

    state1[14] = (state1[2] + pred_mess[9] + state1[6]) ^ ROTL(((ROTL((ROTL(v1_5[6], 7) ^ v1_5[10]), 12) ^ state1[6]) - state1[10]), 16);

    pred_mess[15] = (ROTL(((ROTL((ROTL(v1_5[6], 7) ^ v1_5[10]), 12) ^ state1[6]) - state1[10]), 16) ^ state1[14]) + (ROTL(v1_5[6], 7) ^ v1_5[10]) - v1_5[2];
    pred_mess[15] = -pred_mess[15];

    state0_5[4] = ROT((ROT((v0[4] ^ (state0_5[8] - state0_5[12])), 12) ^ state0_5[8]), 7);

    pred_mess[1] = (ROTL(((ROTL((ROTL(state0_5[4], 7) ^ state0_5[8]), 12) ^ v0[4]) - v0[8]), 16) ^ v0[12]) + (ROTL(state0_5[4], 7) ^ state0_5[8]) - state0_5[0];
    pred_mess[1] = -pred_mess[1];

    state0_5[9] = state1[9] - state1[14] - ROT((state0_5[14] ^ (state0_5[3] + state0_5[4] + pred_mess[14])), 16);

    state1[3] = (ROTL(((ROTL((ROTL(state1[4], 7) ^ state1[9]), 12) ^ state0_5[4]) - state0_5[9]), 16) ^ state0_5[14]) + (ROTL(state1[4], 7) ^ state1[9]) + pred_mess[15];

    pred_mess[13] = (ROTL(((ROTL((ROTL(v1_5[7], 7) ^ v1_5[11]), 12) ^ state1[7]) - state1[11]), 16) ^ state1[15]) - state1[3] - state1[7];

    pred_mess[2] = (ROTL(((ROTL((ROTL(state0_5[5], 7) ^ state0_5[9]), 12) ^ v0[5]) - v0[9]), 16) ^ v0[13]) - v0[1] - v0[5];

    pred_mess[3] = (ROTL(((ROTL((ROTL(state0_5[5], 7) ^ state0_5[9]), 12) ^ v0[5]) - v0[9]), 16) ^ v0[13]) + (ROTL(state0_5[5], 7) ^ state0_5[9]) - state0_5[1];
    pred_mess[3] = -pred_mess[3];

    state0_5[13] = ROTL((ROTL(state1[13], 8) ^ state1[2]), 16) ^ (state1[2] - (ROTL(state1[7], 7) ^ state1[8]) - pred_mess[13]);

    pred_mess[12] = (ROTL(((ROTL((ROTL(state1[7], 7) ^ state1[8]), 12) ^ state0_5[7]) - state0_5[8]), 16) ^ state0_5[13]) - state0_5[2] - state0_5[7];

    return pred_mess;
}

int main()
{
    uint32_t m[16] = {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x445f7730, 0x00000000, 0x65683321, 0x7d800000, 0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t v0_5[16] = {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xbbcca2bf, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000};    // uint32_t v0_5[16] = {0xee2b2437, 0x9f33f7fe, 0xd6081eed, 0xdeef5d0d, 0xbc49041b, 0x985760e7, 0x0e0d2a75, 0x91f00a19, 0x73bb19b4, 0x56548bb3, 0x7c806b57, 0x25bb9672, 0xb37222a8, 0x81d1ec61, 0x31e18d4d, 0x737035b0 };

    // uint32_t v0[16] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19, 0x243f6a88, 0x85a308d3, 0x13198a2e, 0x03707344, 0xa409394a, 0x299f30b8, 0x082efa98, 0xec4e6c89};
    // uint32_t v1_5[16] = {0xbb3809b6, 0x3eb21951, 0x3473e1d7, 0x8eee1830, 0xab8148af, 0x51319e79, 0x5cd72840, 0xa67d0f4f, 0xab4bc747, 0xb116e7fe, 0x0efd1976, 0x1f2b9e39, 0xeb56f6bc, 0xc29b53ed, 0xdf355f55, 0x1ada623d};
    
    uint32_t v0[16] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19, 0x243f6a88, 0x85a308d3, 0x13198a2e, 0x03707344, 0xa409394a, 0x299f30b8, 0x082efa98, 0xec4e6c89};
    uint32_t v1_5[16] = {0xbb3809b6, 0x3eb21951, 0x3473e1d7, 0x8eee1830, 0xab8148af, 0x51319e79, 0x5cd72840, 0xa67d0f4f, 0xab4bc747, 0xb116e7fe, 0x0efd1976, 0x1f2b9e39, 0xeb56f6bc, 0xc29b53ed, 0xdf355f55, 0x1ada623d};

    uint32_t *pred_m;
    pred_m = preimage_attack(v0, v1_5, m[8], m[10], m[11], v0_5[10]);

    for (int i = 0; i < 16; i++)
    {
        if (*(pred_m + i)==0x00000000){
            *(pred_m + i) = m[i];
        }
        printf("(%d): %08x != %08x\n",i, m[i], *(pred_m + i));

    printf("m = [");
    for (int i = 0; i < 16; i++) {
        printf("0x%08x", m[i]);
        if (i < 15) {
            printf(", ");
            if ((i + 1) % 4 == 0) 
                printf("\n     ");
        }
    }
    printf("];\n");
    
    for (int i = 0; i < 16; i++)
        {
            printf("0x%08x\n", *(pred_m + i));
        }
     printf("m = [");
    for (int i = 0; i < 16; i++) {
        printf("0x%08x", pred_m[i]);
        if (i < 15) {
            printf(", ");
            if ((i + 1) % 4 == 0) 
                printf("\n     ");
        }
    }
    printf("];\n");
    return 0;
}
}