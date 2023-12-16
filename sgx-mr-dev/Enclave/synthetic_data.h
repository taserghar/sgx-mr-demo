#ifndef __SYNTHETIC_DATA__
#define __SYNTHETIC_DATA__

#include "RecordStream.h"
#include "BlockFile.h"
#include "Writable.h"
#include "utils.h"
#include "Enclave.h"
#include "Enclave_t.h" /* print_string */
#include <stdarg.h>
#include "Enclave.h"
#include "Enclave_t.h" /* print_string */
#include <stdarg.h>
#include "sgx_tcrypto.h"
#include "sgx_trts.h"
#include <stdio.h> /* vsnprintf */
#include <string.h>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <iostream>

Record<StringWritable,StringWritable> getSyntheticRecord();
 void convTextToRecord( string outputFile);
 void testRecordInputStream(string inputFile);
 void printRecordsFromFile(string inputFileName, int blockSize, int recordLimit);
 void gen_synthetic(int p_nob, int p_blockSize, int p_recordSize, int p_recordLimit);

#endif
