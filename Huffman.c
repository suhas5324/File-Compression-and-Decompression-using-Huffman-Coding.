#include "Huffman.h"


/*****************************************************
* Function to add nodes to the priority queue
* The nodes are stored in a ascending order of the freq
* Returns the address of the new node added in the queue
* Returns NULL if the function fails
******************************************************/
static PQ_t* PQ_Add (node_t newNode)
{
  // if the head is null, then create the head
  if (!PriorityQueue)
  {
    PriorityQueue = (PQ_t*) malloc(sizeof(PQ_t));

    if (!PriorityQueue)
    {
      printf("Error allocating memory for PQ Node.\n");
      return NULL;
    }

    PriorityQueue->node = newNode;
    PriorityQueue->next = NULL;

    return PriorityQueue;
  }

  //else, traverse the list and find a place to add the node
  PQ_t *previous, *current, *newQueue;
  node_t tempNode;

  current = PriorityQueue;

  while (current->next != NULL)
  {
    previous  = current;
    current   = current->next;

    if (previous->node.freq <= newNode.freq &&
        current->node.freq  >  newNode.freq)
    {
      newQueue  = (PQ_t*) malloc(sizeof(PQ_t));

      if (!newQueue)
      {
        printf("Error allocating memory for PQ Node.\n");
        return NULL;
      }

      newQueue->node = newNode;
      newQueue->next = current;
      previous->next = newQueue;

      return newQueue;
    }
  }

  /* As the function didn't return in the while loop,
   * then it implies that the index has reached the end.
   * Therefore, handle the edge case
  */

  // allocate memory for the new node
  newQueue = (PQ_t*) malloc(sizeof(PQ_t));

  if (!newQueue)
  {
    printf("Error allocating memory for PQ Node.\n");
    return NULL;
  }

  current->next   = newQueue;
  newQueue->next  = NULL;

  if (newNode.freq >= current->node.freq)
  {
     newQueue->node = newNode;
     return newQueue;
  }

  else
  {
    tempNode        = current->node;
    current->node   = newNode;
    newQueue->node  = tempNode;

    return current;
  }
}

/*****************************************************
* Function to pop a node from the queue
* The lowest freq node is always returned
* The node is deleted from the queue
******************************************************/
static bool PQ_Pop(node_t* poppedNode)
{
  // if the list is empty, returns NULL
  if (!PriorityQueue)
  {
    poppedNode = NULL;
    return false;
  }

  PQ_t* next   = PriorityQueue->next;
  *poppedNode  = PriorityQueue->node;

  free(PriorityQueue);

  PriorityQueue = next;

  return true;
}

/*****************************************************
* Function to delete the queue
******************************************************/
void PQ_DeletePQ()
{
  PQ_t* current = PriorityQueue;
  PQ_t* next    = NULL;

  while (current != NULL)
  {
    next = current->next;

    free(current);
    current = next;
  }
}

/*****************************************************
* Function to load the frequency table to the PQ
******************************************************/
bool LoadFreqTableInPQ()
{
  node_t node;

  node.left   = NULL;
  node.right  = NULL;

  for (int i = 0; i < FREQ_TABLE_SIZE; i++)
  {
    node.id   = FrequencyTable[i].id;
    node.freq = FrequencyTable[i].freq;

    if (!PQ_Add(node))
    {
      printf("Error loading FreqTable in PQ.\n");
      free(FrequencyTable);
      return false;
    }
  }

  return true;
}

/*****************************************************
* Function to create the Huffman Tree
* The Priority Queue is deleted as PQ_Pop is called
******************************************************/
bool CreateHuffmanTree()
{
  node_t tempRight, tempLeft;

  node_t internalNode;
  node_t *leftNode, *rightNode;

  while (PQ_Pop(&tempLeft) && PQ_Pop(&tempRight))
  {
    //allocate memory for left and right nodes
    leftNode  = (node_t*) malloc(sizeof(node_t));
    rightNode = (node_t*) malloc(sizeof(node_t));

    if (!leftNode || !rightNode)
    {
      printf("Error allocating memory for nodes in Huffman Tree\n");
      return false;
    }

    // copy the temp nodes to the dynamically allocated nodes
    *leftNode   = tempLeft;
    *rightNode  = tempRight;

    internalNode.id   = -1;
    internalNode.freq = leftNode->freq + rightNode->freq;

    internalNode.left  = leftNode;
    internalNode.right = rightNode;

    if (!PQ_Add(internalNode))
      return false;
  }

  //allocate memory for the start node
  HuffmanTree = (node_t*) malloc(sizeof(node_t));

  if (!HuffmanTree)
  {
    printf("Error Allocating memory for start index");
    return false;
  }

  *HuffmanTree = internalNode;

  return true;
}

/*****************************************************
* Traverse the Huffman tree and print codes
* Deletes the Huffman tree
******************************************************/
bool EncodeTree (node_t* nodePtr)
{
  //if NULL pointer, return
  if(!nodePtr)
    return false;

  static bool memAlloc = true;

  // allocate memory for encode table
  if (memAlloc)
  {
    EncodeTable = (code_t*) malloc(sizeof(code_t) * FREQ_TABLE_SIZE);

    if (!EncodeTable)
    {
      printf("Error allocating memory for encode table\n");
      return false;
    }

    memAlloc = false;
  }

  static int  bitIndex = -1;
  static int  arrayIndex = 0;
  static char codes[128];

  if (nodePtr->id != -1)
  {
    for (int i = 0; i <= bitIndex; i++)
      EncodeTable[arrayIndex].code[i] = codes[i];

    //store the code in the encode table
    EncodeTable[arrayIndex].id            = nodePtr->id;
    EncodeTable[arrayIndex].codeSizeBits  = bitIndex + 1;

    bitIndex--;
    arrayIndex++;

    // delete this node
    free(nodePtr);

    // returns from a terminal node containing a character
    return true;
  }

  //left traversal
  bitIndex++;
  codes[bitIndex] = '0';

  EncodeTree(nodePtr->left);


  //right traversal
  bitIndex++;
  codes[bitIndex] = '1';

  EncodeTree(nodePtr->right);

  // returns from an internal node with no id
  bitIndex--;

  // delete this node
  free(nodePtr);

  return true;
}

/*****************************************************
* Prints the encoded table
******************************************************/
void PrintEncodeTable()
{
  if (!EncodeTable)
    return;

  for (int i = 0; i < FREQ_TABLE_SIZE; i++)
  {
    printf("%c ", EncodeTable[i].id);

    for (int j = 0; j < EncodeTable[i].codeSizeBits; j++)
      printf("%c", EncodeTable[i].code[j]);

    printf("\n");
  }
}

