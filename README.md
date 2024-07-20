<b>PROJECT AIM</b>
<br>
The aim of this project is to implement a file compression and decompression system using Huffman Coding in C. Huffman Coding is a widely used algorithm for lossless data compression. The project involves creating a program that can compress a given file by reducing its size without losing any information and decompress it back to its original form.

<b>PROJECT OVERVIEW</b>
<br>
This project involves the following key components:

Building the Huffman Tree: This is a binary tree used for encoding data where the frequency of characters determines the structure of the tree.
<br>
Generating Huffman Codes: Based on the Huffman Tree, each character in the file is assigned a unique binary code.
<br>
Compressing the File: The original file is read, and each character is replaced with its corresponding Huffman code to create the compressed file.
<br>
Decompressing the File: The compressed file is read, and the Huffman codes are translated back to the original characters using the Huffman Tree.

