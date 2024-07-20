#ifndef _huffman_h_
#define _huffman_h_

#include <stdio.h>
#include <stdlib.h>

typedef enum
{
  false = 0,
  true  = 1
}bool;

typedef struct FreqTable
{
  int  id;
  int  freq;

}FreqTable_t;

typedef struct node
{
  int id;
  int freq;

  struct node* left;
  struct node* right;
}node_t;

typedef struct PQ
{
  node_t node;
  struct PQ* next;
}PQ_t;

typedef struct code
{
  int    id;
  char   code[128];          //the code is in binary
  int    codeSizeBits;

}code_t;

int           FREQ_TABLE_SIZE;
FreqTable_t*  FrequencyTable;
PQ_t*         PriorityQueue;
node_t*       HuffmanTree;
code_t*       EncodeTable;


/*****************************************************
* Function to load the frequency table to the PQ
* Deletes the frequency table once loaded
******************************************************/
bool LoadFreqTableInPQ();

/*****************************************************
* Function to create the Huffman Tree
******************************************************/
bool CreateHuffmanTree();

/*****************************************************
* Traverse the Huffman tree and stores the code in the table
* Deletes the Huffman tree
******************************************************/
bool EncodeTree(node_t* nodePtr);

/*****************************************************
* Prints the encoded table
******************************************************/
void PrintEncodeTable();

/*****************************************************
* Function to delete the queue
******************************************************/
void PQ_DeletePQ();

#endif // _encode_h_
