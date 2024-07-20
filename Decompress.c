#include "Decompress.h"

///////////////////////Function Prototypes///////////////////////////
/////////////////////////////////////////////////////////////////////
static bool LoadFreqTableFromFile();
static bool DecompressFile();

/*****************************************************
* Function to decompress a huffman file
* Compressed.huf and FreqTable.tab must be present
* in the folder Decompressed
******************************************************/
bool DecompressHuffmanFile()
{
  bool status;

  printf("Compressed.huf and FreqTable.tab must be present "
         "in Compressed folder\n\n");
  printf("Press Enter to continue... ");
  while (getchar() != '\n');

  /* Load the frequency table from the saved file*/
  status = LoadFreqTableFromFile();

  /* Load the frequency table in the priority Queue*/
  if (status)
    status = LoadFreqTableInPQ();

  /*Create the Huffman Tree*/
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

///////////////////////Private Functions////////////////////////////
////////////////////////////////////////////////////////////////////
/*****************************************************
* Function to load frequency table from a file
******************************************************/
static bool LoadFreqTableFromFile()
{
  FREQ_TABLE_SIZE = 256;
  FrequencyTable = NULL;

  // Dynamically allocate memory for the table
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

    // if the number of characters exceeds the allocated number, resize the array
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

/*****************************************************
* Function to convert decimal to 8 bit binary
********************************************************/
int DecToBin(unsigned int decimal, char* byte)
{
  if (!byte)
    return -1;

  // check if the decimal is less than 255
  if (decimal > 255)
    return -1;

  for (int i = 0; i < 8; i++)
  {
    byte[i] = (decimal % 2) + '0';
    decimal = decimal/2;
  }

  return 0;
}

/*****************************************************
* Function to create and decompress the original file
* Requires freq table, huffman tree
******************************************************/
static bool DecompressFile()
{
  FILE  *decompFile, *compFile;
  int   totalCharCount = 0;           // total char count in the original file
  int   bitIndex = 0;
  unsigned int readBuf, writeBuf;
  char  byte[8];
  bool writeChar = false;

  // Check if FreqTable has been loaded
  if (!FrequencyTable)
  {
    printf("Frequency Table has not been created\n");
    return false;
  }

  // calculate the total char in the original file
  for (int i = 0; i < FREQ_TABLE_SIZE; i++)
    totalCharCount += FrequencyTable[i].freq;

  // Check if the huffman tree has been created
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
    //read a char if the current byte has been decoded
    if (bitIndex == 0)
    {
      readBuf = fgetc(compFile);

      DecToBin(readBuf, byte);
    }

    //traverse the Huffman tree
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
        break;  //break the loop
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
