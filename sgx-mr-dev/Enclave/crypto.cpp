#include "crypto.h"
#include "utils.h"

#include <stdio.h> /* vsnprintf */
#include <string.h>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <iostream>

const sgx_aes_ctr_128bit_key_t* AES_KEY_SGX_MR = (sgx_aes_ctr_128bit_key_t*)"1234567812345678";

void generate_const_iv(uint8_t* iv) {
    memset(iv, 'A', AES_BLOCK_SIZE * sizeof(uint8_t));
}

void aes_encrypt(uint8_t* input_buf, uint8_t* output_buf, int buf_len) {

    uint8_t iv[AES_BLOCK_SIZE];
    uint8_t* enc_out = (uint8_t*)output_buf;
    uint8_t* enc_out_t = enc_out;
    generate_const_iv(iv);
    sgx_status_t stat = sgx_aes_ctr_encrypt(AES_KEY_SGX_MR,
                            (const uint8_t*)input_buf,
                            buf_len,
                            iv,
                            ctr_num_bit_size,
                            enc_out_t);

    if (stat != SGX_SUCCESS) LOG("Failed\n");
}

void aes_decrypt(uint8_t* buf, uint8_t* plain_buf, int buf_len) {

    uint8_t* dec_buf = plain_buf;
    uint8_t ctr[AES_BLOCK_SIZE];
    generate_const_iv(ctr);
    sgx_status_t ret = sgx_aes_ctr_decrypt(AES_KEY_SGX_MR,
                            (const uint8_t*)buf,
                            buf_len ,
                            ctr,
                            ctr_num_bit_size,
                            dec_buf);

    if (ret != SGX_SUCCESS) LOG("Failed\n");
}

void compute_CMAC(uint8_t* buf, int buf_len, uint8_t* cmac_tag) {

    sgx_status_t ret =  sgx_rijndael128_cmac_msg(AES_KEY_SGX_MR,
                                                buf,
                                                buf_len,
                                                (sgx_cmac_128bit_tag_t*)cmac_tag);
    if (ret != SGX_SUCCESS) LOG("Failed\n");
}
