#ifndef __CRYPTO__
#define __CRYPTO__


#include "Enclave_t.h" /* print_string */
#include <stdarg.h>
#include "sgx_tcrypto.h"
#include "sgx_trts.h"



const uint32_t ctr_num_bit_size = 16;
const uint32_t AES_BLOCK_SIZE = 16;
const uint32_t CMAC_SIZE = 16;

void aes_encrypt(uint8_t* input_buf, uint8_t* output_buf, int buf_len);
void aes_decrypt(uint8_t* buf, uint8_t* plain_buf, int buf_len);
void compute_CMAC(uint8_t* buf, int buf_len, uint8_t* cmac_tag);


#endif
