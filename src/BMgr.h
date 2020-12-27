//
// Created by Sili Liu on 2020/12/23.
//

#include "LRU.h"
#include "DSMgr.h"

#ifndef LAB_BMGR_H
#define LAB_BMGR_H

typedef struct _BCB {
    _BCB();

    int page_id;
    int frame_id;
    int latch;
    int count;
    int dirty;
    struct _BCB *next;
} BCB;

typedef struct _NewPage {
    _NewPage();

    int page_id;
    int frame_id;
} NewPage;

class BMgr {
public:

    BMgr();

    // Interface functions

    int FixPage(int page_id, int prot);

    NewPage FixNewPage();

    int UnfixPage(int page_id);

    int NumFreeFrames();

    // Internal Functions
    int SelectVictim();

    int Hash(int page_id);

    void RemoveBCB(BCB *ptr, int page_id);

    void RemoveLRUEle(int frid);

    void SetDirty(int frame_id);

    void UnsetDirty(int frame_id);

    void WriteDirtys();

    void PrintFrame(int frame_id);

    int GetHits();  // returns hits

    void FreeBMgr();    // delete all allocated space

    int GetNumIOs();    // returns I/Os

    int GetOffset(int block_num);   // returns offset

private:

    int ftop[DEFBUFSIZE];   // frame to page id
    BCB *ptof[DEFBUFSIZE];  // page to frame id
    bFrame *buffer[DEFBUFSIZE]; // frame array

    LRU *lru;   // LRU object
    int numFreeFrame;   // number of free frames
    int hits;   // number of hits
    DSMgr *dsmgr;   // data storage manager object

    int LookForFreeFrame(); // returns a free frame id
};


#endif //LAB_BMGR_H
