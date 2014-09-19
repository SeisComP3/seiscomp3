/***************************************************************************
 * stack.c:
 * 
 * Simple stack routines.
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
#include "stack.h"

Stack *
StackCreate()
{
  Stack * newStack;
  
  newStack = (Stack *) malloc (sizeof(Stack));
  newStack->top = newStack->tail=NULL;
  return (newStack);
}

void
StackPush (Stack * theStack, STACK_DATA_TYPE newDataPtr)
{
  StackNode * newNode;

  if (!theStack->top)
    {
      newNode = (StackNode *) malloc (sizeof(StackNode));
      newNode->data = newDataPtr;
      newNode->next = theStack->top;
      theStack->top = newNode;
      theStack->tail = newNode;
    }
  else
    {
      newNode = (StackNode *) malloc (sizeof(StackNode));
      newNode->data = newDataPtr;
      newNode->next = theStack->top;
      theStack->top = newNode;
    }
}

STACK_DATA_TYPE
StackPop (Stack * theStack)
{
  STACK_DATA_TYPE popData;
  StackNode * oldNode;

  if (theStack->top)
    {
      popData = theStack->top->data;
      oldNode = theStack->top;
      theStack->top = theStack->top->next;
      free (oldNode);

      if (!theStack->top) theStack->tail = NULL;
    }
  else
    {
      popData = NULL;
    }

  return (popData);
}

void
StackDestroy (Stack * theStack, void DestFunc(void * a))
{
  StackNode * x = theStack->top;
  StackNode * y;
  
  if (theStack)
    {
      while (x)
	{
	  y = x->next;
	  DestFunc (x->data);
	  free (x);
	  x = y;
	}
      free (theStack);
    }
}

int
StackNotEmpty (Stack * theStack)
{
  return( theStack ? 1 : 0);
}

Stack *
StackJoin (Stack * stack1, Stack * stack2)
{
  if ( !stack1->tail )
    {
      free (stack1);
      return (stack2);
    }
  else
    {
      stack1->tail->next = stack2->top;
      stack1->tail = stack2->tail;
      free (stack2);
      return (stack1);
    }
}
