//
// Created by Sili Liu on 2020/12/23.
//

#include "BMgr.h"
#include <iostream>

using namespace std;

/* Initialize BCB */
_BCB::_BCB() {
    page_id = -1;
    frame_id = -1;
    dirty = 0;
    latch = 1;
    count = 0;
    next = NULL;
}

_NewPage::_NewPage() {
    page_id = frame_id = -1;
}

/**
 * Initialize Buffer Manager.
 */
BMgr::BMgr() {
    //initialize LRU list
    lru = new LRU();

    // initialize two hash tables and buffer of each frame
    int i;
    for (i = 0; i < DEFBUFSIZE; i++) {
        ftop[i] = -1;
        ptof[i] = new _BCB();
        buffer[i] = new bFrame;
    }

    numFreeFrame = DEFBUFSIZE;
    hits = 0;
    dsmgr = new DSMgr();        // initialize a Data Storage Manager
}

/**
 * Give a page id and return its frame id. The file and access manager will call this page with the page_id that is
 * in the record_id of the record. The function looks to see if the page is in the buffer already and returns the
 * corresponding frame_id if it is. If the page is not resident in the buffer yet, it selects a victim page, if needed,
 * and loads in the requested page.
 */
int BMgr::FixPage(int page_id, int prot) {
    BCB *p = ptof[Hash(page_id)];
    for (; p->next != NULL && p->next->page_id != page_id; p = p->next) {}  // find the BCB

    // successfully find the BCB which contains the page id we want
    if (p->next != NULL) {
        hits++;                             // Hit!

        int frame_id = p->next->frame_id;

        // Move the corresponding LRU node to the last of the list (most recently used)
        RemoveLRUEle(frame_id);
        lru->AddLast(frame_id);

        if (prot == 1) p->next->dirty = 1;

        return frame_id;
    }

    // Buffer does not contain this page id. If the operation is reading, we need to load the data of the corresponding
    // block in the disk. Otherwise, the new frame is what the user wants to write.
    bFrame new_frame;
    if (prot == 0) new_frame = dsmgr->ReadPage(page_id);

    // call FixNewPage to get a free frame id
    int new_frame_id;
    new_frame_id = FixNewPage().frame_id;

    // add the page to the frame
    ftop[new_frame_id] = page_id;
    BCB *q = new _BCB();
    q->page_id = page_id;
    q->frame_id = new_frame_id;
    q->dirty = prot;
    q->next = ptof[Hash(page_id)]->next;
    ptof[Hash(page_id)]->next = q;
    lru->AddLast(new_frame_id);

    numFreeFrame--;

    // store the data to the frame
    int i;
    for (i = 0; i < FRAMESIZE; i++) buffer[new_frame_id]->field[i] = new_frame.field[i];

    return new_frame_id;
}

/**
 * Return a free page id and a free frame id. The page id returned is used for File and Access Manager to store data.
 */
NewPage BMgr::FixNewPage() {
    int free_page_id, free_frame_id;
    // if there is no free frame, select a victim
    free_frame_id = numFreeFrame > 0 ? LookForFreeFrame() : SelectVictim();
    free_page_id = dsmgr->LookForFreePage();    // look for a page that can be reused

    NewPage new_page = _NewPage();
    new_page.page_id = free_page_id;
    new_page.frame_id = free_frame_id;
    return new_page;
}

/**
 * Find a free frame and return its id.
 */
int BMgr::LookForFreeFrame() {
    return lru->GetFreeFrame();
}

int BMgr::UnfixPage(int page_id) {
    return 0;
}

/**
 * Get the number of free frames.
 */
int BMgr::NumFreeFrames() {
    return numFreeFrame;
}

/**
 * If there is no free frame, a frame needs to be selected by LRU standard. We need to delete its corresponding BCB,
 * LRU and reset the hash table
 */
int BMgr::SelectVictim() {
    // delete lru node
    int delete_frame_id = lru->GetFirst();
    lru->RemoveFirst();

    int delete_page_id = ftop[delete_frame_id];
    BCB *p;
    RemoveBCB(p, delete_page_id);       // remove its BCB
    ftop[delete_frame_id] = -1;         // set frame id to page id -1

    numFreeFrame++;

    return delete_frame_id;
}

/**
 * Remove the Buffer Control Block for the page_id from the array.
 */
void BMgr::RemoveBCB(BCB *ptr, int page_id) {
    // find the BCB to remove
    BCB *q;
    for (ptr = ptof[Hash(page_id)]; ptr->next != NULL && ptr->next->page_id != page_id; ptr = ptr->next) {}
    q = ptr->next;
    ptr->next = q->next;

    // if dirty, write the buffer of the frame to the disk
    if (q->dirty == 1) dsmgr->WritePage(q->page_id, *buffer[q->frame_id]);

    delete q;
}

/**
 * removes the LRU element for the given frame id from the list
 */
void BMgr::RemoveLRUEle(int frid) {
    lru->Remove(frid);
}

void BMgr::SetDirty(int frame_id) {
    int page_id = ftop[frame_id];
    if (page_id == -1) return;

    BCB *p = ptof[Hash(page_id)];
    for (; p->next != NULL && p->next->page_id != page_id; p = p->next) {}
    p->next->dirty = 1;
}

void BMgr::UnsetDirty(int frame_id) {
    int page_id = ftop[frame_id];
    if (page_id == -1) return;

    BCB *p = ptof[Hash(page_id)];
    for (; p->next != NULL && p->next->page_id != page_id; p = p->next) {}
    p->next->dirty = 0;
}

/**
 * Write out all block that are still in the buffer that may need to be written. It will only write blocks out to the
 * file if the dirty_bit is one.
 */
void BMgr::WriteDirtys() {
    int i;
    BCB *p;
    for (i = 0; i < DEFBUFSIZE; i++)
        for (p = ptof[i]->next; p != NULL; p = p->next)
            if (p->dirty == 1) {
                dsmgr->WritePage(p->page_id, *buffer[p->frame_id]);
                p->dirty = 0;
            }
}

void BMgr::PrintFrame(int frame_id) {
    if (ftop[frame_id] == -1) return;
    cout << buffer[frame_id]->field << endl;
}

/**
 * Take the page_id as the parameter and return the frame id.
 */
int BMgr::Hash(int page_id) {
    return page_id % DEFBUFSIZE;
}

/**
 * Get the number of hits.
 */
int BMgr::GetHits() {
    return hits;
}

/**
 * Delete all the allocated space related to the Buffer Manager.
 */
void BMgr::FreeBMgr() {
    // delete LRU object
    lru->FreeLRU();
    delete lru;

    dsmgr->FreeDSMgr();
    delete dsmgr;       // delete data storage manager

    // delete all the Buffer Control Blocks
    int i;
    BCB *p, *q;
    for (i = 0; i < DEFBUFSIZE; i++) {
        delete buffer[i];

        p = ptof[i];
        while (p != NULL) {
            q = p->next;
            delete p;
            p = q;
        }
    }
}

/**
 * Get number of I/Os.
 */
int BMgr::GetNumIOs() {
    return dsmgr->GetNumIOs();
}

/**
 * Returns the offset of the block.
 */
int BMgr::GetOffset(int block_num) {
    return dsmgr->BlockNumToOffset(block_num);
}