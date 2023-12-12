#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// defining token types, starting from 1
typedef enum {
  oddsym = 1, identsym, numbersym, plussym, minussym,
  multsym, slashsym, ifelsym, eqlsym, neqsym, lessym, leqsym,
  gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym,
  periodsym, becomessym, beginsym, endsym, ifsym, thensym,
  whilesym, dosym, callsym, constsym, varsym, procsym, writesym,
  readsym , elsesym
} token_type;

// struct objects created to hold information of tokens, symbols, and assembly codes
typedef struct token {
  token_type type;
  const char *filename;
  char *text;
  int value;
} token;

typedef struct symbol {
  int kind;
  char name[10];
  int val;
  int level;
  int addr;
  int mark;
} symbol;

typedef struct code {
  char text[4];
  int opNum;
  int level;
  int M;
} code;


// initiate file variables
FILE * inputFile;
char * filename = ".txt";
FILE * outputFile;
char * filename2 = "elf.txt";

// setting global variables
int MAX_SYMBOL_TABLE_SIZE = 500;
int maxIdentLen = 11;
int maxNum = 99999;
// variables for keeping track of the symbol table
symbol symbol_table[500];
int numSymbols = 0;
// variable declared to hold the current address
int curAdd = 0;
int numVars = 0;
// index for the token list, whenever this increments, it acts as getNextToken
int idx = 0;
// variables for keeping track of assembly code
code lines[100];
int curLine = 0;
// variable holding the maximum amount of levels
int levmax = 5;

// finds index of a symbol in the symbol table, -1 if not found
int symbolTableCheck(char name[10]) {
  for (int x=0; x<numSymbols; x++)
    if (strcmp(symbol_table[x].name, name) == 0)
      return x;
  return -1;
}

// finds the correct op number in assembly for an operation
int findOp(char op[]) {
  if (strcmp(op, "LIT") == 0)
    return 1;
  if (strcmp(op, "LOD") == 0)
    return 3;
  if (strcmp(op, "STO") == 0)
    return 4;
  if (strcmp(op, "CAL") == 0)
    return 5;
  if (strcmp(op, "INC") == 0)
    return 6;
  if (strcmp(op, "JMP") == 0)
    return 7;
  if (strcmp(op, "JPC") == 0)
    return 8;
  if (strcmp(op, "SYS") == 0)
    return 9;
  return 2;
}

// creates a new assembly code for an instruction and adds it to the list of codes
void EMIT(char op[], int level, int address) {
  code temp;
  strcpy(temp.text, op);
  temp.opNum = findOp(op);
  temp.level = level;
  temp.M = address;

  lines[curLine] = temp;
  curLine++;
}

// runs code for factor expressions found ident|num|(expression)
void FACTOR(token * lexeme, int lvl) {
  if (lexeme[idx].type == identsym) // ident
  {
    int symIdx = symbolTableCheck(lexeme[idx].text);
    if (symIdx == -1)
    {
      fprintf(outputFile, "Error: undeclared identifier");
      exit(1);
    }
    if (symbol_table[symIdx].kind == 1) // emits code whether ident is const or var
      EMIT("LIT", lvl, symbol_table[symIdx].val);
    else if (symbol_table[symIdx].kind == 2)
      EMIT("LOD", lvl, symbol_table[symIdx].addr);
    else
    { // error check for when input tries to use a procsym for assignments
      fprintf(outputFile, "Error: Assignment to procedure is not allowed");
      exit(1);
    }
    idx++;
  }
  else if (lexeme[idx].type == numbersym) // number
  {
    EMIT("LIT", lvl, 2);
    idx++;
  }
  else if (lexeme[idx].type == lparentsym) // parenthesis expression with error checks
  {
    idx++;
    EXPRESSION(lexeme, lvl);
    if (lexeme[idx].type != rparentsym)
    {
      fprintf(outputFile, "Error: right parenthesis must follow left parenthesis");\
      exit(1);
    }
    idx++;
  }
  else // error check if no expressions given
  {
    fprintf(outputFile, "Error: arithmetic equations must contain operands, parentheses, numbers, or symbols");
    exit(1);
  }
}

// runs code for term expressions, factor {(*|/) factor}
void TERM(token * lexeme, int lvl) {
  FACTOR(lexeme, lvl);
  while (lexeme[idx].type == multsym || lexeme[idx].type == slashsym) // cases for if mult or slash sym
  {
    if (lexeme[idx].type == multsym) // * check
    {
      idx++;
      FACTOR(lexeme, lvl);
      EMIT("OPR", lvl, 3);
    }
    else // / if not *
    {
      idx++;
      FACTOR(lexeme, lvl);
      EMIT("OPR", lvl, 4);
    }
  }
}

// runs code for expressions found, [+|-] term [+|-] term
void EXPRESSION(token * lexeme, int lvl) {
  TERM(lexeme, lvl);
  while (lexeme[idx].type == plussym || lexeme[idx].type == minussym)
  {
    if (lexeme[idx].type == plussym) // + check
    {
      idx++;
      TERM(lexeme, lvl);
      EMIT("OPR", lvl, 1);
    }
    else // - if not +
    {
      idx++;
      TERM(lexeme, lvl);
      EMIT("OPR", lvl, 2);
    }
  }
}

// performs odd-expressions of rel-op expressions
void CONDITION(token * lexeme, int lvl) {
  if (lexeme[idx].type == oddsym) // odd check
  {
    idx++;
    EXPRESSION(lexeme, lvl);
    EMIT("ODD", lvl, curAdd);
  }
  else // if not odd expression, rel-op check
  {
    EXPRESSION(lexeme, lvl);
    if (lexeme[idx].type == eqlsym) // = check
    {
      idx++;
      EXPRESSION(lexeme, lvl);
      EMIT("EQL", lvl, 5);
    }
    else if (lexeme[idx].type == neqsym) // != check
    {
      idx++;
      EXPRESSION(lexeme, lvl);
      EMIT("NEQ", lvl, 6);
    }
    else if (lexeme[idx].type == lessym) // < check
    {
      idx++;
      EXPRESSION(lexeme, lvl);
      EMIT("LSS", lvl, 7);
    }
    else if (lexeme[idx].type == leqsym) // <= check
    {
      idx++;
      EXPRESSION(lexeme, lvl);
      EMIT("LEQ", lvl, 8);
    }
    else if (lexeme[idx].type == gtrsym) // > check
    {
      idx++;
      EXPRESSION(lexeme, lvl);
      EMIT("GTR", lvl, 9);
    }
    else if (lexeme[idx].type == geqsym) // >= check
    {
      idx++;
      EXPRESSION(lexeme, lvl);
      EMIT("GEQ", lvl, 10);
    }
    else // error check if unfinished rel-op
    {
      fprintf(outputFile, "Error: condition must contain comparison operator");
      exit(1);
    }
  }
}

// performs operations for any statements after var\const declaration
void STATEMENT(token * lexeme, int lvl) {
  if (strcmp(lexeme[idx].text, ":=") == 0)
  {
    idx--;
  }
  if (lexeme[idx].type == identsym) // var ops
  {
    int symIdx = symbolTableCheck(lexeme[idx].text);
    if (symIdx == -1) // error if ident not declared in symbol table
    {
      fprintf(outputFile, "Error: undeclared identifier");
      exit(1);
    }
    else if (symbol_table[symIdx].kind != 2) // checks for no const
    {
      fprintf(outputFile, "Error: only variable values may be altered");
      exit(1);
    }
    else
    {
      idx++;
      if (lexeme[idx].type != becomessym)
      {
        fprintf(outputFile, "Error: assignment statements must use :=");
        exit(1);
      }
      idx++;
      EXPRESSION(lexeme, lvl);
      EMIT("STO", lvl, symbol_table[symIdx].addr);
      return;
    }
  }
  if (lexeme[idx].type == callsym) // checks for a call to procedure
  {
    idx++;
    if (lexeme[idx].type != identsym) // must be followed by ident
    {
      fprintf(outputFile, "Error: call must be followed by an identifier");
      exit(1);
    }
    int pos = symbolTableCheck(lexeme[idx].text);
    printf("$d", pos);
    if (pos == -1) // makes sure call exists
    {
      fprintf(outputFile, "Error: undeclared identifier");
      exit(1);
    }
    else if (symbol_table[pos].kind == 3) // if the call is to a proc, proceed
    {
      EMIT("CAL", lvl-symbol_table[pos].level, symbol_table[pos].addr);
      EMIT("JMP", lvl-symbol_table[pos].level, symbol_table[pos].addr);
      EMIT("INC", lvl-symbol_table[pos].level, symbol_table[pos].val);
    }
    else // useless for var/const
    {
      fprintf(outputFile, "Error: call of a constant or variable is meaningless");
      exit(1);
    }
    idx++;
  }
  if (lexeme[idx].type == beginsym) // checks for begin statement
  {
    do
    {
      idx++;
      STATEMENT(lexeme, lvl);
    } while(lexeme[idx].type == semicolonsym); // keep finding statements ending in semicolon
    if (lexeme[idx].type != endsym) // begin must be followed by end
    {
      fprintf(outputFile, "Error: begin must be followed by end");
      exit(1);
    }
    idx++;
    return;
  }
  if (lexeme[idx].type == ifsym) // checks for if
  {
    idx++;
    CONDITION(lexeme, lvl);
    int jpcIdx = curLine; // sets up jump code
    EMIT("JPC", lvl, jpcIdx);
    if (lexeme[idx].type != thensym) // if must be followed by then
    {
      fprintf(outputFile, "Error: if must be followed by then");
      exit(1);
    }
    idx++;
    STATEMENT(lexeme, lvl);
    lines[jpcIdx].M = curLine; // set code addr to current code line
    return;
  }
  if (lexeme[idx].type == whilesym) // checks for while
  {
    idx++;
    int loopIdx = curLine; // sets up place to come back to
    CONDITION(lexeme, lvl);
    if (lexeme[idx].type != dosym)
    {
      fprintf(outputFile, "Error: while must be followed by do");
      exit(1);
    }
    idx++;
    int jpcIdx = curLine; // sets up conditional jump code
    EMIT("JPC", lvl, jpcIdx);
    STATEMENT(lexeme, lvl);
    EMIT("JMP", lvl, loopIdx);
    lines[jpcIdx].M = curLine; // set code addr to current code line if condition met
    return;
  }
  if (lexeme[idx].type == readsym) // check for read
  {
    idx++;
    if (lexeme[idx].type != identsym) // makes sure reading into an ident
    {
      fprintf(outputFile, "Error: only variable values may be altered");
      exit(1);
    }
    int symIdx = symbolTableCheck(lexeme[idx].text);
    if (symIdx == -1) // checks if var undeclared
    {
      fprintf(outputFile, "Error: undeclared identifier");
      exit(1);
    }
    if (symbol_table[symIdx].kind != 2) // makes sure reading into a var
    {
      fprintf(outputFile, "Error: only variable values may be altered");
      exit(1);
    }
    idx++;
    EMIT("SYS", lvl, 2); // read is a sys op
    EMIT("STO", lvl, symbol_table[symIdx].addr);
    return;
  }
  if (lexeme[idx].type == writesym) // checks for write
  {
    idx++;
    EXPRESSION(lexeme, lvl);
    EMIT("SYS", lvl, 1); // write is a sys op
    return;
  }
}

// used to declare and run blocks of code (procedures/main)
void BLOCK(token * lexeme, int lvl) {
  // variables holding whether or not var/consts are declared
  int didConst = 0;
  int didVar = 0;

  // check to make sure there are no infinite loops
  if (lvl > levmax)
  {
    fprintf(outputFile, "Error: level surpasses level maximum");
    exit(1);
  }
  if (strcmp(lexeme[idx].text, "const") == 0)
  {
    do
    {
      didConst = 1;
      idx++;
      if (lexeme[idx].type != identsym) // makes sure ident is given
      {
        fprintf(outputFile, "Error: const, var, and read keywords must be followed by identifier");
        exit(1);
      }
      else if (symbolTableCheck(lexeme[idx].text) != -1) // makes sure ident doesn't exist yet
      {
        fprintf(outputFile, "Error: symbol name has already been declared");
        exit(1);
      }
      else // if new const, start to create new const
      {
        char idenName[10];
        strcpy(idenName, lexeme[idx].text);
        idx++;
        if (lexeme[idx].type != eqlsym) // needs eql sym to take in a value
        {
          fprintf(outputFile, "Error: constants must be assigned with =");
          exit(1);
        }
        else if (lexeme[idx+1].type != numbersym) // if no value given, also error
        {
          fprintf(outputFile, "Error: constants must be assigned an integer value");
          exit(1);
        }
        else // if passes fully, create const and add to the symbol table
        {
          symbol temp;
          temp.kind = 1;
          strcpy(temp.name, idenName);
          temp.val = lexeme[idx+1].value;
          temp.level = lvl;
          temp.addr = curAdd;
          curAdd++;
          temp.mark = 0;
          symbol_table[numSymbols] = temp;
          numSymbols++;
          idx += 2;
        }
      }
    } while(lexeme[idx].type == commasym); // checks for more than one const ident
    if (lexeme[idx].type != semicolonsym) // statement must end in semicolon
    {
      fprintf(outputFile, "Error: constant and variable declarations must be followed by a semicolon");
      exit(1);
    }
  }
  if (didConst == 1)
    idx++;
  // var declaration (only happens once so done at beginning without a function, still updates numVars)
  if (strcmp(lexeme[idx].text, "var") == 0) // checks for var
  {
    do {
      idx++;
      numVars++;
      didVar = 1;
      if (lexeme[idx].type != identsym) // if not ident, error
      {
        fprintf(outputFile, "Error: const, var, and read keywords must be followed by identifier");
        exit(1);
      }
      else if (symbolTableCheck(lexeme[idx].text) != -1) // makes sure given ident doesn't exist
      {
        fprintf(outputFile, "Error: symbol name has already been declared");
        exit(1);
      }
      else // if passes, create new var and add it tot he symbol table
      {
        symbol temp;
        temp.kind = 2;
        strcpy(temp.name, lexeme[idx].text);
        temp.val = 0;
        temp.level = lvl;
        temp.addr = curAdd + numVars - 1; // (-1 since numVars was updated before the address work)
        temp.mark = 0;
        symbol_table[numSymbols] = temp;
        numSymbols++;
        idx++;
      }
    } while(lexeme[idx].type == commasym); // checks for multiple vars in declaration
    if (lexeme[idx].type != semicolonsym) // statement must end in semicolon
    {
      fprintf(outputFile, "Error: constant and variable declarations must be followed by a semicolon");
      exit(1);
    }
  }
  if (didVar == 1)
  {
    idx++;
  }
  while (lexeme[idx].type == procsym) // checks for procedure
  {
    idx++;
    if (lexeme[idx].type == identsym) // adds procedure into table if correct syntax
    {
      symbol temp;
      temp.kind = 3;
      strcpy(temp.name, lexeme[idx].text);
      temp.val = 0;
      temp.level = lvl;
      temp.addr = curAdd + numVars - 1; // (-1 since numVars was updated before the address work)
      temp.mark = 0;
      symbol_table[numSymbols] = temp;
      numSymbols++;
      idx++;
    }
    else
    {
      fprintf(outputFile, "Error: incorrect symbol after procedure declaration");
      exit(1);
    }
    if (lexeme[idx].type != semicolonsym)
    {
      fprintf(outputFile, "Error: semicolon or comma missing");
      exit(1);
    }
    idx++;
    BLOCK(lexeme, lvl+1);
    if (lexeme[idx].type != semicolonsym)
    {
      fprintf(outputFile, "Error: semicolon or end expected");
      exit(1);
    }
    idx++;
  }
  int pos = symbolTableCheck(lexeme[idx].text);
  while ((lexeme[idx].type == commasym) || (lexeme[idx].type == varsym) || (lexeme[idx].type == procsym))
  {
    if (symbolTableCheck(lexeme[idx].text) == -1) // makes sure given ident doesn't exist
    {
      fprintf(outputFile, "Error: undeclared identifier");
      exit(1);
    }
    else
    { // assembly code emitting after block runs
      lines[symbol_table[pos].addr].M = curAdd;
      symbol_table[pos].addr = curAdd;
      EMIT("JMP", 0,  3*curAdd);
      EMIT("INC", 0, symbol_table[pos].addr);
    }
  }
  // update address/index
  idx++;
  curAdd += numVars;
  EMIT("JMP", 0,  3*curAdd);
  EMIT("INC", 0, symbol_table[pos].addr);

  // look for statements after declarations
  STATEMENT(lexeme, lvl);
  EMIT("OPR", 0, 0);
}

// first part of main is from lex.c, second half for pcg.c starting at const declaration
int main(int argc, char *argv[])
{
  // initialize pointers to the files
  inputFile = fopen(filename, "r");
  outputFile = fopen(filename2, "w");
  // create an array of tokens to save
  token * lexeme = malloc(sizeof(token)*100);
  int lexCount = 0;

  // if there is not enough arguments passed from the terminal
  if (argc < 2)
  {
      fprintf(outputFile, "Error: Filename missing!\n");
      exit(1);
  }
  else
  {
      // get the file name from the passed argument.
      filename = argv[1];
      fprintf(outputFile, "Filename: %s\n", filename);
  }

  // read and print the file
  inputFile = fopen(filename, "r");
  // if file cannot be opened properly
  if (!inputFile)
  {
      fprintf(outputFile, "Error: Failed to open file\n");
      exit(1);
  }

  // while loop continues until EOF
  char inputToken;
  while ((inputToken = getc(inputFile)) != EOF)
  {
    // reset token values
    token tok;
    tok.type = 0;
    tok.text = "";

    // do not create a new token if ' ' or '\n'
    if (inputToken == ' ')
    {
      fprintf(outputFile, " ");
      continue;
    }
    if (inputToken == '\n')
    {
      fprintf(outputFile, "\n");
      continue;
    }
    if (inputToken == '/') // comment checker/printer with no token creation
    {
      inputToken = getc(inputFile);
      if (inputToken == '*'){
        fprintf(outputFile, "/%c", inputToken);
        while ((inputToken = getc(inputFile)) != '/')
        {
          fprintf(outputFile, "%c", inputToken);
          if (inputToken == EOF)
            fprintf(outputFile, "File ended...");
        }
        fprintf(outputFile, "/");
      }
      continue;
    }
    if (ispunct(inputToken) != 0) // token creation for punction marks/symbols
    {
      if (inputToken == '.')
      {
        fprintf(outputFile, "%c", inputToken);
        tok.text = malloc(sizeof(char)*5);
        strcpy(tok.text, ".");
        tok.type = periodsym;
      }
      else if (inputToken == ';')
      {
        fprintf(outputFile, "%c", inputToken);
        tok.text = malloc(sizeof(char)*5);
        strcpy(tok.text, ";");
        tok.type = semicolonsym;
      }
      else if (inputToken == ',')
      {
        fprintf(outputFile, "%c", inputToken);
        tok.text = malloc(sizeof(char)*5);
        strcpy(tok.text, ",");
        tok.type = commasym;
      }
      else if (inputToken == '(')
      {
        fprintf(outputFile, "%c", inputToken);
        tok.text = malloc(sizeof(char)*5);
        strcpy(tok.text, "(");
        tok.type = lparentsym;
      }
      else if (inputToken == ')')
      {
        fprintf(outputFile, "%c", inputToken);
        tok.text = malloc(sizeof(char)*5);
        strcpy(tok.text, ")");
        tok.type = rparentsym;
      }
      else if (inputToken == ':') // checks to see if ':=', else invalid symbol
      {
        fprintf(outputFile, "%c", inputToken);
        inputToken = getc(inputFile);
        if (inputToken == '=')
        {
          fprintf(outputFile, "%c", inputToken);
          tok.text = malloc(sizeof(char)*5);
          strcpy(tok.text, ":=");
          tok.type = becomessym;
        }
        else
        {
          ungetc(inputToken, inputFile);
          fprintf(outputFile, "Error: symbol not found");
          exit(1);
        }
      }
      else if (inputToken == '=')
      {
        fprintf(outputFile, "%c", inputToken);
        tok.text = malloc(sizeof(char)*5);
        strcpy(tok.text, "=");
        tok.type = eqlsym;
      }
      else if (inputToken == '+')
      {
        fprintf(outputFile, "%c", inputToken);
        tok.text = malloc(sizeof(char)*5);
        strcpy(tok.text, "+");
        tok.type = plussym;
      }
      else if (inputToken == '-')
      {
        fprintf(outputFile, "%c", inputToken);
        tok.text = malloc(sizeof(char)*5);
        strcpy(tok.text, "-");
        tok.type = minussym;
      }
      else if (inputToken == '*')
      {
        fprintf(outputFile, "%c", inputToken);
        tok.text = malloc(sizeof(char)*5);
        strcpy(tok.text, "*");
        tok.type = multsym;
      }
      else if (inputToken == '/')
      {
        fprintf(outputFile, "%c", inputToken);
        tok.text = malloc(sizeof(char)*5);
        strcpy(tok.text, "/");
        tok.type = slashsym;
      }
      else if (inputToken == '<')
      {
        fprintf(outputFile, "%c", inputToken);
        inputToken = getc(inputFile);
        if (inputToken == "=")
        {
          fprintf(outputFile, "%c", inputToken);
          tok.text = malloc(sizeof(char)*5);
          strcpy(tok.text, "<=");
          tok.type = leqsym;
        }
        else if (inputToken == '>')
        {
          fprintf(outputFile, "%c", inputToken);
          tok.text = malloc(sizeof(char)*5);
          strcpy(tok.text, "<>");
          tok.type = neqsym;
        }
        else
        {
          ungetc(inputToken, inputFile);
          tok.text = malloc(sizeof(char)*5);
          strcpy(tok.text, "<");
          tok.type = lessym;
        }
      }
      else if (inputToken == '>')
      {
        fprintf(outputFile, "%c", inputToken);
        inputToken = getc(inputFile);
        if (inputToken == '=')
        {
          fprintf(outputFile, "%c", inputToken);
          tok.text = malloc(sizeof(char)*5);
          strcpy(tok.text, ">=");
          tok.type = geqsym;
        }
        else
        {
          ungetc(inputToken, inputFile);
          tok.text = malloc(sizeof(char)*5);
          strcpy(tok.text, ">");
          tok.type = gtrsym;
        }
      }
      else
      {
        fprintf(outputFile, "Error: symbol not found");
        exit(1);
      }
    }

    else if (isdigit(inputToken) != 0) // token creation for integer inputs
    {
      int fullNum = 0;
      // math done to create the integer as more digits come in
      while (isdigit(inputToken) != 0)
      {
        fullNum *= 10;
        fullNum += ((int)inputToken - '0');
        inputToken = getc(inputFile);
      }

      ungetc(inputToken, inputFile);

      // makes sure the input integer isn't too large before creating the token
      if (fullNum <= maxNum)
      {
        tok.value = fullNum;
        tok.type = numbersym;
        fprintf(outputFile, "%d", fullNum);
      }
      else
      {
        fprintf(outputFile, "Error: number too large");
        exit(1);
      }
    }

    else if (isalpha(inputToken) !=0) // token creation for declarations/idents
    {
      int index = 0;
      char func[maxIdentLen];
      func[index] = inputToken;

      // keeps taking in chars until an empty space
      while (isalnum(inputToken) != 0)
      {
        index++;
        inputToken = getc(inputFile);
        func[index] = inputToken;
      }

      // creation of token with type dependent on the input phrase
      ungetc(inputToken, inputFile);
      func[index] = '\0';
      if (index <= maxIdentLen) // only make a token if the length is valid
      {
        fprintf(outputFile, "%s", func);
        tok.text = malloc(sizeof(char)*maxIdentLen);
        strcpy(tok.text, func);

        if (strcmp(tok.text, "begin") == 0)
          tok.type = beginsym;
        else if (strcmp(tok.text, "end") == 0)
          tok.type = endsym;
        else if (strcmp(tok.text, "if") == 0)
          tok.type = ifsym;
        else if (strcmp(tok.text, "then") == 0)
          tok.type = thensym;
        else if (strcmp(tok.text, "while") == 0)
          tok.type = whilesym;
        else if (strcmp(tok.text, "do") == 0)
          tok.type = dosym;
        else if (strcmp(tok.text, "const") == 0)
          tok.type = constsym;
        else if (strcmp(tok.text, "var") == 0)
          tok.type = varsym;
        else if (strcmp(tok.text, "write") == 0)
          tok.type = writesym;
        else if (strcmp(tok.text, "read") == 0)
          tok.type = readsym;
        else if (strcmp(tok.text, "odd") == 0)
          tok.type = oddsym;
        else if (strcmp(tok.text, "call") == 0)
          tok.type = callsym;
        else if (strcmp(tok.text, "procedure") == 0)
          tok.type = procsym;
        else
          tok.type = identsym;

        func[0] = '\0';
      }
      else
      {
        fprintf(outputFile, "Error: identifier too large");
        exit(1);
      }
    }
    if (tok.type != 0) // only adds our token to the list if not ' ' or '\n'
    {
      lexeme[lexCount] = tok;
      lexCount++;
    }

  } // end token getting

    fprintf(outputFile, "\n");
    // start compiling
    int lvl = 0;
    while (idx<lexCount-1)
    {
      BLOCK(lexeme, lvl);
    }
    fprintf(outputFile, "\n");

    if (lexeme[idx].type != periodsym) // if code doesn't end in period, error
    {
      fprintf(outputFile, "Error: program must end with period");
      exit(1);
    }
    EMIT("SYS", 0, 3); // halt code when finished

    // mark all symbols in table when done
    for (int z=0; z<numSymbols; z++)
    {
      symbol_table[z].mark = 1;
    }

    // no errors if made to this point
    fprintf(outputFile, "No errors, program is syntactically correct\n\n");

    // printing of the assembly codes
    fprintf(outputFile, "OP\tL\tM\n");
    for (int x=0; x<curLine; x++)
    {
      fprintf(outputFile, "%d\t%d\t%d\n", lines[x].opNum, lines[x].level, lines[x].M);
    }
  return 0;
}
