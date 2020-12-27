//
// Created by Sili Liu on 2020/12/23.
//

#include "DSMgr.h"

using namespace std;

/**
 * Initialize the Data Storage Manager
 */

_bFrame::_bFrame() {}

DSMgr::DSMgr() {
    GenerateDatabase(); // after first compilation, this line can be commented out

    OpenFile("../data/data.dbf");   // open the database file

    numPages = MAXPAGES;    // initially, all the pages ids are used
    int i;
    for (i = 0; i < MAXPAGES; i++) {    // set the use array of page ids, block ids and the index table
        blocks[i] = 1;
        pages[i] = 1;
        index[i] = i;
    }

    numIOs = 0;
}

/**
 * Open the file whose name is given as filename
 */
int DSMgr::OpenFile(string filename) {
    const char *open_file_name = filename.c_str();
    currFile = fopen(open_file_name, "rb+");
    if (currFile == NULL) return 0;
    return 1;
}

/**
 * Close the file
 */
int DSMgr::CloseFile() {
    return fclose(currFile);
}

/**
 * This method is called by the FixPage method in the buffer manager. It returns a buffer frame that contains the data
 * it reads from the disk.
 */
bFrame DSMgr::ReadPage(int page_id) {
    bFrame frm = _bFrame();
    int block_num = GetBlockNum(page_id);   // get the corresponding id of block in the disk.
    if (block_num == -1) return frm;

    // read the frame of the block and return it
    numIOs++;
    int offset = BlockNumToOffset(block_num);
    Seek(offset, 0);
    fread(frm.field, sizeof(char), FRAMESIZE, currFile);
    Seek(0, 0);
    return frm;
}

/**
 * Write page to the disk, which will increase IO by one and decrease number of file page by one
 */
int DSMgr::WritePage(int frame_id, bFrame frm) {
    numIOs++;

    // Get the corresponding id of block in the disk. If there is no corresponding block id of the given page id, a
    // free block in the disk needs to be found and update the index table.
    int block_num = GetBlockNum(frame_id);
    if (block_num == -1) {
        block_num = LookForFreeBlock();
        SetIndex(frame_id, block_num);
        IncNumPages();
    }

    // get the offset of the block
    int offset = BlockNumToOffset(block_num);
    Seek(offset, 0);
    fwrite(frm.field, sizeof(char), FRAMESIZE, currFile);
    Seek(0, 0); // rewind the file pointer

    return FRAMESIZE;
}

/**
 * Move the file pointer from pos to pos+offset
 */
int DSMgr::Seek(int offset, int pos) {
    return fseek(currFile, offset, pos);
}

/**
 * Get file pointer currFile
 */
FILE *DSMgr::GetFile() {
    return currFile;
}

/**
 * Increase number of blocks
 */
void DSMgr::IncNumPages() {
    numPages++;
}

/**
 * Get number of pages that are being used
 */
int DSMgr::GetNumPages() {
    return numPages;
}

/**
 * Set pages[index] to use_bit
 */
void DSMgr::SetUse(int index, int use_bit) {
    pages[index] = use_bit;
}

/**
 * Get use_bit of pages[index]
 */
int DSMgr::GetUse(int index) {
    return pages[index];
}

/**
 * Get current number of disk I/Os
 */
int DSMgr::GetNumIOs() {
    return numIOs;
}

/**
 * This is a helper method used for initialization to generate data.dbf. In the file "data.dbf", there are 49
 * directory pages and 50,000 data pages. The directory pages store the pointers, each of which points to a data page.
 */
void DSMgr::GenerateDatabase() {
    FILE *data_file = fopen("../data/data.dbf", "wb");

    int content[1024];
    int i, j, offset;
    for (i = 0; i < 1024; i++) content[i] = -1;

    // initialize the directory pages
    for (i = 0; i < 49; i++) {
        content[0] = (i + 1) * FRAMESIZE;
        offset = (i * 1023 + 49) * FRAMESIZE;
        for (j = 1; j < 1024; j++) {
            content[j] = offset;
            offset += FRAMESIZE;
        }


        fwrite(content, sizeof(int), 1024, data_file);
    }

    // initialize the data pages
    for (i = 49; i < 50049; i++) {
        for (j = 0; j < 1024; j++) content[j] = i - 49;

        fwrite(content, sizeof(int), 1024, data_file);
    }

    fclose(data_file);
}

/**
 * Transfers the file page id to the offset (physical address in disk).
 */
int DSMgr::BlockNumToOffset(int block_num) {
    int dir_num = block_num / 1023, offset_in_dir = block_num % 1023 + 1;
    int i, offset = 0;

    // find the directory page
    for (i = 1; i <= dir_num; i++) {
        fread(&offset, sizeof(int), 1, currFile);
        Seek(offset, 0);
    }
    offset += offset_in_dir * 4;    // get the offset of the pointer that points to the block we want
    Seek(offset, 0);

    fread(&offset, sizeof(int), 1, currFile);   // read the pointer and get the offset
    Seek(0, 0);                            // rewind the file pointer
    return offset;
}

/**
 * Returns the index of the memory page id.
 */
int DSMgr::GetBlockNum(int page_id) {
    return index[page_id];
}

/**
 * Set the index of the memory page id.
 */
void DSMgr::SetIndex(int page_id, int block_num) {
    index[page_id] = block_num;
}

/**
 * Returns a memory page id that can be reused, which will be used by File and Access Manager later.
 */
int DSMgr::LookForFreePage() {
    if (numPages ==MAXPAGES) return -1;

    int page_id;
    for (page_id = 0; page_id < MAXPAGES && pages[page_id] == 0; page_id++) {}  // looks for a free page id
    SetUse(page_id, 1);
    return page_id;
}

/**
 * Returns an id of a file page that is empty, which will be used for writing data to the disk.
 */
int DSMgr::LookForFreeBlock() {
    int page_id;
    for (page_id = 0; page_id < MAXPAGES && blocks[page_id] == 0; page_id++) {}     // looks for a free block id
    if (page_id < MAXPAGES) blocks[page_id] = 1;
    return page_id;
}

/**
 * Deletes all the allocated space related to this Data Storage Manager.
 */
void DSMgr::FreeDSMgr() {
    fclose(currFile);
}