#include "Compress.h"

///////////////////////Function Prototypes///////////////////////////
/////////////////////////////////////////////////////////////////////
static bool CreateFreqTable(FILE**);
static bool WriteFreqTableInFile();
static void PrintFreqTable();
static bool WriteCompressedFile(FILE**);

/*****************************************************
* Function to compress a text file
* The file to be compressed must be named Original.txt
* and stored in the Original folder of the project
******************************************************/
bool CompressTextFile()
{
  bool status = false;

  printf("\nThe file to be compressed must be a named Original.txt\n"
         "\nOriginal.txt must be stored in the folder named Original\n");
  printf("\nPress Enter to continue... ");
  while (getchar() != '\n');

  /*Open the original file*/
  FILE* fp = fopen("Original/Original.txt", "r");

  if (!fp)
  {
    printf("Error! Original.txt could not be opened!\n");
    return false;
  }

  /*Create the Frequency Table*/
  status = CreateFreqTable(&fp);

  PrintFreqTable();

  /*Save the Frequency Table in a file*/
  /*Load the Frequency Table in Priority Queue*/
  if (status)
    status = WriteFreqTableInFile() && LoadFreqTableInPQ();

  /*Create the Huffman Tree*/
  if (status)
  {
    status = CreateHuffmanTree();

    if (!status)
      PQ_DeletePQ();
  }

  /*Create the Encode Table*/
  /* Huffman tree is deleted in this function*/
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

///////////////////////Private Functions////////////////////////////
////////////////////////////////////////////////////////////////////
/*****************************************************
* Function to count and store the frequencies of different characters
* Argument: reference to the input file pointer
* Returns true if successful
* Returns false otherwise
******************************************************/
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

  // Dynamically allocate memory for the table
  FrequencyTable = (FreqTable_t*) malloc(sizeof(FreqTable_t) * FREQ_TABLE_SIZE);

  if (!FrequencyTable)
  {
    printf("Error allocating memory for FreqTable in CreateFreqTable.\n");
    return false;
  }

  //initialize the frequency table
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
    // Read a character from the file
    buffer = fgetc(inputFP);

    if (ferror(inputFP))
    {
      printf("Error reading file.\n");
      return false;
    }

    //check if end of file
    if (feof(inputFP))
    {
      printf("Finished reading file.\n");
      break;
    }

    containsChar = false;

    //iterate through the array and fill up the table
    for (int i = 0; i <= currSize; i++)
    {
      if (buffer == FrequencyTable[i].id)
      {
        FrequencyTable[i].freq++;
        containsChar = true;
        break;
      }
    }

    // if the character hasn't been stored before, store it at the end of the array
    if (!containsChar)
    {
      FrequencyTable[currSize].id = buffer;
      FrequencyTable[currSize].freq++;

      currSize++;

      // if the number of characters exceeds the allocated number, resize the array
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

/*****************************************************
* Function to write frequency table in a file
******************************************************/
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

/*****************************************************
* Function to print the frequency table
******************************************************/
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


/*****************************************************
* Function to convert 8 bit binary to decimal
********************************************************/
static unsigned int BinToDec(char* code)
{
  if(!code)
    return -1;

  unsigned int decimal = 0;
  int power = 1;

  //ASCII values for '0' = 48, '1' = 49
  for (int i = 0; i < 8; i++)
  {
    decimal += power*(code[i] - '0');
    power *= 2;
  }

  return decimal;
}


/*******************************************************
* Function to close the opened file
********************************************************/
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
      // read a char from the file
      readBuf = fgetc(inputFP);

      for (int i = 0; i < FREQ_TABLE_SIZE && readBuf != EOF; i++)
      {
        if (EncodeTable[i].id == readBuf)
        {
          codeSizeBits = EncodeTable[i].codeSizeBits;

          // Copy the code
          for (int j = 0; j < codeSizeBits; j++)
            code[j] = EncodeTable[i].code[j];

          break;
        }
      }
    }

    /* The last condition handles the case when the coded file
     * files a byte completely and no extra 0's need to be added.
     * bitsWritten = 0 implies that one complete byte has been written
     */
    while (bitsWritten < 8 && codeBitsWritten != codeSizeBits &&
           !(bitsWritten == 0 && readBuf == EOF))
    {
      // Fill the rest of the byte with zeros
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

  fclose(fp);

  return true;
}

