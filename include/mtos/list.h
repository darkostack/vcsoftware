#ifndef MTOS_LIST_H
#define MTOS_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mtListNode
{
    struct mtListNode *mNext;
} mtListNode;

#ifdef __cplusplus
}
#endif

#endif /* MTOS_LIST_H */
