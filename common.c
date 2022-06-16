#include <stdlib.h>

#include "common.h"

#define random(x) (rand()%x)

int g_iDisplayFlag;

int StringToHex(char *str, unsigned char *out, unsigned int *outlen)
{
    char *p = str;
    char high = 0, low = 0;
    int tmplen = strlen(p), cnt = 0;
    tmplen = strlen(p);

    while (cnt < (tmplen / 2))
    {
        high =  ((*p > '9') && ((*p <= 'F') || (*p <= 'f'))) ? *p - 48 - 7 : *p - 48;
        low = (*(++p) > '9' && ((*p <= 'F') || (*p <= 'f'))) ? *(p) - 48 - 7 : *(p) - 48;
        out[cnt] = ((high & 0x0f) << 4 | (low & 0x0f));
        p++;
        cnt++;
    }

    if (tmplen % 2 != 0)
        out[cnt] = ((*p > '9') && ((*p <= 'F') || (*p <= 'f'))) ? *p - 48 - 7 : *p - 48;

    if (outlen != NULL)
        *outlen = tmplen / 2 + tmplen % 2;

    return tmplen / 2 + tmplen % 2;
}

static void hex_dump(const void *src, size_t length)
{
    int i = 0;
    const unsigned char *address = src;
    // const unsigned char *line = address;
    // unsigned char c;
    unsigned int num=0;
    size_t line_size=16;

    printf("%08x | ", num);
    num += 16;
    while (length-- > 0)
    {
        printf("%02x ", *address++);

        if ((i+1)%8==0 && (i+1)%16==8)
        {
            printf("  ");
        }

        if (!(++i % line_size) || (length == 0 && i % line_size))
        {
#if 0
            // if (length == 0) {
            //     while (i++ % line_size) {
            //         printf("   ");
            //     }
            // }

            printf(" | ");  /* right close */
            while (line < address) {
                c = *line++;
                printf("%c", (c < 33 || c == 255) ? 0x2E : c);
            }
#endif
            printf("\n");

            if (length > 0)
            {
                printf("%08x | ", num);
                num += 16;
            }
        }
    }
    printf("\n");
}

void tcmPrintf(const char *title, int iLength, const BYTE *a)
{
    if (g_iDisplayFlag == 0)
    {
        printf("%s\n", title);
        hex_dump(a, iLength);
    }
}

void Pack32(BYTE* dst, int val)
{
    dst[0] = (BYTE)((val >> 24) & 0xff);
    dst[1] = (BYTE)((val >> 16) & 0xff);
    dst[2] = (BYTE)((val >> 8) & 0xff);
    dst[3] = (BYTE)(val & 0xff);
}

uint32_t Unpack32(BYTE* src)
{
    return(((uint32_t) src[0]) << 24
         | ((uint32_t) src[1]) << 16
         | ((uint32_t) src[2]) << 8
         | (uint32_t) src[3]);
}

void Pack16(BYTE* dst, int val)
{
    dst[0] = (BYTE)((val >> 8) & 0xff);
    dst[1] = (BYTE)(val & 0xff);
}

int read_File_data(char *fileName, BYTE binData[MAX_BUFSIZE], uint32_t* length)
{
    FILE *infile;
    BYTE buf[MAX_BUFSIZE];

    infile = fopen(fileName, "rb");
    if (infile == NULL )
    {
        printf("%s not exist\n", fileName);
        return 1;
    }

    fseek(infile, 0, SEEK_END);
    *length = ftell(infile);
    fseek(infile, 0, SEEK_SET);

    fread(buf, *length, 1, infile);

    memcpy(binData, buf, *length);
    fclose(infile);

    tcmPrintf("binData", *length, binData);

    return 0;
}

int write_File_data(char* filename, BYTE binData[MAX_BUFSIZE], uint32_t data_len)
{
    FILE *fp;

    fp = fopen(filename, "w");
    fwrite(binData, data_len, 1, fp);
    fclose(fp);

    return 0;
}

/* 将uint16_t数据高位和低位互换 */
uint16_t reverse_bytes_uint16(uint16_t value)
{
    return (value & 0x00FFU) << 8 | (value & 0xFF00U) >> 8;
}

/* 将uint32_t数据高位和低位互换 */
uint32_t reverse_bytes_uint32(uint32_t value)
{
    return (value & 0x000000FFU) << 24 | (value & 0x0000FF00U) << 8 | 
           (value & 0x00FF0000U) >> 8 | (value & 0xFF000000U) >> 24; 
}
