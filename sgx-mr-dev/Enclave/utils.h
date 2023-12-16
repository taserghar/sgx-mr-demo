
#ifndef  __UTILS__
#define __UTILS__


#include <cstring>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <utility>
#include "Enclave_t.h" /* print_string */
#include "Enclave.h"
#include <stdarg.h>
#include "Enclave.h"
#include "Enclave_t.h" /* print_string */
#include <stdarg.h>
#include "sgx_tcrypto.h"
#include "sgx_trts.h"
#include <stdio.h> /* vsnprintf */

using namespace std;

enum IOMode_t
{
    IOMode_READ, IOMode_WRITE
};

const int enable_oblivious_swap = 0;


#define LOG(fmt, ...) \
    printf("[LOG] %s %s()# " fmt, __FILE__, __func__,##__VA_ARGS__)

#define ERR(fmt, ...) \
    printf("[ERROR] %s %s()# " fmt, __FILE__, __func__,##__VA_ARGS__)


// utils
int min(int x, int y);

// must contain exactly 4 byte
int bytes_to_int(char* bytes);
// bytes must be allocated to size 4 or more
void int_to_bytes(int value, char* bytes);

unsigned int getRandomNumber();

// void  obliviousSort(vector<int>& array);
// void printRecordsFromFile(string inputFileName, int blockSize, int recordLimit);

// clock_t getCurrentTime();
//
// double timeDiff(clock_t prev);


#endif
