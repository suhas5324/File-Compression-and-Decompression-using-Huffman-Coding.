#include "Decompress.h"
#include "Compress.h"
static bool CreateFreqTable(FILE**);
static bool WriteFreqTableInFile();
static void PrintFreqTable();
static bool WriteCompressedFile(FILE**);
bool CompressTextFile()
{
  bool status = false;
  printf("\nThe file to be compressed must be a named Original.txt\n"
         "\nOriginal.txt must be stored in the folder named Original\n");
  printf("\nPress Enter to continue... ");
  while (getchar() != '\n');
  FILE* fp = fopen("Original/Original.txt", "r");
  if (!fp)
  {
    printf("Error! Original.txt could not be opened!\n");
    return false;
  }
  status = CreateFreqTable(&fp);
  PrintFreqTable();
  if (status)
    status = WriteFreqTableInFile() && LoadFreqTableInPQ();
  if (status)
  {
    status = CreateHuffmanTree();
    if (!status)
      PQ_DeletePQ();
  }
  if (status)
    status = EncodeTree(HuffmanTree);
  PrintEncodeTable();

  status = WriteCompressedFile(&fp);

  if (status)
    printf("File compressed and stored in 'Compressed' folder\n");

  fclose(fp);

  free(FrequencyTable);
  free(EncodeTable);

  FrequencyTable = NULL;
  EncodeTable    = NULL;

  return status;
}
static bool CreateFreqTable(FILE** fp)
{
  FILE* inputFP = *fp;

  if (!inputFP)
  {
    printf("Error! Null FP in CreateFreqTable\n");
    return false;
  }

  FREQ_TABLE_SIZE = 256;
  FrequencyTable  = NULL;
  FrequencyTable = (FreqTable_t*) malloc(sizeof(FreqTable_t) * FREQ_TABLE_SIZE);

  if (!FrequencyTable)
  {
    printf("Error allocating memory for FreqTable in CreateFreqTable.\n");
    return false;
  }
  for (int i = 0; i < FREQ_TABLE_SIZE; i++)
  {
    FrequencyTable[i].id   = -1;
    FrequencyTable[i].freq = 0;
  }

  int  buffer;
  int  currSize     = 0;
  bool containsChar = false;

  while (1)
  {
    buffer = fgetc(inputFP);

    if (ferror(inputFP))
    {
      printf("Error reading file.\n");
      return false;
    }
    if (feof(inputFP))
    {
      printf("Finished reading file.\n");
      break;
    }

    containsChar = false;
    for (int i = 0; i <= currSize; i++)
    {
      if (buffer == FrequencyTable[i].id)
      {
        FrequencyTable[i].freq++;
        containsChar = true;
        break;
      }
    }
    if (!containsChar)
    {
      FrequencyTable[currSize].id = buffer;
      FrequencyTable[currSize].freq++;
      currSize++;
      if (currSize >= FREQ_TABLE_SIZE)
      {
        FrequencyTable = (FreqTable_t*) realloc(FrequencyTable,
                                                sizeof(FreqTable_t) * 128);

        if (!FrequencyTable)
        {
          printf("Error allocating memory for FreqTable.\n");
          return false;
        }

        printf("Memory reallocated for FreqTable\n");

        FREQ_TABLE_SIZE+= 128;
      }
    }
  }

  FREQ_TABLE_SIZE = currSize;

  return true;
}
static bool WriteFreqTableInFile()
{
  if (!FrequencyTable)
  {
    printf("Error! Null pointer to Frequency Table\n");
    return false;
  }

  FILE* fp = fopen("Compressed/FreqTable.tab", "w");

  if (!fp)
  {
    printf("Freq Table file could not be opened\n");
    return false;
  }

  for (int i = 0; i < FREQ_TABLE_SIZE; i++)
    fprintf(fp, "%d %d\n", FrequencyTable[i].id, FrequencyTable[i].freq);

  fclose(fp);

  return true;
}
static void PrintFreqTable()
{
  int charCount = 0;

  for (int i = 0; i < FREQ_TABLE_SIZE; i++)
  {
    printf("\t%c\t%d\n", FrequencyTable[i].id,
                       FrequencyTable[i].freq);

    charCount += FrequencyTable[i].freq;
  }

  printf("**** Byte Count: %d ****\n", charCount);
}
static unsigned int BinToDec(char* code)
{
  if(!code)
    return -1;

  unsigned int decimal = 0;
  int power = 1;
  for (int i = 0; i < 8; i++)
  {
    decimal += power*(code[i] - '0');
    power *= 2;
  }

  return decimal;
}
static bool WriteCompressedFile(FILE** inpFP)
{
  FILE* inputFP = *inpFP;

  if (!inputFP)
  {
    printf("Error! NULL FP in WriteCompressedFile \n");
    return false;
  }

  if (!EncodeTable)
  {
    printf("Error! NULL EncodeTable in WriteCompressedFile \n");
    return false;
  }

  FILE* fp = fopen("Compressed/Compressed.huf", "w");

  if (!fp)
  {
    printf("Error! Could not create Compressed.huf\n");
    return false;
  }

  rewind(inputFP);

  unsigned int  readBuf, writeBuf;
  int  bitsWritten = 0, codeBitsWritten = 0;
  int  codeSizeBits;
  char code[128], byte[8];
  int  byteCount = 0;

  while (true)
  {
    if (codeBitsWritten == 0)
    {
      readBuf = fgetc(inputFP);

      for (int i = 0; i < FREQ_TABLE_SIZE && readBuf != EOF; i++)
      {
        if (EncodeTable[i].id == readBuf)
        {
          codeSizeBits = EncodeTable[i].codeSizeBits;
          for (int j = 0; j < codeSizeBits; j++)
            code[j] = EncodeTable[i].code[j];

          break;
        }
      }
    }
    while (bitsWritten < 8 && codeBitsWritten != codeSizeBits &&
           !(bitsWritten == 0 && readBuf == EOF))
    {
      if (readBuf == EOF)
        byte[bitsWritten] = '0';
      else
      {
        byte[bitsWritten] = code[codeBitsWritten];
        codeBitsWritten++;
      }

      bitsWritten++;
    }

    if (codeBitsWritten == codeSizeBits)
      codeBitsWritten = 0;

    if (bitsWritten == 8)
    {
      writeBuf = BinToDec(byte);
      fputc(writeBuf, fp);
      bitsWritten = 0;
      byteCount++;
    }

    if (readBuf == EOF)
      break;
  }

  printf("**** Bytes Written: %d ****\n", byteCount);
  printf("HELLO");

  fclose(fp);

  return true;
}
#include "Decompress.h"

static bool LoadFreqTableFromFile();
static bool DecompressFile();
bool DecompressHuffmanFile()
{
  bool status;

  printf("Compressed.huf and FreqTable.tab must be present "
         "in Compressed folder\n\n");
  printf("Press Enter to continue... ");
  while (getchar() != '\n');
  status = LoadFreqTableFromFile();
  if (status)
    status = LoadFreqTableInPQ();
  if (status)
  {
    status = CreateHuffmanTree();

    if (!status)
      PQ_DeletePQ();
  }
  if (status)
    status = DecompressFile();
  if (status)
    printf("File decompressed and stored in 'Decompressed' folder\n");
  free(FrequencyTable);
  free(HuffmanTree);

  FrequencyTable = NULL;
  HuffmanTree    = NULL;

  return status;
}
static bool LoadFreqTableFromFile()
{
  FREQ_TABLE_SIZE = 256;
  FrequencyTable = NULL;
  FrequencyTable = (FreqTable_t*) malloc(sizeof(FreqTable_t) * FREQ_TABLE_SIZE);

  if (!FrequencyTable)
  {
    printf("Error allocating memory for FreqTable.\n");
    return false;
  }

  FILE* fp = fopen("Compressed/FreqTable.tab", "r");

  if (!fp)
  {
    printf("Error opening FreqTable.tab\n");
    return false;
  }

  int currSize = 0;

  while (true)
  {
    fscanf(fp, "%d %d\n", &FrequencyTable[currSize].id,
                        &FrequencyTable[currSize].freq);

    currSize++;

    if (feof(fp))
      break;

    if (currSize >= FREQ_TABLE_SIZE)
    {
      FrequencyTable = (FreqTable_t*) realloc(FrequencyTable,
                                              sizeof(FreqTable_t) * 128);

      if (!FrequencyTable)
      {
        printf("Error allocating memory for FreqTable.\n");
        fclose(fp);
        return false;
      }

      printf("Memory reallocated for FreqTable\n");

      FREQ_TABLE_SIZE+= 128;
    }
  }

  FREQ_TABLE_SIZE = currSize;

  fclose(fp);

  return true;
}


int DecToBin(unsigned int decimal, char* byte)
{
  if (!byte)
    return -1;

  if (decimal > 255)
    return -1;

  for (int i = 0; i < 8; i++)
  {
    byte[i] = (decimal % 2) + '0';
    decimal = decimal/2;
  }

  return 0;
}
static bool DecompressFile()
{
  FILE  *decompFile, *compFile;
  int   totalCharCount = 0;      
  int   bitIndex = 0;
  unsigned int readBuf, writeBuf;
  char  byte[8];
  bool writeChar = false;

  if (!FrequencyTable)
  {
    printf("Frequency Table has not been created\n");
    return false;
  }

  for (int i = 0; i < FREQ_TABLE_SIZE; i++)
    totalCharCount += FrequencyTable[i].freq;


  if (!HuffmanTree)
  {
    printf("Huffman Tree has not been created\n");
    return false;
  }

  compFile = fopen("Compressed/Compressed.huf", "r");

  if (!compFile)
  {
    printf("Error opening compressed file.\n");
    return false;
  }

  decompFile = fopen("Decompressed/Decompressed.txt", "w");

  if (!decompFile)
  {
    printf("Error creating decompressed file.\n");
    return false;
  }

  node_t* current = HuffmanTree;

  while (true)
  {
 
    if (bitIndex == 0)
    {
      readBuf = fgetc(compFile);

      DecToBin(readBuf, byte);
    }

    while (bitIndex < 8)
    {
      if (byte[bitIndex] == '0')
        current = current->left;

      else if (byte[bitIndex] == '1')
        current = current->right;

      bitIndex++;

      // end of tree
      if (current->left == NULL)
      {
        writeBuf  = current->id;
        current   = HuffmanTree;
        writeChar = true;
        break;  
      }
    }

    if (writeChar)
    {
      fputc(writeBuf, decompFile);

      writeChar = false;

      if (--totalCharCount == 0)
        break;
    }

    if (bitIndex == 8)
      bitIndex = 0;
  }

  fclose(compFile);
  fclose(decompFile);

  return true;
}
#include "Huffman.h"
static PQ_t* PQ_Add (node_t newNode)
{
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
static bool PQ_Pop(node_t* poppedNode)
{
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
bool CreateHuffmanTree()
{
  node_t tempRight, tempLeft;

  node_t internalNode;
  node_t *leftNode, *rightNode;

  while (PQ_Pop(&tempLeft) && PQ_Pop(&tempRight))
  {
    leftNode  = (node_t*) malloc(sizeof(node_t));
    rightNode = (node_t*) malloc(sizeof(node_t));

    if (!leftNode || !rightNode)
    {
      printf("Error allocating memory for nodes in Huffman Tree\n");
      return false;
    }
    *leftNode   = tempLeft;
    *rightNode  = tempRight;

    internalNode.id   = -1;
    internalNode.freq = leftNode->freq + rightNode->freq;

    internalNode.left  = leftNode;
    internalNode.right = rightNode;

    if (!PQ_Add(internalNode))
      return false;
  }
  HuffmanTree = (node_t*) malloc(sizeof(node_t));

  if (!HuffmanTree)
  {
    printf("Error Allocating memory for start index");
    return false;
  }

  *HuffmanTree = internalNode;

  return true;
}
bool EncodeTree (node_t* nodePtr)
{
  if(!nodePtr)
    return false;

  static bool memAlloc = true;
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
    free(nodePtr);
    return true;
  }
  bitIndex++;
  codes[bitIndex] = '0';

  EncodeTree(nodePtr->left);
  bitIndex++;
  codes[bitIndex] = '1';

  EncodeTree(nodePtr->right);
  bitIndex--;
  free(nodePtr);
  return true;
}
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
int main()
{
  char input = 0;
  printf("The program compresses/decompresses text files using Huffman Coding\n\n");
  while (input != 'C' && input != 'D')
  {
    printf("Do you want to compress (C) or decompress(D)? > ");
    scanf("%c", &input);
    fflush(stdin);
  }
  if (input == 'C')
    CompressTextFile();
  else
    DecompressHuffmanFile();
  printf("\nPress Enter to exit...");
  while(getchar() != '\n');
  return 0;
}
