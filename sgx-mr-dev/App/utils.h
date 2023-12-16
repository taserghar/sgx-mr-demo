
#ifndef  __UTILS__
#define __UTILS__

#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <utility>
using namespace std;

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

// void printRecordsFromFile(string inputFileName, int blockSize, int recordLimit);

clock_t getCurrentTime();

double getCurrentTimeInMS();

double timeDiff(clock_t prev);
double timeDiff(clock_t prior_event, clock_t later_event);

#endif
