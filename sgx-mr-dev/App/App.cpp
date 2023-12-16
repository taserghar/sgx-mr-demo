/*
 * Copyright (C) 2011-2019 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include <stdio.h>
#include <string.h>
#include <string> //Justin
#include <assert.h>
#include <fstream>
# include <unistd.h>
# include <pwd.h>
# define MAX_PATH FILENAME_MAX
#include <sys/stat.h>
#include "sgx_urts.h"
#include "App.h"
#include "Enclave_u.h"
#include "untrusted.h"
#include "utils.h"
#include "message.h" //Justin

using namespace std;
/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;

//Message object sends message to client
//
Message messageClient = Message();

typedef struct _sgx_errlist_t {
    sgx_status_t err;
    const char *msg;
    const char *sug; /* Suggestion */
} sgx_errlist_t;

/* Error code returned by sgx_create_enclave */
static sgx_errlist_t sgx_errlist[] = {
    {
        SGX_ERROR_UNEXPECTED,
        "Unexpected error occurred.",
        NULL
    },
    {
        SGX_ERROR_INVALID_PARAMETER,
        "Invalid parameter.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_MEMORY,
        "Out of memory.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_LOST,
        "Power transition occurred.",
        "Please refer to the sample \"PowerTransition\" for details."
    },
    {
        SGX_ERROR_INVALID_ENCLAVE,
        "Invalid enclave image.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ENCLAVE_ID,
        "Invalid enclave identification.",
        NULL
    },
    {
        SGX_ERROR_INVALID_SIGNATURE,
        "Invalid enclave signature.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_EPC,
        "Out of EPC memory.",
        NULL
    },
    {
        SGX_ERROR_NO_DEVICE,
        "Invalid SGX device.",
        "Please make sure SGX module is enabled in the BIOS, and install SGX driver afterwards."
    },
    {
        SGX_ERROR_MEMORY_MAP_CONFLICT,
        "Memory map conflicted.",
        NULL
    },
    {
        SGX_ERROR_INVALID_METADATA,
        "Invalid enclave metadata.",
        NULL
    },
    {
        SGX_ERROR_DEVICE_BUSY,
        "SGX device was busy.",
        NULL
    },
    {
        SGX_ERROR_INVALID_VERSION,
        "Enclave version was invalid.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ATTRIBUTE,
        "Enclave was not authorized.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_FILE_ACCESS,
        "Can't open enclave file.",
        NULL
    },
};

/* Check error conditions for loading enclave */
void print_error_message(sgx_status_t ret)
{
    size_t idx = 0;
    size_t ttl = sizeof sgx_errlist/sizeof sgx_errlist[0];

    for (idx = 0; idx < ttl; idx++) {
        if(ret == sgx_errlist[idx].err) {
            if(NULL != sgx_errlist[idx].sug)
                printf("Info: %s\n", sgx_errlist[idx].sug);
            printf("Error: %s\n", sgx_errlist[idx].msg);
            break;
        }
    }

    if (idx == ttl)
    	printf("Error code is 0x%X. Please refer to the \"Intel SGX SDK Developer Reference\" for more details.\n", ret);
}

/* Initialize the enclave:
 *   Call sgx_create_enclave to initialize an enclave instance
 */
int initialize_enclave(void)
{
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;

    /* Call sgx_create_enclave to initialize an enclave instance */
    /* Debug Support: set 2nd parameter to 1 */
    ret = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, NULL, NULL, &global_eid, NULL);
    if (ret != SGX_SUCCESS) {
        print_error_message(ret);
        return -1;
    }

    return 0;
}

/* OCall functions */
void ocall_print_string(const char *str)
{
    /* Proxy/Bridge will check the length and null-terminate
     * the input string to prevent buffer overflow.
     */
    printf("%s", str);
}

char* read_file(const char* filename, int* lSize) {
    FILE * pFile;
    char* buffer;
    // long lSize;
    size_t result;
    
    //printf("filename: %s", filename);

    pFile = fopen ( filename , "rb" );
    if (pFile==NULL) {fputs ("File error",stderr); exit (1);}

    // obtain file size:
    fseek (pFile , 0 , SEEK_END);
    *lSize = ftell (pFile);
    rewind (pFile);

    // allocate memory to contain the whole file:
    buffer = (char*) malloc (sizeof(char)* (*lSize));
    if (buffer == NULL) {fputs ("Memory error",stderr); exit (2);}

    // copy the file into the buffer:
    result = fread (buffer,1,(*lSize),pFile);
    if (result != (*lSize)) {fputs ("Reading error",stderr); exit (3);}

    /* the whole file is now loaded in the memory buffer. */
    // terminate
    fclose (pFile);
    return buffer;
//free (buffer);
}
void ocall_write_file(const char* buf, int size, const char* filename, const char* type) {
    // printf("[APP] ocall_write_file(): %s\n", buf);
    printf("[APP] dec output:");
    for( int i =0 ; i < 100; i++) printf("%02X", buf[i]);
    printf("\n");
    FILE* file;
    file = fopen(filename, type);
    fwrite((uint8_t*)buf, sizeof(uint8_t), size, file);
    fclose(file);
    printf("[APP] ocall_write_file(): %d bytes written to %s\n", sizeof(buf), filename);
}

void ocall_write_file_2(const char* buf, int size, const char* filename, const char* type) {
    // printf("[APP] ocall_write_file(): %s\n", buf);
    printf("[APP] dec output:");
    for( int i =0 ; i < 100; i++) printf("%c", buf[i]);
    printf("\n");
    FILE* file;
    file = fopen(filename, type);
    fwrite((uint8_t*)buf, sizeof(uint8_t), size, file);
    fclose(file);
    printf("[APP] ocall_write_file(): %d bytes written to %s\n", sizeof(buf), filename);
}

void ocall_OpenFile(const char* pFileName, int fileId, int pIOMode){
    // LOG("called\n");
    OpenFile(string(pFileName), fileId, pIOMode);
}
void ocall_CloseFile(int fileId){
    // LOG("called\n");
    CloseFile( fileId);
}
//The global solution is not ideal but works
//Using json to encode the data
std::string global_IOstring = "{\"IOMessageArr\": [";
int global_msg_counter = 0; //Send message at 3000
void ocall_ReadFile(int fileId, int pos, char* pBuf, int pSize, int* byteReads){
    // LOG("called\n");
    //Updating global variable for demo
    string messageType = "READ";
    string blockId = to_string(pos/pSize);
    string currTime = to_string(getCurrentTimeInMS());
    string jsonMsg = "{\"type\": \"" + messageType + "\", \"blockid\": " + blockId + ", \"time\": " + currTime + "},"  ;
    //printf("Message: %s\n", jsonMsg.c_str());
    global_IOstring += jsonMsg;
    global_msg_counter++;
    if(global_msg_counter > 3000)
    {
	    // Remove hanging comma, finish string, send message
	    global_IOstring.pop_back();
	    global_IOstring += "]}";
	    messageClient.sendMessage(global_IOstring.c_str());
        // std::cout<<"-----------------------\n"<<endl;
        // std::cout<<global_IOstring<<endl;

	    // Reset variables
	    global_IOstring = "{\"IOMessageArr\": [";
	    global_msg_counter = 0;
    }

    ReadFile( fileId,  pos,  pBuf,  pSize, byteReads);
}

void ocall_WriteFile(int fileId, int pos, char* pBuf, int pSize, int* ret_status){
    // LOG("called\n");
    // Updating global variable for demo
    string messageType = "WRITE";
    string blockId = to_string(pos/pSize);
    string currTime = to_string(getCurrentTimeInMS());
    string jsonMsg = "{\"type\": \"" + messageType + "\", \"blockid\": " + blockId + ", \"time\": " + currTime + "},"  ;
    //printf("Message: %s\n", jsonMsg.c_str());
    global_IOstring += jsonMsg;
    global_msg_counter++;
    if(global_msg_counter > 3000)
    {
	    global_IOstring.pop_back();
	    global_IOstring += "]}";
	    messageClient.sendMessage(global_IOstring.c_str());

	    global_IOstring = "{\"IOMessageArr\": [";
	    global_msg_counter = 0;
    }
    
    
    WriteFile(fileId,  pos,  pBuf, pSize, ret_status);
}

vector<clock_t>global_timestamps;
void ocall_takeTimestamp(){
    global_timestamps.push_back(getCurrentTime());
}

//Justin
//Uses SQS system to send a message
//Kind of ugly because I needed std::string to do some operations
//There is probably a better way to do this without mixing strings
void ocall_update_client(const char *str){
	bool success = false;
	double currTime = getCurrentTimeInMS();
	string finalMessage = str;

	printf("Message: %s \n", str);
	printf("Time: %f \n", currTime);	
	
	finalMessage = finalMessage +  "@" + to_string(currTime);
	success = messageClient.sendMessage(finalMessage.c_str());
}

void copyFile(string src_file, string dst_file){
    std::ifstream  src(src_file, std::ios::binary);
   std::ofstream  dst(dst_file,   std::ios::binary);
   dst << src.rdbuf();
}

string getSyntheticFileName(int nob) {
    return "synthetic_bsize_" + to_string(nob) + ".txt";
}

inline bool isFileExist (const string& name) {
  struct stat buffer;
  return (stat (name.c_str(), &buffer) == 0);
}

int calculateRecordLimit(int blockSize) {
    double default_block_size = 1000.0;
    double default_record_limit = 15.0;
    return int (blockSize * default_record_limit / default_block_size);
}

void start_analyze(int NOB, int blockSize, int isInEnclaveMemory){


    int recordLimit = 2;//calculateRecordLimit(blockSize);
    int recordSize = 52428800;//1024;
    string src_dir = "../data/synthetic_data/";
    string src_file = getSyntheticFileName(blockSize);
    string dst_dir = "../data/test/";
    // LOG("1\n");
    gen_data(global_eid, NOB,blockSize, recordSize, recordLimit);
     // LOG("2\n");
     // return;
    string dst_file = src_file;
    // if ( !isFileExist(src_dir + src_file)) {
    //     LOG("Error: File Not Exist\n");
    //     exit(1);
    // }
    // copyFile(src_dir + src_file, dst_dir + dst_file);

    prepareSort(global_eid, NOB, blockSize, recordSize, recordLimit, isInEnclaveMemory);
    clock_t initTime = getCurrentTime();
    runSort(global_eid, isInEnclaveMemory);
    double diff = timeDiff(initTime);
    postSort(global_eid, isInEnclaveMemory);
    LOG("::::::::::::::::::::END::::::::::::::::::::::\n");
    // remove((dst_dir + dst_file).c_str());
    LOG("Executed Time:%lf\n", diff);
}


/* Application entry */
int SGX_CDECL main(int argc, char *argv[])
{
    (void)(argc);
    (void)(argv);
	
    /*/ofstream logfile;
    logfile.open("logs.txt", ofstream::out);
    if(logfile.is_open())
    {
    	printf("LOGFILE OPENED\n");
	logfile << "testing";
    }
    logfile.close();*/

//    Aws::SDKOptions options;
//    Aws::InitAPI(options);
//    printf("AWS API INITed\n"); 

    /* Initialize the enclave */
    if(initialize_enclave() < 0){
        printf("Enclave Init error\n");
    }
//------
    printf("App started with %d params\n", argc);
    // sending dummy message queue data
    messageClient.dummySendMessage();
    string param_1 = string(argv[1]);
    if (param_1.compare("wordcount-encoder") == 0) {
        printf("Encoder Call\n");
        int blockCnt = stoi(argv[2]);
        int blockSize = stoi(argv[3]);
        //arg4 is inputfile name
        // arg5 is outputfile
        ecall_encoder(global_eid, blockCnt, blockSize, argv[4], argv[5]);
        printf("Encoder Returned\n");
    } else if (param_1.compare("encoder_join") == 0) {
        printf("Encoder Join Call\n");
        int blockCnt = stoi(argv[2]);
        ecall_encoder_join(global_eid, blockCnt);
        printf("Encoder Returned\n");
    } else if(param_1.compare("wordcount") == 0) {
        int blockSize = stoi(argv[2]);
        int enablePadding = stoi(argv[3]);
        int sort_type = stoi(argv[4]);
        // argv 5 is inputFile
        // argv 6 is outputPath
        global_timestamps.clear();
        clock_t initTime = getCurrentTime();
        printf("%d: %d\n",enablePadding, sort_type );
        ecall_wordcount(global_eid, blockSize, enablePadding, sort_type, argv[5], argv[6]);
        // double reducerTime = timeDiff(global_timestamps[0]);
        double diff = timeDiff(initTime);
        // remove((dst_dir + dst_file).c_str());
        double mapperTime = timeDiff(global_timestamps[0], global_timestamps[1] );
        double sortTime = timeDiff(global_timestamps[1], global_timestamps[2] );
        double reducerTime = timeDiff(global_timestamps[2], global_timestamps[3] );
        // remove((dst_dir + dst_file).c_str());
        LOG("::::::::::::::::::::Word Count::::::::::::::::::::::\n");
        LOG("Executed Time: %lf\n", diff);
        LOG("Mapper Time: %lf\n", mapperTime);
        LOG("Sort Time: %lf\n", sortTime);
        LOG("Reducer Time: %lf\n", reducerTime);
        LOG(":::::::::::::::::::::::END::::::::::::::::::::::::::\n");

    } else if(param_1.compare("kmeans") == 0) {
        int blockSize = stoi(argv[2]);
        int enablePadding = stoi(argv[3]);
        int sort_type = stoi(argv[4]);
        // argv 5 is centroidInputFile
        // arg 6 is coordinateInputFile
        // argv 7 is outputPath
        global_timestamps.clear();
        clock_t initTime = getCurrentTime();
        ecall_kmeans(global_eid, blockSize, enablePadding,sort_type, argv[5], argv[6], argv[7]);
        // double reducerTime = timeDiff(global_timestamps[0]);
        double diff = timeDiff(initTime);
        // remove((dst_dir + dst_file).c_str());
        double mapperTime = timeDiff(global_timestamps[0], global_timestamps[1] );
        double sortTime = timeDiff(global_timestamps[1], global_timestamps[2] );
        double reducerTime = timeDiff(global_timestamps[2], global_timestamps[3] );
        LOG("::::::::::::::::::::KMeans::::::::::::::::::::::\n");
        LOG("Executed Time: %lf\n", diff);
        LOG("Mapper Time: %lf\n", mapperTime);
        LOG("Sort Time: %lf\n", sortTime);
        LOG("Reducer Time: %lf\n", reducerTime);
        LOG(":::::::::::::::::::::END::::::::::::::::::::::::\n");

    } else if(param_1.compare("kmeans-encoder") == 0) {
	printf("Kmeans-Encoder selected\n");
        int totalBlock = stoi(argv[2]);
        int blockSize = stoi(argv[3]);
        global_timestamps.clear();
        clock_t initTime = getCurrentTime();
        // arg 4 is the centroidFile
        // arg 5 is the coordinateFile
	printf("Calling Ecall Encoder\n");
        ecall_kmeans_encoder(global_eid, totalBlock,blockSize, argv[4], argv[5]);
        // double reducerTime = timeDiff(global_timestamps[0]);
        double diff = timeDiff(initTime);
        // remove((dst_dir + dst_file).c_str());
        LOG("::::::::::::::::::::END::::::::::::::::::::::\n");
        LOG("Executed Time: %lf\n", diff);
        // LOG("Reducer Time: %lf\n", reducerTime);
        LOG("::::::::::::::::::::END::::::::::::::::::::::\n");

    } else if(param_1.compare("merge_join") == 0) {
        int enablePadding = stoi(argv[2]);
        int sort_type = stoi(argv[3]);
        global_timestamps.clear();
        clock_t initTime = getCurrentTime();
        ecall_merge_join(global_eid, enablePadding,sort_type);
        // double reducerTime = timeDiff(global_timestamps[0]);
        double diff = timeDiff(initTime);
        // remove((dst_dir + dst_file).c_str());
        LOG("::::::::::::::::::::END::::::::::::::::::::::\n");
        LOG("Executed Time: %lf\n", diff);
        // LOG("Reducer Time: %lf\n", reducerTime);
        LOG("::::::::::::::::::::END::::::::::::::::::::::\n");
    } else if(param_1.compare("memtest") == 0) {
        LOG("%s %s\n", argv[2], argv[3]);
        int n = stoi(argv[2]);
        string seq = string(argv[3]);

        int isSeq = seq.compare("seq") == 0 ? 1 : 0;
        ecall_prepareMemtest(global_eid, n);
        global_timestamps.clear();
        clock_t initTime = getCurrentTime();
        ecall_memtest(global_eid, n, isSeq);
        // double reducerTime = timeDiff(global_timestamps[0]);
        double diff = timeDiff(initTime);
        double avg = diff/n;
        // remove((dst_dir + dst_file).c_str());
        LOG("::::::::::::::::::::END::::::::::::::::::::::\n");
        LOG("Executed Time: %lf\n", avg);
        // LOG("Reducer Time: %lf\n", reducerTime);
        LOG("::::::::::::::::::::END::::::::::::::::::::::\n");
    } else if(param_1.compare("test") == 0) {
        LOG("::::::::::::::::::::TEST::::::::::::::::::::::\n");
        ecall_prepare_test(global_eid);
        clock_t initTime = getCurrentTime();
        ecall_test(global_eid);
        double diff = timeDiff(initTime);
        // remove((dst_dir + dst_file).c_str());
        LOG("::::::::::::::::::::END::::::::::::::::::::::\n");
        LOG("Executed Time: %lf\n", diff);
        // LOG("Reducer Time: %lf\n", reducerTime);
        LOG("::::::::::::::::::::END::::::::::::::::::::::\n");
    } else {
        // data mgmt
        // synthetic sorting
        int nob =stoi(argv[1]);
        int bSize = stoi(argv[2]);
        string data_mgmt = string(argv[3]);
        int isInEnclaveMemory = 0; // bydefault regular mode
        if ( data_mgmt.compare("enclave") == 0) {
            isInEnclaveMemory = 1;
        }
        printf("Calling Start Analyze # of Block:%d Block size %d ()\n", nob, bSize);
        start_analyze(nob, bSize, isInEnclaveMemory);
        printf("End Start Analyze ()\n");
    }


    //Don't send message if you are encoding
    if(param_1.compare("kmeans-encoder") != 0 && param_1.compare("wordcount-encoder") != 0)
    {
	printf("Supposed to send logs from here\n");
       //Final message with all of the memory access information
       //in JSON format: an array of objects that holds type, blockid, time
       //Get rid of the hanging comma
       global_IOstring.pop_back();
       //close the json object
       global_IOstring += "]}";
	
	printf("Sending Message via SQS ...\n");
       messageClient.sendMessage(global_IOstring.c_str());
       //printf("Sending::::\n%s", global_IOstring.c_str());
       string currentTime = to_string(getCurrentTimeInMS());
       string finMsg = "SGXMR_FINISHED@" + currentTime;
       messageClient.sendMessage(finMsg.c_str());
	
    }

    /* Utilize edger8r attributes */
    edger8r_array_attributes();
    edger8r_pointer_attributes();
    edger8r_type_attributes();
    edger8r_function_attributes();

    /* Utilize trusted libraries */
    ecall_libc_functions();
    ecall_libcxx_functions();
    ecall_thread_functions();

    /* Destroy the enclave */
    sgx_destroy_enclave(global_eid);

    printf("Info: SampleEnclave successfully returned.\n");
//    Aws::ShutdownAPI(options);
    return 0;
}
