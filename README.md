# PL-0-Compiler
Recursive Decent Parser &amp; Intermediate Code Generator

First, my file is compiled with a gcc compiler, and then the a.out file is ran after the source code is turned into an executable.
All the input is included in the input text file which has lines of source code to be taken in by the code in "hw4compiler.c".
It takes in file input, character by character, until it reaches the end of the file. The program will have special cases for certain 
symbols, phrases, and declarations. Once the program recognizes what it is taking in as input, it creates a token to be saved in the
lexeme table, which I simulated using an array. As the program takes in more characters as input, it keeps creating more tokens until 
the end of the file. When the program is done simulating a lexical analyzer, the next portion of the code will call operations
to simulate the PL/0 compiler. The code goes through the created lexeme table, in order, and performs a series of functions to 
compile the input and process whether or not there are errors in the code. Functions were simulated in the code using actual methods in C, called 
BLOCK, STATEMENT, EXPRESSION, CONDITION, TERM, and FACTOR. These methods run recursively depending on the token input. Furthermore, 
there is a method created to find the index of a symbol in the symbol table as well as one to find the operation number for a named operation 
such as "LIT". While running through the list of tokens, there are a number of errors that are accounted for, including specific ones for procedure 
and call tokens. When an error is found, the program prints the error it found and closes the program. If there are no errors in the input, then 
the program finishes by printing the assembly code with numberics rather than mnemonics. This complier now has the funcitonality of procedures and 
calling to them. As stated before, the new procedure and call code in "hw4compiler.c" is tested with "procTest.txt" files for procedures and "callTest.txt" 
files for calls. Not only this, but there is also a basic input file called "inputFile.txt" with input meant to display the successful running of 
the program. Output from this test is in a file called "elf.txt", which would act as the executable file for the virtual machine.

Below is a list of the error testing files and their intentions:
callTest1 - test for undeclared ident
callTest2 - test for no ident after callsym
callTest3 - test for calling a non-procedure

procTest1 - test for no ident after proc declaration
procTest2 - test for lack of semicolon after proc declaration
procTest3 - test for using procedure in assignment

error#.txt - test for other, more common errors

No Group, project done alone
