/***************************************************************************
 * rbtree.h:
 * 
 * Red-Black Tree routines, imlements balanced tree data structures.
 *
 * The code base was originally written by Emin Marinian:
 * http://www.csua.berkeley.edu/~emin/index.html
 *
 * Further modifications were for cleanup, clarity and some
 * optimization.
 *
 * modified: 2004.230
 ***************************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "rbtree.h"

/***********************************************************************
 *  SafeMalloc:
 *
 *  mallocs new memory and return results.  If malloc fails, prints
 *  error message.
 ***********************************************************************/
void *
SafeMalloc (size_t size)
{
  void * result;
  
  if ( ! (result = malloc(size)) )
    printf ("memory overflow: malloc failed in SafeMalloc.");
  
  return (result);
}


/***********************************************************************
 *  FUNCTION:  RBTreeCreate
 *
 *  INPUTS: All the inputs are names of functions.  CompareFunc takes
 *  two void pointers to keys and returns 1 if the first arguement is
 *  "greater than" the second.  DestroyKeyFunc takes a pointer to a
 *  key and destroys it in the appropriate manner when the node
 *  containing that key is deleted.  DestroyDataFunc takes a pointer
 *  to the data of a node and destroys it.
 *
 *  OUTPUT:  This function returns a pointer to the newly created
 *  red-black tree.
 *
 *  Modifies Input: none
 ***********************************************************************/
RBTree *
RBTreeCreate ( int (*CompareFunc) (const void*, const void*),
	       void (*DestroyKeyFunc) (void*),
	       void (*DestroyDataFunc) (void*))
{
  RBTree *newTree;
  RBNode *temp;
  
  newTree=(RBTree*) SafeMalloc (sizeof(RBTree));
  newTree->Compare = CompareFunc;
  newTree->DestroyKey = DestroyKeyFunc;
  newTree->DestroyData = DestroyDataFunc;
  
  /*  see the comment in the RBTree structure in rbtree.h */
  /*  for information on nil and root */
  temp=newTree->nil = (RBNode*) SafeMalloc (sizeof(RBNode));
  temp->parent = temp->left = temp->right = temp;
  temp->red = 0;
  temp->data = 0;
  temp->key = 0;
  
  temp = newTree->root = (RBNode*) SafeMalloc (sizeof(RBNode));
  temp->parent = temp->left = temp->right = newTree->nil;
  temp->key = 0;
  temp->data = 0;
  temp->red = 0;
  
  return(newTree);
}


/**********************************************************************
 *  FUNCTION:  LeftRotate
 *
 *  INPUTS:  This takes a tree so that it can access the appropriate
 *         root and nil pointers, and the node to rotate on.
 *
 *  OUTPUT:  None
 *
 *  Modifies Input: tree, x
 *
 *  EFFECTS:  Rotates as described in _Introduction_To_Algorithms by
 *            Cormen, Leiserson, Rivest (Chapter 14).  Basically this
 *            makes the parent of x be to the left of x, x the parent of
 *            its parent before the rotation and fixes other pointers
 *            accordingly.
 ***********************************************************************/
void
LeftRotate (RBTree *tree, RBNode *x)
{
  RBNode *y;
  RBNode *nil = tree->nil;
  
  y = x->right;
  x->right = y->left;
  
  if (y->left != nil) y->left->parent = x;
  
  y->parent = x->parent;
  
  /* instead of checking if x->parent is the root as in the book, we */
  /* count on the root sentinel to implicitly take care of this case */
  if ( x == x->parent->left )
    {
      x->parent->left = y;
    }
  else
    {
      x->parent->right = y;
    }

  y->left = x;
  x->parent = y;
}


/***********************************************************************
 *  FUNCTION:  RighttRotate
 *
 *  INPUTS:  This takes a tree so that it can access the appropriate
 *           root and nil pointers, and the node to rotate on.
 *
 *  OUTPUT:  None
 *
 *  Modifies Input?: tree, y
 *
 *  EFFECTS:  Rotates as described in _Introduction_To_Algorithms by
 *            Cormen, Leiserson, Rivest (Chapter 14).  Basically this
 *            makes the parent of x be to the left of x, x the parent of
 *            its parent before the rotation and fixes other pointers
 *            accordingly.
 ***********************************************************************/
void
RightRotate (RBTree *tree, RBNode *y)
{
  RBNode *x;
  RBNode *nil = tree->nil;
  
  x = y->left;
  y->left = x->right;

  if (nil != x->right)  x->right->parent = y;
  
  x->parent = y->parent;

  /* instead of checking if x->parent is the root as in the book, we */
  /* count on the root sentinel to implicitly take care of this case */
  if ( y == y->parent->left )
    {
      y->parent->left = x;
    }
  else
    {
      y->parent->right = x;
    }

  x->right = y;
  y->parent = x;
}


/***********************************************************************
 *  FUNCTION:  TreeInsertHelp
 *
 *  INPUTS:  tree is the tree to insert into and z is the node to insert.
 *
 *  OUTPUT:  none
 *
 *  Modifies Input:  tree, z
 *
 *  EFFECTS:  Inserts z into the tree as if it were a regular binary tree
 *            using the algorithm described in _Introduction_To_Algorithms_
 *            by Cormen et al.  This funciton is only intended to be called
 *            by the RBTreeInsert function and not by the user.
 ***********************************************************************/
void
TreeInsertHelp (RBTree *tree, RBNode *z)
{
  /*  This function should only be called by InsertRBTree (see above) */
  RBNode *x;
  RBNode *y;
  RBNode *nil = tree->nil;
  
  z->left = z->right=nil;
  y = tree->root;
  x = tree->root->left;

  while( x != nil)
    {
      y = x;
      if (1 == tree->Compare (x->key, z->key)) /* x.key > z.key */
	{
	  x = x->left;
	}
      else /* x,key <= z.key */
	{
	  x = x->right;
	}
    }

  z->parent = y;
 
  if ( (y == tree->root) ||
       (1 == tree->Compare (y->key, z->key)))  /* y.key > z.key */
    {
      y->left = z;
    } 
  else
    {
      y->right = z;
    }
}


/***********************************************************************
 *  FUNCTION:  RBTreeInsert
 *
 *  INPUTS:  tree is the red-black tree to insert a node which has a key
 *           pointed to by key and data pointed to by data.
 *
 *  OUTPUT:  This function returns a pointer to the newly inserted node
 *           which is guaranteed to be valid until this node is deleted.
 *           What this means is if another data structure stores this
 *           pointer then the tree does not need to be searched when this
 *           is to be deleted.
 *
 *  Modifies Input: tree
 *
 *  EFFECTS: Creates a node node which contains the appropriate key and
 *           data pointers and inserts it into the tree.
 ***********************************************************************/
RBNode *
RBTreeInsert (RBTree *tree, void *key, void *data)
{
  RBNode *y;
  RBNode *x;
  RBNode *newNode;

  x = (RBNode*) SafeMalloc (sizeof(RBNode));
  x->key = key;
  x->data = data;

  TreeInsertHelp (tree, x);
  newNode = x;
  x->red = 1;

  while (x->parent->red) /* use sentinel instead of checking for root */
    {
      if (x->parent == x->parent->parent->left)
	{
	  y = x->parent->parent->right;
	  if (y->red)
	    {
	      x->parent->red = 0;
	      y->red = 0;
	      x->parent->parent->red = 1;
	      x = x->parent->parent;
	    }
	  else
	    {
	      if (x == x->parent->right)
		{
		  x = x->parent;
		  LeftRotate (tree, x);
		}
	      x->parent->red = 0;
	      x->parent->parent->red = 1;
	      RightRotate (tree, x->parent->parent);
	    }
	}
      else /* case for x->parent == x->parent->parent->right */
	{
	  y = x->parent->parent->left;
	  if (y->red)
	    {
	      x->parent->red = 0;
	      y->red = 0;
	      x->parent->parent->red = 1;
	      x = x->parent->parent;
	    }
	  else
	    {
	      if (x == x->parent->left)
		{
		  x = x->parent;
		  RightRotate (tree, x);
		}
	      x->parent->red = 0;
	      x->parent->parent->red = 1;
	      LeftRotate (tree, x->parent->parent);
	    }
	}
    }
  
  tree->root->left->red = 0;
  
  return (newNode);
}


/***********************************************************************
 *  FUNCTION:  RBFind
 *
 *    INPUTS:  tree is the tree to search and key is a pointer to the key
 *             we are searching for.
 *
 *    OUTPUT:  returns the a node with key equal to key.  If there are
 *             multiple nodes with key equal to key this function
 *             returns the one highest in the tree.  If no nodes have
 *             a matching key 0 is returned.
 *
 *    Modifies Input: none
 ***********************************************************************/
RBNode*
RBFind (RBTree *tree, void *key)
{
  RBNode *x = tree->root->left;
  RBNode *nil = tree->nil;
  int compVal;

  if ( x == nil ) return (0);
  
  compVal = tree->Compare (x->key, key);
  
  while ( compVal != 0 )  /*assignemnt*/
    {
      if ( compVal == 1 ) /* x->key > key */
	{
	  x = x->left;
	}
      else
	{
	  x = x->right;
	}
      
      if ( x == nil) return(0);
      
      compVal = tree->Compare (x->key, key);
    }

  return (x);
}


/***********************************************************************
 *  FUNCTION:  TreeSuccessor 
 *
 *    INPUTS:  tree is the tree in question, and node is the node we want the
 *             the successor of.
 *
 *    OUTPUT:  This function returns the successor of node or NULL if no
 *             successor exists.
 *
 *    Modifies Input: none
 *
 *    Note:  uses the algorithm in _Introduction_To_Algorithms_
 ***********************************************************************/
RBNode *
TreeSuccessor (RBTree *tree, RBNode *node)
{
  RBNode *y;
  RBNode *nil = tree->nil;
  RBNode *root = tree->root;
  
  if (nil != (y = node->right)) /* assignment to y is intentional */
    {
      while (y->left != nil)  /* returns the minium of the right subtree of node */
	{
	  y = y->left;
	}
      return (y);
    }
  else
    {
      y = node->parent;
      
      while (node == y->right)   /* sentinel used instead of checking for nil */
	{
	  node = y;
	  y = y->parent;
	}
      
      if (y == root) return (nil);
      
      return(y);
    }
}


/***********************************************************************
 *  FUNCTION:  TreePredecessor 
 *
 *    INPUTS:  tree is the tree in question, and node is the node we want the
 *             the predecessor of.
 *
 *    OUTPUT:  This function returns the predecessor of node or NULL if no
 *             predecessor exists.
 *
 *    Modifies Input: none
 *
 *    Note:  uses the algorithm in _Introduction_To_Algorithms_
 ***********************************************************************/
RBNode *
TreePredecessor (RBTree *tree, RBNode *node)
{
  RBNode *y;
  RBNode *nil = tree->nil;
  RBNode *root = tree->root;
  
  if (nil != (y = node->left)) /* assignment to y is intentional */
    {
      while (y->right != nil)  /* returns the maximum of the left subtree of node */
	{
	  y = y->right;
	}

      return(y);
    }
  else
    {
      y = node->parent;

      while (node == y->left)
	{ 
	  if (y == root) return (nil); 
	  node = y;
	  y = y->parent;
	}

      return(y);
    }
}


/***********************************************************************
 *  FUNCTION:  TreeDestHelper
 *
 *    INPUTS:  tree is the tree to destroy and node is the current node
 *
 *    OUTPUT:  none 
 *
 *    EFFECTS:  This function recursively destroys the nodes of the tree
 *              postorder using the DestroyKey and DestroyData functions.
 *
 *    Modifies Input: tree, node
 *
 *    Note:    This function should only be called by RBTreeDestroy
 ***********************************************************************/
void
TreeDestHelper (RBTree *tree, RBNode *node)
{
  RBNode *nil = tree->nil;
  
  if (node != nil)
    {
      TreeDestHelper (tree, node->left);
      TreeDestHelper (tree, node->right);
      tree->DestroyKey (node->key);
      tree->DestroyData (node->data);
      free (node);
    }
}


/***********************************************************************
 *  FUNCTION:  RBTreeDestroy
 *
 *    INPUTS:  tree is the tree to destroy
 *
 *    OUTPUT:  none
 *
 *    EFFECT:  Destroys the key and frees memory
 *
 *    Modifies Input: tree
 ***********************************************************************/
void
RBTreeDestroy (RBTree *tree)
{
  TreeDestHelper (tree, tree->root->left);
  free (tree->root);
  free (tree->nil);
  free (tree);
}


/***********************************************************************
 *  FUNCTION:  RBDeleteFixUp
 *
 *    INPUTS:  tree is the tree to fix and node is the child of the spliced
 *             out node in RBDelete.
 *
 *    OUTPUT:  none
 *
 *    EFFECT:  Performs rotations and changes colors to restore red-black
 *             properties after a node is deleted
 *
 *    Modifies Input: tree, node
 *
 *    The algorithm from this function is from _Introduction_To_Algorithms_
 ***********************************************************************/
void
RBDeleteFixUp (RBTree *tree, RBNode *node)
{
  RBNode *root = tree->root->left;
  RBNode *w;
  
  while ( (!node->red) && (node != root) )
    {
      if (node == node->parent->left)
	{
	  w = node->parent->right;
	  if (w->red)
	    {
	      w->red = 0;
	      node->parent->red = 1;
	      LeftRotate (tree, node->parent);
	      w = node->parent->right;
	    }
	  
	  if ( (!w->right->red) && (!w->left->red) )
	    { 
	      w->red = 1;
	      node = node->parent;
	    }
	  else
	    {
	      if ( !w->right->red )
		{
		  w->left->red = 0;
		  w->red = 1;
		  RightRotate (tree, w);
		  w = node->parent->right;
		}
	      
	      w->red = node->parent->red;
	      node->parent->red = 0;
	      w->right->red = 0;
	      LeftRotate (tree, node->parent);
	      node = root; /* this is to exit while loop */
	    }
	}
      else /* the code below is has left and right switched from above */
	{
	  w = node->parent->left;
	  if (w->red)
	    {
	      w->red = 0;
	      node->parent->red = 1;
	      RightRotate (tree, node->parent);
	      w = node->parent->left;
	    }
	  
	  if ( (!w->right->red) && (!w->left->red) )
	    { 
	      w->red = 1;
	      node = node->parent;
	    }
	  else
	    {
	      if ( !w->left->red )
		{
		  w->right->red = 0;
		  w->red = 1;
		  LeftRotate (tree, w);
		  w = node->parent->left;
		}

	      w->red = node->parent->red;
	      node->parent->red = 0;
	      w->left->red = 0;
	      RightRotate (tree, node->parent);
	      node = root; /* this is to exit while loop */
	    }
	}
    }

  node->red=0;
}


/***********************************************************************
 *  FUNCTION:  RBDelete
 *
 *    INPUTS:  tree is the tree to delete node from
 *
 *    OUTPUT:  none
 *
 *    EFFECT:  Deletes node from tree and frees the key and data of
 *             node using DestoryKey and DestoryData.  Then calls
 *             RBDeleteFixUp to restore red-black properties
 *
 *    Modifies Input: tree, node
 *
 *    The algorithm from this function is from _Introduction_To_Algorithms_
 ***********************************************************************/
void
RBDelete (RBTree *tree, RBNode *node)
{
  RBNode *y;
  RBNode *x;
  RBNode *nil = tree->nil;
  RBNode *root = tree->root;
  
  y = ((node->left == nil) || (node->right == nil)) ? node : TreeSuccessor (tree, node);
  x = (y->left == nil) ? y->right : y->left;

  if (root == (x->parent = y->parent))  /* assignment of y->p to x->p is intentional */
    {
      root->left = x;
    }
  else
    {
      if (y == y->parent->left)
	{
	  y->parent->left = x;
	}
      else
	{
	  y->parent->right = x;
	}
    }

  if (y != node) /* y should not be nil in this case */
    {
      /* y is the node to splice out and x is its child */
      
      if (!(y->red)) RBDeleteFixUp (tree, x);
      
      tree->DestroyKey (node->key);
      tree->DestroyData (node->data);
      y->left = node->left;
      y->right = node->right;
      y->parent = node->parent;
      y->red = node->red;
      node->left->parent = node->right->parent=y;

      if (node == node->parent->left)
	{
	  node->parent->left = y;
	}
      else
	{
	  node->parent->right = y;
	}
      free (node); 
    }
  else
    {
      tree->DestroyKey (y->key);
      tree->DestroyData (y->data);

      if (!(y->red)) RBDeleteFixUp (tree, x);

      free (y);
    }
}


/***********************************************************************
 *  FUNCTION: RBTraverseHelper and RBTraverse
 *
 *  Traverse a tree in order (left|root|right) and run the passed
 *  function NodeFunc for every node.
 ***********************************************************************/
void
RBTraverseHelper (RBTree *tree, RBNode *node, void (*NodeFunc)(RBNode*))
{
  if (node != tree->nil)
    {
      RBTraverseHelper (tree, node->left, NodeFunc);
      
      NodeFunc (node);
      
      RBTraverseHelper (tree, node->right, NodeFunc);
    }
}

void
RBTraverse (RBTree *tree, void (*NodeFunc)(RBNode*))
{
  RBTraverseHelper (tree, tree->root->left, NodeFunc);
}


/***********************************************************************
 *  FUNCTION: RBBuildStackHelper and RBBuildStack
 *
 *  Traverse the tree in reverse order (right|root|left) and build a
 *  Stack of all entries.
 ***********************************************************************/
void
RBBuildStackHelper (RBTree *tree, RBNode *node, Stack *stack)
{
  if (node != tree->nil)
    {
      RBBuildStackHelper (tree, node->right, stack);
      
      StackPush (stack, node);
      
      RBBuildStackHelper (tree, node->left, stack);
    }
}

void
RBBuildStack (RBTree *tree, Stack *stack)
{
  RBBuildStackHelper (tree, tree->root->left, stack);
}


/***********************************************************************
 *  FUNCTION:  RBTreePrintNode
 *
 *    INPUTS:  tree is the tree to print and node is the current inorder node
 *
 *    OUTPUT:  none 
 *
 *    EFFECTS: This function recursively prints the nodes of the tree
 *             inorder using the PrintKey and PrintData functions.
 *
 *    Modifies Input: none
 *
 *    Note:    This function should only be called from RBTreePrint
 ***********************************************************************/
void
RBTreePrintNode (RBTree *tree, RBNode *node,
		 void (*PrintKey)(void*),
		 void (*PrintData)(void*))
{
  RBNode *nil = tree->nil;
  RBNode *root = tree->root;
  
  if (node != tree->nil)
    {
      RBTreePrintNode (tree, node->left, PrintKey, PrintData);
      printf("data=");
      PrintData (node->data);
      printf("  key=");
      PrintKey (node->key);
      printf("  l->key=");
      if ( node->left == nil) printf("NULL"); else PrintKey (node->left->key);
      printf("  r->key=");
      if ( node->right == nil) printf("NULL"); else PrintKey (node->right->key);
      printf("  p->key=");
      if ( node->parent == root) printf("NULL"); else PrintKey (node->parent->key);
      printf("  red=%i\n", node->red);
      RBTreePrintNode (tree, node->right, PrintKey, PrintData);
    }
}


/***********************************************************************
 *  FUNCTION:  RBTreePrint
 *
 *    INPUTS:  tree is the tree to print
 *
 *    OUTPUT:  none
 *
 *    EFFECT:  This function recursively prints the nodes of the tree 
 *             inorder using the PrintKey and PrintData functions.
 ***********************************************************************/
void
RBTreePrint(RBTree *tree,
	    void (*PrintKey)(void*),
	    void (*PrintData)(void*))
{
  RBTreePrintNode (tree, tree->root->left, PrintKey, PrintData);
}
