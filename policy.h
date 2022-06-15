#ifndef __POLICY_H__
#define __POLICY_H__

#include "common.h"

/* 对一个度量区域定义的TCM策略结构 */
typedef struct {
    BYTE start_addr[4];      //度量区域的起始地址
    BYTE length[4];          //度量区域的长度
    BYTE hash_len[2];        //sm3哈希长度，固定为32，即0x0020
    BYTE sm3_hash[32];       //哈希值
    BYTE reserved1[2];       //保留字段，固定为0xffff
    BYTE hashAlg[2];         //计算hash用的算法，目前仅支持sm3，定义为0x0012
    BYTE policy;             //策略，goon为0x01，hang为0x00
    BYTE reserved2;          //保留字段，固定为0xffff
} __attribute__((__packed__)) policy_tcm_t;

/* TCM可信策略文件的结构 */
typedef struct {
    BYTE flashrom_valid[4];    //spi flash rom标记，最多支持4个度量spi flash
                               //度量flash rom1时，则flashrom_flag[0]=0x01
    BYTE area_valid1[4];       //度量的flash rom中的区域标记，最多支持4个度量区域
                               //度量区域为1时，policy[0]中策略生效，后面类推
    policy_tcm_t policy1[4];  //flash rom1中支持的4个度量区域相应的策略信息
    
    BYTE area_valid2[4];
    policy_tcm_t policy2[4];
} __attribute__((__packed__)) policy_tcm_file_t;


/* 对一个度量区域定义的TPM策略结构 */
typedef struct {
    BYTE start_addr[4];      //度量区域的起始地址
    BYTE length[4];          //度量区域的长度
    BYTE hash_len[2];        //sm3哈希长度，固定为32，即0x0020
    BYTE sm3_hash[32];       //哈希值
    BYTE reserved1[16];      //保留字段
    BYTE hashAlg[2];         //计算hash用的算法，目前仅支持sm3，定义为0x0012
    BYTE policy;             //策略，goon为0x01，hang为0x00
    BYTE reserved2[3];       //保留字段，固定为0xffffff
} __attribute__((__packed__)) policy_tpm_t;

/* TPM可信策略文件的结构 */
typedef struct {
    BYTE flashrom_valid[4];    //spi flash rom标记，最多支持4个度量spi flash
                               //度量flash rom1时，则flashrom_flag[0]=0x01
    BYTE area_valid1[4];       //度量的flash rom中的区域标记，最多支持4个度量区域
                               //度量区域为1时，policy[0]中策略生效，后面类推
    policy_tpm_t policy1[4];   //flash rom1中支持的4个度量区域相应的策略信息
    
    BYTE area_valid2[4];
    policy_tpm_t policy2[4];
} __attribute__((__packed__)) policy_tpm_file_t;


#endif
