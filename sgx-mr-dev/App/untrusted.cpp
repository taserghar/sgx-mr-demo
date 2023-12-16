#include "untrusted.h"
#include "utils.h"

OFile::OFile(){}
OFile::OFile(string pFileName, int pFileId, IOMode_t pIOMode) {
    mFileName = pFileName;
    mIOMode = pIOMode;
    mFileId = pFileId;

    switch (pIOMode) {
        case IOMode_READ:
            {
                clock_t initTime = getCurrentTime();

                rFile = ifstream(pFileName, ios::in|ios::binary|ios::ate);
                if (!rFile.is_open()) {
                    ERR("Unable to open : %s\n", pFileName.c_str());
                    exit(1);
                }
                // rFile.seekg (0, ios::beg);
                rFile.seekg(0, std::ios_base::end);
                std::streampos fileSize = rFile.tellg();
                fileBuf.resize(fileSize);

                rFile.seekg(0, std::ios_base::beg);
                rFile.read(&fileBuf[0], fileSize);
                rFile.close();

                double diff = timeDiff(initTime);

                // LOG("[%s]Reading Time: %.2lf milis\n", mFileName.c_str(), diff);
            }
            break;

        case IOMode_WRITE:
            break;
        default:
            ERR("This line shoudl not be reached\n");
        }
}

void OFile::updateSeek(int pos) {
    switch (mIOMode) {
        case IOMode_READ:
            // rFile.clear();
            rFile.seekg (pos, ios::beg);
        break;
        case IOMode_WRITE:
            wFile.clear();
            wFile.seekp (pos, ios::beg);
        break;
        default:
            ERR("This line shoudl not be reached\n");
        }
}

void OFile::read(char* memblock, int size, int* bytesRead ) {
    if ( mIOMode == IOMode_READ) {
        if(rFile.eof()){
            *bytesRead = 0;
            return;
        }
        rFile.read(memblock, size);

        *bytesRead = (int) rFile.gcount();
    } else{
        ERR("read() not supported for %s\n", mFileName.c_str());
    }
}

void OFile::read(int pos, char* memblock, int size, int* bytesRead) {
    // LOG("enter %d %d :: %d\n", pos, size, (int)fileBuf.size());
    if ( mIOMode == IOMode_READ) {
	//fileBuf.size() needs to be unsigned for larger files
	//LOG("fileBuf.size() = %u\n", fileBuf.size());
        if( pos + size <= (unsigned int) fileBuf.size()) {
            std::copy(fileBuf.begin() + pos, fileBuf.begin() + pos + size, memblock);
            *bytesRead = size;
        } else {
            // LOG("returning zero\n");
            *bytesRead = 0;
        }
    } else{
        ERR("read() not supported for %s\n", mFileName.c_str());
    }
    // LOG("End\n");
}

void OFile::write(char* memblock, int size, int* status ) {
    // @depricated
    if ( mIOMode == IOMode_WRITE) {
        wFile.write (memblock, size);
        *status = 0;
    } else{
        *status = -1;
        ERR("write() not supported for %s\n", mFileName.c_str());
    }
}

void OFile::write(int pos, char* memblock, int size, int*status ) {
    if( pos + size > (int) fileBuf.size()) {
        fileBuf.resize(pos + size);
    }
    // LOG("1\n");
    std::copy(memblock, memblock + size, fileBuf.begin() + pos);
    *status = 0;
    // LOG("2\n");

}

void OFile::close(){
    clock_t initTime = getCurrentTime();

    wFile = ofstream(mFileName, ios::out|ios::binary);
    if (!wFile.is_open()) {
        ERR("Unable to open : %s\n", mFileName.c_str());
        exit(1);
    }

    wFile.seekp (0, ios::beg);
    wFile.write(&fileBuf[0], fileBuf.size());
    fileBuf.clear();
    fileBuf.resize(0);
    wFile.close();
    // LOG(" %s [Done]\n", mFileName.c_str());

}

// ------------------- FileManager

FileManager* FileManager::getInstance(){
    static FileManager* instance;
    // Thread safety not required for this SGX project
    if (instance == NULL){
        instance = new FileManager();
    }
    return instance;
}

OFile& FileManager::createFile(string pFileName, int pFileId, IOMode_t pIOMode) {

    int pos = (int) fileCollection.size();
    fileCollection.emplace_back(OFile(pFileName, pFileId, pIOMode));
    if (fileMap.find(pFileId) == fileMap.end()) {
        fileMap.insert(make_pair(pFileId, pos));
    } else {
        ERR("%s already exist\n", pFileName.c_str());
        exit(1);
    }
    return fileCollection[pos];
}

OFile& FileManager::getFile(int pFileId) {
    // // not handling exception
    // for (const auto &[k, v] : fileMap)
    //     std::cout << "*******(" << k << ", " << v << ") " << std::endl;
    // // considering correct file always exist
    if ( fileMap.find(pFileId) == fileMap.end()) {
        ERR("File with id:%d does not exist\n", pFileId);
        exit(1);
    }
    int pos = fileMap[pFileId];
    return fileCollection[pos];
}

void FileManager::removeFile(int pFileId) {
    //TODO: This functon has problem, if I remove it not handling properly

    // for (const auto &[k, v] : file_map)
    //     std::cout << "*******(" << k << ", " << v << ") " << std::endl;
    // int pos;
    // if(fileMap.find(pFileId) != fileMap.end()) {
    //     pos = fileMap[pFileId];
    //     fileCollection.erase(fileCollection.begin() + pos);
    //     fileMap.erase(pFileId);
    // } else{
    //         ERR("File with id:%d does not exist\n", pFileId);
    // }

}

// *********************** dummy enclave interface start //
void OpenFile(string pFileName, int pFileId, int pIOMode) {
    // LOG("called()\n");
    FileManager* mFileManager = FileManager:: getInstance();
    mFileManager->createFile(pFileName, pFileId, (IOMode_t) pIOMode);
}

void CloseFile(int pFileId) {
    // LOG("Called\n");
    FileManager* fileManager = FileManager:: getInstance();
    OFile& file = fileManager->getFile(pFileId);
    file.close();
    fileManager->removeFile(pFileId);
}

void ReadFile(int pFileId, int pos, char* pBuf, int pSize, int* pByteReads) {
    // LOG("\n");
   
    FileManager* fileManager = FileManager:: getInstance();
    OFile& file = fileManager->getFile(pFileId);
    // file.updateSeek(pos);
    file.read(pos, pBuf, pSize, pByteReads);
}

void WriteFile(int pfileId, int pos, char* pBuf, int pSize, int* pStatus) {
    
    // LOG("1\n");
    FileManager* fileManager = FileManager:: getInstance();
    // LOG("2\n");
    OFile& file = fileManager->getFile(pfileId);
    // file.updateSeek(pos);
    // LOG("3\n");
    file.write(pos, pBuf, pSize, pStatus);
    // LOG("4\n");
}
// *********************** dummy enclave interface end //
