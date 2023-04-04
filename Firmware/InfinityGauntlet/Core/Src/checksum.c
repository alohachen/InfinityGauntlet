/*
 * checksum.c
 *
 *  Created on: Feb 16, 2021
 *      Author: chenyu
 */
#include "main.h"


static const unsigned short crcTable[256]={
0x0,0x8005,0x800f,0xa,0x801b,0x1e,0x14,0x8011,0x8033,0x36,0x3c,0x8039,0x28,0x802d,0x8027,0x22,0x8063,0x66,0x6c,0x8069,0x78,0x807d,0x8077,0x72,0x50,0x8055,0x805f,0x5a,0x804b,0x4e,0x44,0x8041,0x80c3,0xc6,0xcc,0x80c9,0xd8,0x80dd,0x80d7,0xd2,0xf0,0x80f5,0x80ff,0xfa,0x80eb,0xee,0xe4,0x80e1,0xa0,0x80a5,0x80af,0xaa,0x80bb,0xbe,0xb4,0x80b1,0x8093,0x96,0x9c,0x8099,0x88,0x808d,0x8087,0x82,0x8183,0x186,0x18c,0x8189,0x198,0x819d,0x8197,0x192,0x1b0,0x81b5,0x81bf,0x1ba,0x81ab,0x1ae,0x1a4,0x81a1,0x1e0,0x81e5,0x81ef,0x1ea,0x81fb,0x1fe,0x1f4,0x81f1,0x81d3,0x1d6,0x1dc,0x81d9,0x1c8,0x81cd,0x81c7,0x1c2,0x140,0x8145,0x814f,0x14a,0x815b,0x15e,0x154,0x8151,0x8173,0x176,0x17c,0x8179,0x168,0x816d,0x8167,0x162,0x8123,0x126,0x12c,0x8129,0x138,0x813d,0x8137,0x132,0x110,0x8115,0x811f,0x11a,0x810b,0x10e,0x104,0x8101,0x8303,0x306,0x30c,0x8309,0x318,0x831d,0x8317,0x312,0x330,0x8335,0x833f,0x33a,0x832b,0x32e,0x324,0x8321,0x360,0x8365,0x836f,0x36a,0x837b,0x37e,0x374,0x8371,0x8353,0x356,0x35c,0x8359,0x348,0x834d,0x8347,0x342,0x3c0,0x83c5,0x83cf,0x3ca,0x83db,0x3de,0x3d4,0x83d1,0x83f3,0x3f6,0x3fc,0x83f9,0x3e8,0x83ed,0x83e7,0x3e2,0x83a3,0x3a6,0x3ac,0x83a9,0x3b8,0x83bd,0x83b7,0x3b2,0x390,0x8395,0x839f,0x39a,0x838b,0x38e,0x384,0x8381,0x280,0x8285,0x828f,0x28a,0x829b,0x29e,0x294,0x8291,0x82b3,0x2b6,0x2bc,0x82b9,0x2a8,0x82ad,0x82a7,0x2a2,0x82e3,0x2e6,0x2ec,0x82e9,0x2f8,0x82fd,0x82f7,0x2f2,0x2d0,0x82d5,0x82df,0x2da,0x82cb,0x2ce,0x2c4,0x82c1,0x8243,0x246,0x24c,0x8249,0x258,0x825d,0x8257,0x252,0x270,0x8275,0x827f,0x27a,0x826b,0x26e,0x264,0x8261,0x220,0x8225,0x822f,0x22a,0x823b,0x23e,0x234,0x8231,0x8213,0x216,0x21c,0x8219,0x208,0x820d,0x8207,0x202
};


unsigned short gf_crc_rawdata(unsigned char *param_1, unsigned short param_2)
{
    unsigned short crc = 0xffff;
    unsigned int cnt = 0;
    unsigned char *buf;

    while(param_2--){
        if(cnt%2==0)
            buf=param_1+cnt+1;
        else
            buf=param_1+cnt-1;
        cnt++;
        crc = ((crc << 8) & 0xFF00) ^ crcTable[((crc >> 8) & 0xFF) ^ (*buf)];
    }
    return crc;
}

/*
unsigned short gf_crc_rawdata(unsigned char *param_1,unsigned short param_2)
{
  unsigned int uVar1;
  unsigned int uVar2;
  unsigned short uVar3;
  unsigned char bVar4;
  unsigned int uVar5;
  unsigned char *pbVar6;
  short sVar7;
  unsigned char *pbVar8;

  if (param_2 == 0) {
    return 0xffff;
  }
  uVar3 = param_2 & 1;
  if (param_2 == 1) {
    uVar5 = 0xffff;
  }
  else {
    uVar5 = 0xffff;
    pbVar6 = param_1;
    sVar7 = param_2 - (param_2 & 1);
    do {
      pbVar8 = pbVar6 + -1;
      if ((param_2 & 1) == 0) {
        pbVar8 = pbVar6 + 1;
      }
      param_2 = param_2 - 2;
      param_1 = pbVar6 + 2;
      uVar5 = uVar5 ^ (unsigned int)*pbVar8 << 8;
      uVar2 = (uVar5 & 0xffff) << 1;
      uVar1 = uVar2 ^ 0x8005;
      if (-1 < (short)uVar5) {
        uVar1 = uVar2;
      }
      uVar5 = uVar1 << 1 ^ 0x8005;
      if (-1 < (short)uVar1) {
        uVar5 = uVar1 << 1;
      }
      uVar1 = uVar5 << 1 ^ 0x8005;
      if (-1 < (short)uVar5) {
        uVar1 = uVar5 << 1;
      }
      uVar5 = uVar1 << 1 ^ 0x8005;
      if (-1 < (short)uVar1) {
        uVar5 = uVar1 << 1;
      }
      uVar1 = uVar5 << 1 ^ 0x8005;
      if (-1 < (short)uVar5) {
        uVar1 = uVar5 << 1;
      }
      uVar5 = uVar1 << 1 ^ 0x8005;
      if (-1 < (short)uVar1) {
        uVar5 = uVar1 << 1;
      }
      uVar1 = uVar5 << 1 ^ 0x8005;
      if (-1 < (short)uVar5) {
        uVar1 = uVar5 << 1;
      }
      uVar5 = uVar1 << 1 ^ 0x8005;
      if (-1 < (short)uVar1) {
        uVar5 = uVar1 << 1;
      }
      if ((param_2 & 1) != 0) {
        pbVar6 = param_1;
      }
      uVar5 = uVar5 ^ (unsigned int)*pbVar6 << 8;
      uVar2 = (uVar5 & 0xffff) << 1;
      uVar1 = uVar2 ^ 0x8005;
      if (-1 < (short)uVar5) {
        uVar1 = uVar2;
      }
      uVar5 = uVar1 << 1 ^ 0x8005;
      if (-1 < (short)uVar1) {
        uVar5 = uVar1 << 1;
      }
      uVar1 = uVar5 << 1 ^ 0x8005;
      if (-1 < (short)uVar5) {
        uVar1 = uVar5 << 1;
      }
      uVar5 = uVar1 << 1 ^ 0x8005;
      if (-1 < (short)uVar1) {
        uVar5 = uVar1 << 1;
      }
      uVar1 = uVar5 << 1 ^ 0x8005;
      if (-1 < (short)uVar5) {
        uVar1 = uVar5 << 1;
      }
      uVar5 = uVar1 << 1 ^ 0x8005;
      if (-1 < (short)uVar1) {
        uVar5 = uVar1 << 1;
      }
      uVar1 = uVar5 << 1 ^ 0x8005;
      if (-1 < (short)uVar5) {
        uVar1 = uVar5 << 1;
      }
      uVar5 = uVar1 << 1 ^ 0x8005;
      if (-1 < (short)uVar1) {
        uVar5 = uVar1 << 1;
      }
      bVar4 = sVar7 != 2;
      pbVar6 = param_1;
      sVar7 = sVar7 + -2;
    } while (bVar4);
  }
  if (uVar3 != 0) {
    pbVar6 = param_1 + -1;
    if ((param_2 & 1) == 0) {
      pbVar6 = param_1 + 1;
    }
    uVar5 = uVar5 ^ (unsigned int)*pbVar6 << 8;
    uVar2 = (uVar5 & 0xffff) << 1;
    uVar1 = uVar2 ^ 0x8005;
    if (-1 < (short)uVar5) {
      uVar1 = uVar2;
    }
    uVar5 = uVar1 << 1 ^ 0x8005;
    if (-1 < (short)uVar1) {
      uVar5 = uVar1 << 1;
    }
    uVar1 = uVar5 << 1 ^ 0x8005;
    if (-1 < (short)uVar5) {
      uVar1 = uVar5 << 1;
    }
    uVar5 = uVar1 << 1 ^ 0x8005;
    if (-1 < (short)uVar1) {
      uVar5 = uVar1 << 1;
    }
    uVar1 = uVar5 << 1 ^ 0x8005;
    if (-1 < (short)uVar5) {
      uVar1 = uVar5 << 1;
    }
    uVar5 = uVar1 << 1 ^ 0x8005;
    if (-1 < (short)uVar1) {
      uVar5 = uVar1 << 1;
    }
    uVar1 = uVar5 << 1 ^ 0x8005;
    if (-1 < (short)uVar5) {
      uVar1 = uVar5 << 1;
    }
    uVar5 = uVar1 << 1 ^ 0x8005;
    if (-1 < (short)uVar1) {
      uVar5 = uVar1 << 1;
    }
  }
  return (unsigned short)uVar5;
}
*/

static unsigned short vaddw_u8_result[8];

static __inline unsigned short * vaddw_u8_sim(unsigned short *a,unsigned char *b)
{
    vaddw_u8_result[0] = a[0] + b[0];
    vaddw_u8_result[1] = a[1] + b[1];
    vaddw_u8_result[2] = a[2] + b[2];
    vaddw_u8_result[3] = a[3] + b[3];
    vaddw_u8_result[4] = a[4] + b[4];
    vaddw_u8_result[5] = a[5] + b[5];
    vaddw_u8_result[6] = a[6] + b[6];
    vaddw_u8_result[7] = a[7] + b[7];

    return vaddw_u8_result;
}


static unsigned short vaddw_high_u8_result[8];

static __inline unsigned short * vaddw_high_u8_sim(unsigned short *a,unsigned char *b)
{
    vaddw_high_u8_result[0] = a[0] + b[8];
    vaddw_high_u8_result[1] = a[1] + b[9];
    vaddw_high_u8_result[2] = a[2] + b[10];
    vaddw_high_u8_result[3] = a[3] + b[11];
    vaddw_high_u8_result[4] = a[4] + b[12];
    vaddw_high_u8_result[5] = a[5] + b[13];
    vaddw_high_u8_result[6] = a[6] + b[14];
    vaddw_high_u8_result[7] = a[7] + b[15];

    return vaddw_high_u8_result;
}

static short vaddq_s16_result[8];

static __inline short * vaddq_s16_sim(short *a,short *b)
{
	*(unsigned long *)vaddq_s16_result = __SADD16(*(unsigned long *)a,*(unsigned long *)b);
	*(unsigned long *)(vaddq_s16_result+2) = __SADD16(*(unsigned long *)(a+2),*(unsigned long *)(b+2));
	*(unsigned long *)(vaddq_s16_result+4) = __SADD16(*(unsigned long *)(a+4),*(unsigned long *)(b+4));
	*(unsigned long *)(vaddq_s16_result+6) = __SADD16(*(unsigned long *)(a+6),*(unsigned long *)(b+6));
	/*
    vaddq_s16_result[0] = a[0] + b[0];
    vaddq_s16_result[1] = a[1] + b[1];
    vaddq_s16_result[2] = a[2] + b[2];
    vaddq_s16_result[3] = a[3] + b[3];
    vaddq_s16_result[4] = a[4] + b[4];
    vaddq_s16_result[5] = a[5] + b[5];
    vaddq_s16_result[6] = a[6] + b[6];
    vaddq_s16_result[7] = a[7] + b[7];
    */

    return vaddq_s16_result;
}


static short vadd_s16_result[4];

static __inline short * vadd_s16_sim(short *a,short *b)
{
	*(unsigned long *)vadd_s16_result = __SADD16(*(unsigned long *)a,*(unsigned long *)b);
	*(unsigned long *)(vadd_s16_result+2) = __SADD16(*(unsigned long *)(a+2),*(unsigned long *)(b+2));
	/*
    vadd_s16_result[0] = a[0] + b[0];
    vadd_s16_result[1] = a[1] + b[1];
    vadd_s16_result[2] = a[2] + b[2];
    vadd_s16_result[3] = a[3] + b[3];
	 */
    return vadd_s16_result;
}


static long vadd_s32_result[2];

static __inline long * vadd_s32_sim(long *a,long *b)
{
    vadd_s32_result[0] = a[0] + b[0];
    vadd_s32_result[1] = a[1] + b[1];

    return vadd_s32_result;
}



long long gf_checksum_rawdata(unsigned char *a1, int a2)
{
    signed long long v2;
    signed long long v3;
    unsigned short v4[8]={0,0,0,0,0,0,0,0};
    unsigned long long v5;
    unsigned long long v6;
    unsigned short v7[8] = {0,0,0,0,0,0,0,0};
    unsigned char *v8;
    int v10;
    unsigned int v11;
    int v12;
    unsigned char *v13;
    int v14;
    int v15;
    int v16;
    int v17;
    int v18;
    unsigned long long v19;
    unsigned char *v20;
    unsigned short v21;
    unsigned char v22;
    unsigned char v23;
    unsigned char *v24;
    unsigned char v25;
    unsigned char *v26;
    unsigned char v27;
    unsigned char v28;
    unsigned short v29;
    unsigned char v30;
    unsigned char v31;
    unsigned char v32;
    unsigned char *v33;
    unsigned char v34;
    signed char v35[16];
    unsigned long long v36;
    long v37[2];
    long v38[2];
    unsigned long long v39;
    int v40;
    char v41;
    unsigned char v42[16];
    unsigned char v43[16];
    unsigned char v44[16];
    unsigned char v45[16];
    unsigned char v46[16];
    unsigned char v47[16];
    unsigned char v48[16];
    unsigned char v49[16];
    unsigned char v50[16];

  if ( (short)a2 <= 0 )
  {
    v10 = a2 & 7;
    if ( (unsigned short)(a2 - 1) >= 7u )
    {
      v11 = 0;
      v12 = a2 - v10;
      v13 = a1;
      do
      {
        v14 = (unsigned short)v12;
        a1 = v13 + 8;
        v12 -= 8;
        v15 = v13[7];
        v16 = v13[6] + v13[5] + v13[4] + v13[3] + v13[2] + v13[1] + *v13 + v11;
        v13 += 8;
        v11 = v15 + v16;
      }
      while ( v14 != 8 );
    }
    else
    {
      v11 = 0;
    }
    if ( (a2 & 7) != 0 )
    {
      do
      {
        v17 = *a1++;
        v18 = (unsigned short)v10--;
        v11 += v17;
      }
      while ( v18 != 1 );
    }
    return v11;
  }
  v2 = (short)a2;
  if ( (short)a2 <= 15 )
  {
    v11 = 0;
    v3 = 0LL;
  }
  else
  {
    v3 = (short)a2 & 0xFFFFFFFFFFFFFFF0LL;
    //v4 = 0uLL;
    v5 = ((unsigned long long)(v3 - 16) >> 4) + 1;
    v6 = v5 & 7;
    if ( (unsigned long long)(v3 - 16) >= 0x70 )
    {
      //v7 = 0uLL;
      v19 = v5 - v6;
      v8 = a1;
      do
      {
        v19 -= 8LL;
        *((unsigned long long *)(&v43)) = (unsigned short)(*v8 | (v8[1] << 8)) | ((unsigned short)(v8[2] | (v8[3] << 8)) << 16) | ((unsigned long long)((unsigned short)(v8[4] | (v8[5] << 8)) | ((unsigned short)(v8[6] | (v8[7] << 8)) << 16)) << 32);
        *((unsigned long long *)(&v43)+1) = (unsigned short)(v8[8] | (v8[9] << 8)) | ((unsigned short)(v8[10] | (v8[11] << 8)) << 16) | ((unsigned long long)((unsigned short)(v8[12] | (v8[13] << 8)) | ((unsigned short)(v8[14] | (v8[15] << 8)) << 16)) << 32);
        *((unsigned long long *)(&v50)) = (unsigned short)(v8[16] | (v8[17] << 8)) | ((unsigned short)(v8[18] | (v8[19] << 8)) << 16) | ((unsigned long long)((unsigned short)(v8[20] | (v8[21] << 8)) | ((unsigned short)(v8[22] | (v8[23] << 8)) << 16)) << 32);
        *((unsigned long long *)(&v50)+1) = (unsigned short)(v8[24] | (v8[25] << 8)) | ((unsigned short)(v8[26] | (v8[27] << 8)) << 16) | ((unsigned long long)((unsigned short)(v8[28] | (v8[29] << 8)) | ((unsigned short)(v8[30] | (v8[31] << 8)) << 16)) << 32);
        *((unsigned long long *)(&v49)) = (unsigned short)(v8[32] | (v8[33] << 8)) | ((unsigned short)(v8[34] | (v8[35] << 8)) << 16) | ((unsigned long long)((unsigned short)(v8[36] | (v8[37] << 8)) | ((unsigned short)(v8[38] | (v8[39] << 8)) << 16)) << 32);
        *((unsigned long long *)(&v49)+1)  = (unsigned short)(v8[40] | (v8[41] << 8)) | ((unsigned short)(v8[42] | (v8[43] << 8)) << 16) | ((unsigned long long)((unsigned short)(v8[44] | (v8[45] << 8)) | ((unsigned short)(v8[46] | (v8[47] << 8)) << 16)) << 32);
        *((unsigned long long *)(&v48)) = (unsigned short)(v8[48] | (v8[49] << 8)) | ((unsigned short)(v8[50] | (v8[51] << 8)) << 16) | ((unsigned long long)((unsigned short)(v8[52] | (v8[53] << 8)) | ((unsigned short)(v8[54] | (v8[55] << 8)) << 16)) << 32);
        *((unsigned long long *)(&v48)+1) = (unsigned short)(v8[56] | (v8[57] << 8)) | ((unsigned short)(v8[58] | (v8[59] << 8)) << 16) | ((unsigned long long)((unsigned short)(v8[60] | (v8[61] << 8)) | ((unsigned short)(v8[62] | (v8[63] << 8)) << 16)) << 32);
        *((unsigned long long *)(&v47)) = (unsigned short)(v8[64] | (v8[65] << 8)) | ((unsigned short)(v8[66] | (v8[67] << 8)) << 16) | ((unsigned long long)((unsigned short)(v8[68] | (v8[69] << 8)) | ((unsigned short)(v8[70] | (v8[71] << 8)) << 16)) << 32);
        *((unsigned long long *)(&v47)+1) = (unsigned short)(v8[72] | (v8[73] << 8)) | ((unsigned short)(v8[74] | (v8[75] << 8)) << 16) | ((unsigned long long)((unsigned short)(v8[76] | (v8[77] << 8)) | ((unsigned short)(v8[78] | (v8[79] << 8)) << 16)) << 32);
        *((unsigned long long *)(&v46)) = (unsigned short)(v8[80] | (v8[81] << 8)) | ((unsigned short)(v8[82] | (v8[83] << 8)) << 16) | ((unsigned long long)((unsigned short)(v8[84] | (v8[85] << 8)) | ((unsigned short)(v8[86] | (v8[87] << 8)) << 16)) << 32);
        *((unsigned long long *)(&v46)+1) = (unsigned short)(v8[88] | (v8[89] << 8)) | ((unsigned short)(v8[90] | (v8[91] << 8)) << 16) | ((unsigned long long)((unsigned short)(v8[92] | (v8[93] << 8)) | ((unsigned short)(v8[94] | (v8[95] << 8)) << 16)) << 32);
        *((unsigned long long *)(&v45)) = (unsigned short)(v8[96] | (v8[97] << 8)) | ((unsigned short)(v8[98] | (v8[99] << 8)) << 16) | ((unsigned long long)((unsigned short)(v8[100] | (v8[101] << 8)) | ((unsigned short)(v8[102] | (v8[103] << 8)) << 16)) << 32);
        *((unsigned long long *)(&v45)+1) = (unsigned short)(v8[104] | (v8[105] << 8)) | ((unsigned short)(v8[106] | (v8[107] << 8)) << 16) | ((unsigned long long)((unsigned short)(v8[108] | (v8[109] << 8)) | ((unsigned short)(v8[110] | (v8[111] << 8)) << 16)) << 32);
        v20 = v8 + 112;
        v21 = v8[112] | (v8[113] << 8);
        v22 = v8[115];
        v23 = v8[114];
        v24 = v8 + 112;
        v26 = v8 + 116;
        v25 = v8[116];
        v27 = v8[117];
        v8 += 128;
        *((unsigned long long *)(&v44)) = v21 | ((unsigned short)(v23 | (v22 << 8)) << 16) | ((unsigned long long)((unsigned short)(v25 | (v27 << 8)) | ((unsigned short)(v26[2] | (v26[3] << 8)) << 16)) << 32);
        v28 = v24[8];
        v24 += 8;
        *((unsigned long long *)(&v44)+1) = (unsigned short)(v28 | (v20[9] << 8)) | ((unsigned short)(v24[2] | (v24[3] << 8)) << 16) | ((unsigned long long)((unsigned short)(v24[4] | (v20[13] << 8)) | ((unsigned short)(v24[6] | (v24[7] << 8)) << 16)) << 32);


          unsigned short *pv7 = (unsigned short *)vaddw_u8_sim(
                 vaddw_u8_sim(
                   vaddw_u8_sim(
                     vaddw_u8_sim(
                       vaddw_u8_sim(
                         vaddw_u8_sim(
                                  vaddw_u8_sim(vaddw_u8_sim((unsigned short *)v7, (unsigned char *)&v43), (unsigned char *)&v50),
                                      (unsigned char *)&v49),
                                    (unsigned char *)&v48),
                                  (unsigned char *)&v47),
                                (unsigned char *)&v46),
                              (unsigned char *)&v45),
                                       (unsigned char *)&v44);
          *(unsigned long *)v7 = *(unsigned long *)pv7;
          *(unsigned long *)(v7+2) = *(unsigned long *)(pv7+2);
          *(unsigned long *)(v7+4) = *(unsigned long *)(pv7+4);
          *(unsigned long *)(v7+6) = *(unsigned long *)(pv7+6);
          /*
          v7[0] = pv7[0];
          v7[1] = pv7[1];
          v7[2] = pv7[2];
          v7[3] = pv7[3];
          v7[4] = pv7[4];
          v7[5] = pv7[5];
          v7[6] = pv7[6];
          v7[7] = pv7[7];
          */
        unsigned short * pv4 = (unsigned short *)vaddw_high_u8_sim(
                 vaddw_high_u8_sim(
                   vaddw_high_u8_sim(
                     vaddw_high_u8_sim(
                       vaddw_high_u8_sim(vaddw_high_u8_sim(vaddw_high_u8_sim(vaddw_high_u8_sim((unsigned short *)v4, (unsigned char *)&v43), (unsigned char *)&v50), (unsigned char *)&v49), (unsigned char *)&v48),
                                       (unsigned char *)&v47),
                                     (unsigned char *)&v46),
                                   (unsigned char *)&v45),
                                                               (unsigned char *)&v44);
        *(unsigned long *)v4 = *(unsigned long *)pv4;
        *(unsigned long *)(v4+2) = *(unsigned long *)(pv4+2);
        *(unsigned long *)(v4+4) = *(unsigned long *)(pv4+4);
        *(unsigned long *)(v4+6) = *(unsigned long *)(pv4+6);
        /*
          v4[0] = pv4[0];
          v4[1] = pv4[1];
          v4[2] = pv4[2];
          v4[3] = pv4[3];
          v4[4] = pv4[4];
          v4[5] = pv4[5];
          v4[6] = pv4[6];
          v4[7] = pv4[7];
          */
      }
      while ( v19 );
    }
    else
    {
      //v7 = 0uLL;
        v7[0] = 0;
        v7[1] = 0;
        v7[2] = 0;
        v7[3] = 0;
        v7[4] = 0;
        v7[5] = 0;
        v7[6] = 0;
        v7[7] = 0;
      v8 = a1;
    }
    if ( (v5 & 7) != 0 )
    {
      do
      {
        --v6;

        *((unsigned long long *)(&v42)) = (unsigned short)(*v8 | (v8[1] << 8)) | ((unsigned short)(v8[2] | (v8[3] << 8)) << 16) | ((unsigned long long)((unsigned short)(v8[4] | (v8[5] << 8)) | ((unsigned short)(v8[6] | (v8[7] << 8)) << 16)) << 32);

        v29 = v8[8] | (v8[9] << 8);
        v30 = v8[11];
        v31 = v8[10];
        v33 = v8 + 12;
        v32 = v8[12];
        v34 = v8[13];
        v8 += 16;

        *((unsigned long long *)(&v42)+1) = v29 | ((unsigned short)(v31 | (v30 << 8)) << 16) | ((unsigned long long)((unsigned short)(v32 | (v34 << 8)) | ((unsigned short)(v33[2] | (v33[3] << 8)) << 16)) << 32);
          unsigned short * pv7 = (unsigned short *)vaddw_u8_sim((unsigned short *)v7, (unsigned char *)&v42);
          *(unsigned long *)v7 = *(unsigned long *)pv7;
          *(unsigned long *)(v7+2) = *(unsigned long *)(pv7+2);
          *(unsigned long *)(v7+4) = *(unsigned long *)(pv7+4);
          *(unsigned long *)(v7+6) = *(unsigned long *)(pv7+6);
          /*
          v7[0] = pv7[0];
          v7[1] = pv7[1];
          v7[2] = pv7[2];
          v7[3] = pv7[3];
          v7[4] = pv7[4];
          v7[5] = pv7[5];
          v7[6] = pv7[6];
          v7[7] = pv7[7];
          */
          unsigned short * pv4 = (unsigned short *)vaddw_high_u8_sim((unsigned short *)v4, (unsigned char *)&v42);
          *(unsigned long *)v4 = *(unsigned long *)pv4;
          *(unsigned long *)(v4+2) = *(unsigned long *)(pv4+2);
          *(unsigned long *)(v4+4) = *(unsigned long *)(pv4+4);
          *(unsigned long *)(v4+6) = *(unsigned long *)(pv4+6);
          /*
          v4[0] = pv4[0];
          v4[1] = pv4[1];
          v4[2] = pv4[2];
          v4[3] = pv4[3];
          v4[4] = pv4[4];
          v4[5] = pv4[5];
          v4[6] = pv4[6];
          v4[7] = pv4[7];
          */
      }
      while ( v6 );
    }
      signed char * pv35 = (signed char *)vaddq_s16_sim((short *)v7, (short *)v4);
      *(unsigned long *)v35 = *(unsigned long *)pv35;
      *(unsigned long *)(v35+4) = *(unsigned long *)(pv35+4);
      *(unsigned long *)(v35+8) = *(unsigned long *)(pv35+8);
      *(unsigned long *)(v35+12) = *(unsigned long *)(pv35+12);
      /*
      v35[0] = pv35[0];
      v35[1] = pv35[1];
      v35[2] = pv35[2];
      v35[3] = pv35[3];

      v35[4] = pv35[4];
      v35[5] = pv35[5];
      v35[6] = pv35[6];
      v35[7] = pv35[7];

      v35[8] = pv35[8];
      v35[9] = pv35[9];
      v35[10] = pv35[10];
      v35[11] = pv35[11];

      v35[12] = pv35[12];
      v35[13] = pv35[13];
      v35[14] = pv35[14];
      v35[15] = pv35[15];
      */
      short tmp1[4];
      short * ptmp1 = (short *)&v35;
      *(unsigned long *)tmp1 = *(unsigned long *)ptmp1;
      *(unsigned long *)(tmp1+2) = *(unsigned long *)(ptmp1+2);
      /*
      tmp1[0] = ptmp1[0];
      tmp1[1] = ptmp1[1];
      tmp1[2] = ptmp1[2];
      tmp1[3] = ptmp1[3];
      */
      signed char tmp4[16];
      *(unsigned long *)tmp4 = *(unsigned long *)(v35+8);
      *(unsigned long *)(tmp4+4) = *(unsigned long *)(v35+12);
      *(unsigned long *)(tmp4+8) = *(unsigned long *)v35;
      *(unsigned long *)(tmp4+12) = *(unsigned long *)(v35+4);
      /*
      tmp4[0] = v35[8];
      tmp4[1] = v35[9];
      tmp4[2] = v35[10];
      tmp4[3] = v35[11];

      tmp4[4] = v35[12];
      tmp4[5] = v35[13];
      tmp4[6] = v35[14];
      tmp4[7] = v35[15];

      tmp4[8] = v35[0];
      tmp4[9] = v35[1];
      tmp4[10] = v35[2];
      tmp4[11] = v35[3];

      tmp4[12] = v35[4];
      tmp4[13] = v35[5];
      tmp4[14] = v35[6];
      tmp4[15] = v35[7];
      */
      short tmp2[4];
      short * ptmp2 = (short *)&tmp4;
      *(unsigned long *)tmp2 = *(unsigned long *)ptmp2;
      *(unsigned long *)(tmp2+2) = *(unsigned long *)(ptmp2+2);
      /*
      tmp2[0] = ptmp2[0];
      tmp2[1] = ptmp2[1];
      tmp2[2] = ptmp2[2];
      tmp2[3] = ptmp2[3];
      */
      short tmp3[4];
      short * ptmp3 = (short *)vadd_s16_sim((short *)&tmp1,(short *)&tmp2);
      *(unsigned long *)tmp3 = *(unsigned long *)ptmp3;
      *(unsigned long *)(tmp3+2) = *(unsigned long *)(ptmp3+2);
      /*
      tmp3[0] = ptmp3[0];
      tmp3[1] = ptmp3[1];
      tmp3[2] = ptmp3[2];
      tmp3[3] = ptmp3[3];
      */
    v36 = *(unsigned long long *)&tmp3;
    *(unsigned int *)&v37 = (unsigned short)v36;
    *(unsigned int *)&v38 = *((unsigned short *)&v36 + 2);
      *((unsigned int *)(&v37)+1) = *((unsigned short *)&v36 + 1);
      *((unsigned int *)(&v38)+1) = *((unsigned short *)&v36 + 3);
      long tmp5[2];
      long * ptmp5 = (long *)vadd_s32_sim((long *)&v37, (long *)&v38);
      tmp5[0] = ptmp5[0];
      tmp5[1] = ptmp5[1];
      v39 = *(unsigned long long *)&tmp5;
      v11 = v39 + *((unsigned int *)&v39 + 1);
    if ( v3 >= v2 )
      return v11;
  }
  do
  {
    v40 = a1[v3];
    v41 = v3++ < v2 - 1;
    v11 += v40;
  }
  while ( v41 );
  return v11;
}








