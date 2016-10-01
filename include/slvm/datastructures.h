#ifndef SLVM_DATA_STRUCTURE_H
#define SLVM_DATA_STRUCTURE_H

typedef struct SLVM_LinkedList_ SLVM_LinkedList;
typedef struct SLVM_LinkedListNode_ SLVM_LinkedListNode;

struct SLVM_LinkedListNode_
{
    SLVM_LinkedListNode *previous;
    SLVM_LinkedListNode *next;
};

struct SLVM_LinkedList_
{
    SLVM_LinkedListNode *first;
    SLVM_LinkedListNode *last;
};

void slvm_list_addNode(SLVM_LinkedList *list, SLVM_LinkedListNode *node);
void slvm_list_removeNode(SLVM_LinkedList *list, SLVM_LinkedListNode *node);

#endif /* SLVM_DATA_STRUCTURE_H */
