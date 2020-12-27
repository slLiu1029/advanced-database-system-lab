//
// Created by Sili Liu on 2020/12/23.
//

#include <iostream>
#include <cstdio>

using namespace std;

#ifndef LAB_DSMGR_H
#define LAB_DSMGR_H

#define FRAMESIZE 4096
#define MAXPAGES 50000

typedef struct _bFrame {
    _bFrame();

    char field[FRAMESIZE];
} bFrame;

class DSMgr {
public:

    DSMgr();

    int OpenFile(string filename);

    int CloseFile();

    bFrame ReadPage(int page_id);

    int WritePage(int frame_id, bFrame frm);

    int Seek(int offset, int pos);

    FILE *GetFile();

    void IncNumPages();

    int GetNumPages();

    void SetUse(int index, int use_bit);

    int GetUse(int index);

    int GetNumIOs();    // returns number of I/O operations

    int GetBlockNum(int page_id);   // returns page id in file

    void SetIndex(int page_id, int block_num);  // set index of memory virtual page id to file page id

    int BlockNumToOffset(int block_num);    // returns offset from the beginning of file

    int LookForFreePage();  // returns the id of a free virtual page of memory

    int LookForFreeBlock(); // returns the id of a free page of file

    void FreeDSMgr();   // deletes all allocated space

private:

    FILE *currFile;

    int numPages;
    int blocks[MAXPAGES];   // page id in file manager
    int pages[MAXPAGES];    // page id in memory
    int index[MAXPAGES];    // page id of memory to page id of file

    int numIOs;

    static void GenerateDatabase();
};


#endif //LAB_DSMGR_H
