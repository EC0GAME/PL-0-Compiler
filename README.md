# PL-0-Compiler
Recursive Decent Parser &amp; Intermediate Code Generator

First, my file is compiled with a gcc compiler, and then the a.out file is ran after the source code is turned into an executable.
All the input is included in the input text file, "ELF.txt", which has lines of source code to be taken in by the code in "parsercodegen.c".
It takes in file input character by character until it reaches the end of the file. The program will have special cases for certain 
symbols, phrases, and declarations. Once the program recognizes what it is taking in as input, it creates a token to be saved in the
lexeme table, which I simulated using an array. As the program takes in more characters as input, it keeps creating more tokens until 
the end of the file. When the program is done simulating a lexical analyzer (code from HW2), the next portion of the code will call operations
to simulate the PL/0 compiler. The code goes through the created lexeme table, in order, and performs a series of functions to compile the input
and process whether or not there are errors in the code. Functions were simulated in the code using actual methods in C, called STATEMENT, EXPRESSION, CONDITION, TERM, and FACTOR. These methods run recursively depending on the token input. Furthermore, there is a method created to find the index 
of a symbol in the symbol table. While running through the list of tokens, there are 15 errors in total that are all checked in the code and 
tested with individual input files named "errorin#". Input files are utilized exactly like "ELF.txt" but for errors. When an error is found, 
the program prints the error it found and closes the program. If there are no errors in the input, then the program finishes by printing the 
assembly code and symbol table of the input. To re-iterate, ELF.txt is a general input file that was used for general testing of the program 
while the other input files are for testing errors specifically.
