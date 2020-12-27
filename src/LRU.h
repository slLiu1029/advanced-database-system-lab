//
// Created by Sili Liu on 2020/12/23.
//

#ifndef LAB_LRU_H
#define LAB_LRU_H

#define DEFBUFSIZE 80000

typedef struct _LRUNode {
    int frame_id;
    struct _LRUNode *previous;
    struct _LRUNode *next;
} LRUNode;

class LRU {
public:
    LRU();

    void RemoveFirst();

    void AddLast(int frame_id);

    void Remove(int frame_id);

    int GetSize();

    int GetFirst();

    int GetFreeFrame(); // returns a free frame id from spare list

    void FreeLRU();

private:

    LRUNode *use_sentinel, *free_sentinel;
    LRUNode *hash_table[DEFBUFSIZE];
    int size;       // first node of LRU
};


#endif //LAB_LRU_H
