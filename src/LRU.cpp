//
// Created by Sili Liu on 2020/12/23.
//

#include "LRU.h"
#include <iostream>

/**
 * Initialize LRU list.
 */
LRU::LRU() {
    use_sentinel = new LRUNode;
    free_sentinel = new LRUNode;
    size = 0;
    use_sentinel->next = use_sentinel->previous = use_sentinel;
    free_sentinel->next = free_sentinel->previous = free_sentinel;
    int i;
    LRUNode *p;
    for (i = 0; i < DEFBUFSIZE; i++) {
        p = new LRUNode;
        p->frame_id = DEFBUFSIZE - 1 - i;
        p->next = free_sentinel->next;
        p->previous = free_sentinel;
        free_sentinel->next = free_sentinel->next->previous = p;
        hash_table[DEFBUFSIZE - 1 - i] = p;
    }
}

/**
 * Remove the first element of LRU list.
 */
void LRU::RemoveFirst() {
    if (size == 0)
        return;

    LRUNode *p = use_sentinel->next;
    use_sentinel->next->next->previous = use_sentinel;
    use_sentinel->next = use_sentinel->next->next;
    size--;

    p->next = free_sentinel->next;
    p->previous = free_sentinel;
    free_sentinel->next = free_sentinel->next->previous = p;
}

/**
 * Add a LRU node to the last of the list with the given frame id.
 */
void LRU::AddLast(int frame_id) {
    // get the node of given frame id in spare list and its previous node
    LRUNode *p = hash_table[frame_id];
    LRUNode *q = p->previous;

    // delete the node from spare list
    q->next = p->next;
    p->next->previous = q;

    // add the node to the tail of LRU list
    use_sentinel->previous->next = p;
    p->previous = use_sentinel->previous;
    p->next = use_sentinel;
    use_sentinel->previous = p;
    size++;
}

/**
 * Remove the LRU node which contains the give frame id.
 */
void LRU::Remove(int frame_id) {
    LRUNode *p, *q;
    // get the node to delete with hash table and then get its previous node
    q = hash_table[frame_id];
    p = q->previous;

    // delete the node from LRU list
    p->next = q->next;
    p->next->previous = p;
    size--;

    // add the node to the spare list as the first node
    q->next = free_sentinel->next;
    q->previous = free_sentinel;
    free_sentinel->next = free_sentinel->next->previous = q;
}

/**
 * Get the size of this LRU list.
 */
int LRU::GetSize() {
    return size;
}

/**
 * Get the frame id which is contained in the first element of the list.
 */
int LRU::GetFirst() {
    if (size == 0) return -1;
    return use_sentinel->next->frame_id;
}

int LRU::GetFreeFrame() {
    return free_sentinel->next->frame_id;
}

/**
 * delete all the allocated space related to LRU list.
 */
void LRU::FreeLRU() {
    LRUNode *p;
    while (use_sentinel->next != use_sentinel) {
        p = use_sentinel->next;
        use_sentinel->next = p->next;
        p->next->previous = use_sentinel;
        delete p;
    }
    delete use_sentinel;

    while (free_sentinel->next != free_sentinel) {
        p = free_sentinel->next;
        free_sentinel->next = p->next;
        p->next->previous = free_sentinel;
        delete p;
    }
    delete free_sentinel;
}