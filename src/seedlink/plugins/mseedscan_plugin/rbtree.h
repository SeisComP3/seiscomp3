/***************************************************************************
 * rbtree.h:
 * 
 * Red-Black Tree routines, imlements balanced tree data structures.
 *
 * The code base was originally written by Emin Marinian:
 * http://www.csua.berkeley.edu/~emin/index.html
 *
 * modified: 2004.230
 ***************************************************************************/

#ifndef RBTREE_H
#define RBTREE_H 1

#include "stack.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RBNode {
  void* key;
  void* data;
  int red;    /* if red=0 then the node is black */
  struct RBNode* left;
  struct RBNode* right;
  struct RBNode* parent;
} RBNode;


/* Compare(a,b) should return 1 if *a > *b, -1 if *a < *b, and 0 otherwise */
/* Destroy(a) takes a pointer to whatever key might be and frees it accordingly */
typedef struct RBTree {
  int (*Compare) (const void* a, const void* b);
  void (*DestroyKey) (void* a);
  void (*DestroyData) (void* a);

  /*  A sentinel is used for root and for nil.  These sentinels are */
  /*  created when RBTreeCreate is called.  root->left should always */
  /*  point to the node which is the root of the tree.  nil points to a */
  /*  node which should always be black but has aribtrary children and */
  /*  parent and no key or data.  The point of using these sentinels is so */
  /*  that the root and nil nodes do not require special cases in the code */
  RBNode* root;
  RBNode* nil;
} RBTree;


RBTree* RBTreeCreate (int  (*CompareFunc)(const void*, const void*),
		      void (*DestroyKeyFunc)(void*),
		      void (*DestroyDataFunc)(void*));
RBNode* RBTreeInsert (RBTree *tree, void *key, void *data);
RBNode* RBFind (RBTree *tree, void *key);
RBNode* TreeSuccessor (RBTree *tree, RBNode *node);
RBNode* TreePredecessor (RBTree *tree, RBNode *node);
void    RBTreeDestroy (RBTree *tree);
void    RBDelete (RBTree *tree, RBNode *node);
void    RBTraverse (RBTree *tree, void (*NodeFunc)(RBNode*));
void    RBBuildStack (RBTree *tree, Stack *stack);
void    RBTreePrint (RBTree *tree,
		     void (*PrintKey)(void*),
		     void (*PrintData)(void*));


#ifdef __cplusplus
}
#endif

#endif /* RBTREE_H */
