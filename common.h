#ifndef _COMMON_H_
#define _COMMON_H_

#include <string.h>
#include <stdio.h>

#ifndef BYTE
typedef unsigned char  BYTE;
typedef unsigned char  uint8_t;
#endif

#ifndef UINT16
typedef unsigned short uint16_t;
#endif

#ifndef UINT32
typedef unsigned int   uint32_t;
#endif

#ifndef UINT64
typedef unsigned long  uint64_t;
#endif

#define BYTEn(x, n)   (*((unsigned char*)&(x)+n))

#define BYTE0(x)   BYTEn(x,  0)
#define BYTE1(x)   BYTEn(x,  1)
#define BYTE2(x)   BYTEn(x,  2)
#define BYTE3(x)   BYTEn(x,  3)

#define MAX_LINE    1024
#define MAX_BUFSIZE 4096

#ifndef DIGEST_LEN
#define DIGEST_LEN  32
#endif

uint32_t Unpack32(BYTE* src);
void Pack32(BYTE* dst, int val);
void Pack16(BYTE* dst, int val);

/*
* 判断运行环境是否为小端
*/
static const int endianTest = 1;
#define IsLittleEndian() (*(char *)&endianTest == 1)

uint16_t reverse_bytes_uint16(uint16_t value);
uint32_t reverse_bytes_uint32(uint32_t value);

int StringToHex(char *str, unsigned char *out, unsigned int *outlen);
int read_File_data(char* fileName, BYTE binData[MAX_BUFSIZE], uint32_t* length);
int write_File_data(char* filename, BYTE binData[MAX_BUFSIZE], uint32_t data_len);

void tcmPrintf(const char *title, int iLength, const BYTE *a);

#endif
