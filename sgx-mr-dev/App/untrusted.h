#ifndef __UNTRUSTED__
#define __UNTRUSTED__

#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <utility>
using namespace std;

enum IOMode_t
{
    IOMode_READ, IOMode_WRITE
};

class OFile {
public:

    IOMode_t mIOMode;
    string mFileName;
    int mFileId;
    ifstream rFile;
    ofstream wFile;
    vector<char>fileBuf;
    OFile();
    OFile(string pFileName, int pFileId, IOMode_t pIOMode);

    void updateSeek(int pos);
    void read(char* memblock, int size, int* byteReads);
    void read(int pos, char* memblock, int size, int* bytesRead);
    void write(char* memblock, int size, int* status);
    void write(int pos, char* memblock, int size, int* status);
    void close();
};

class FileManager {
private:
    vector<OFile> fileCollection;
    unordered_map<int,int> fileMap; // <id,index>
    FileManager(){};
public:
    static FileManager* getInstance();
    OFile& createFile(string pFilename, int pFileId, IOMode_t pMode);
    OFile& getFile(int pFileId);
    void removeFile(int pFileId);
};

// *********************** dummy enclave interface start //
void OpenFile(string pFileName, int fileId, int pIOMode);
void CloseFile(int fileId);
void ReadFile(int fileId, int pos, char* pBuf, int pSize, int* byteReads);
void WriteFile(int fileId, int pos, char* pBuf, int pSize, int* status);
// *********************** dummy enclave interface end //

#endif
