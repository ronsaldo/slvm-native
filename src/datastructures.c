#include <assert.h>
#include <stddef.h>
#include "slvm/datastructures.h"

void slvm_list_addNode(SLVM_LinkedList *list, SLVM_LinkedListNode *node)
{
    assert(list);
    assert(node);

    /* Set links in the node. */
    node->previous = list->last;
    node->next = NULL;

    /* Set the new last node. */
    list->last = node;

    /* Update the previous link. */
    if(node->previous)
        node->previous->next = node;
    else
        list->first = node;
}

void slvm_list_removeNode(SLVM_LinkedList *list, SLVM_LinkedListNode *node)
{
    assert(list);
    assert(node);

    /* Unlink the previous and the next node. */
    if(node->previous)
        node->previous->next = node->next;
    else
        list->first = node->next;

    if(node->next)
        node->next->previous = node->previous;
    else
        list->last = node->previous;

    node->previous = node->next = NULL;
}
