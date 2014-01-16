// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include "alloc.h"
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#include <error.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */

struct command_stream{
  command_t* commands;
  command_t** cst_it;
  int size;
};

/*Helper Functions*/

/*
 * Checks if the given character is within the set of all
 * valid characters allowed by our interpreter.
 */
int isValidCharacter(char c)
{
  if( isalnum(c) || isspace(c) || c == '!' || c == '%'
       || c == '+' || c == ',' || c == '-' || c == '.'
       || c == '/' || c == ':' || c == '@' || c == '^'
       || c == '_' || c == '|' || c == '&' || c == '('
       || c == ')' || c == '<' || c == '>' || c == ';')
  {
    return 1;
  }
  return 0;
}

int checkOpenParen(char* parsedFile, char c, int initSize)
{
  if( c != '(')
  {
    error(1, 0, "Improper call to checkOpenParen");
    return 0;
  }
  int parsePointer = initSize-1;
  while(parsePointer > 0)
  {
    if( parsedFile[parsePointer] == c)
    {
      //do Nothing
    }
    else if( parsedFile[parsePointer] == '|' || parsedFile[parsePointer] == '&' || 
             parsedFile[parsePointer] == ';' || parsedFile[parsePointer] == '(' ||
             parsedFile[parsePointer] == '\n')
    {
      return 1;
    }
    else
    {
      return 0;
    }
    parsePointer--;
  }
  return 0;
}

int checkCloseParen(char* parsedFile, char c, int initSize)
{
  int parsePointer = initSize-1;
  int semicolonFound = 0;
  while(parsePointer > 0)
  {
    if( parsedFile[parsePointer] == c || isspace(parsedFile[parsePointer]))
    {
      //do nothing
    }
    else if( parsedFile[parsePointer] == '|' || parsedFile[parsePointer] == '&'|| 
             parsedFile[parsePointer] == '(')
    {
      return 0;
    }
    else if( isValidCharacter(parsedFile[parsePointer]) )
    {
      return 1;
    }
    parsePointer--;
  }
  return 0;
}

int checkSpecialToken(char* parsedFile, char c, int initSize)
{
  if(c != '&' && c != '|' && c != ';' && c != '(' && c != ')')
  {
    error(1, 0, "Improper call to checkSpecialToken");
    return 0;
  }

  if(c == '(')
  {
    return checkOpenParen(parsedFile, c, initSize);
  }

  if(c == ')')
  {
    return checkCloseParen(parsedFile, c, initSize);
  }

  int tempCounter = 0;
  int parsePointer = initSize-1;
  int spaceFound = 0;
  int closeParenFound = 0;
  while(parsePointer > 0)
  {
    if( parsedFile[parsePointer] == c)
    {
      if(c == '|' || c == '&')
      {
        tempCounter++;
        if(spaceFound || closeParenFound)
        {
          return 0;
        }
        else if(tempCounter > 1)
        {
          return 0;
        }
      }
      else if(c == ';')
      {
        if(closeParenFound)
        {
          return 1;
        }
        else
        {
          return 0;
        }
      }
    }
    else if( parsedFile[parsePointer] == '|' || parsedFile[parsePointer] == '&' || 
             parsedFile[parsePointer] == ';' || parsedFile[parsePointer] == '(' ||
             parsedFile[parsePointer] == '\n')
    {
      return 0;
    }
    else if( isspace(parsedFile[parsePointer]) )
    {
      spaceFound = 1;
    }
    else if( parsedFile[parsePointer] == ')' )
    {
      closeParenFound = 1;
    }
    else if( isValidCharacter(parsedFile[parsePointer]) )
    {
      return 1;
    }
    parsePointer--;
  }

  return 0;
}

int checkRedirectToken(char* parsedFile, char c, int initSize)
{
  if(c != '<' && c != '>')
  {
    error(1, 0, "Improper call to checkRedirectToken");
    return 0;   
  }

  int parsePointer = initSize-1;
  while(parsePointer > 0)
  {
    if( parsedFile[parsePointer] == '|' || parsedFile[parsePointer] == '&' || 
        parsedFile[parsePointer] == ';' || parsedFile[parsePointer] == '(' ||
        parsedFile[parsePointer] == '>' || parsedFile[parsePointer] == '<' ||
        parsedFile[parsePointer] == '\n')
    {
      return 0;
    }
    else if(isspace(parsedFile[parsePointer]) || parsedFile[parsePointer] == ')')
    {
      //do nothing
    }
    else
    {
      return 1;
    }
    parsePointer--;
  }
  return 0;
}

/*
 * Checks if next char value is valid in respect to current set of values
 * if next char is valid, it returns 1
 * if next char is invalid, it returns 0
 */
int isProperGrammar(char* parsedFile, int initSize, int* parenCount, char nextInput)
{
  // if its the first character, 
  // then it must be a non-special character
  if(!isValidCharacter(nextInput))
    return 0;

  if(initSize == 0)
  {
    if(nextInput != '&' && nextInput != '|' && nextInput != ';')
      return 1;
    else
      return 0;
  }
  // If & or | follows a different special character
  // or there are three of these back to back without
  // intermediate valid characters
  // Then return false
  if(nextInput == '&' || nextInput == '|' || nextInput == ';' ||
     nextInput == '(' || nextInput == ')' )
  {
    if(!checkSpecialToken(parsedFile, nextInput, initSize))
    {
      return 0;
    }
  }
  else if(nextInput == '<' || nextInput == '>')
  {
    if(!checkRedirectToken(parsedFile, nextInput, initSize))
    {
      return 0;
    }
  }
  else
  {
      int parsePointer = initSize-1;
      if(parsedFile[parsePointer] == '&')
      {
        parsePointer--;
        if(parsePointer < 0 || parsedFile[parsePointer] != '&')
          return 0;
      }
  }

  if(nextInput == '(')
  {
    (*parenCount)++;
  }
  else if(nextInput == ')')
  {
    (*parenCount)--;
    if( (*parenCount) < 0 )
    {
      return 0;
    }
  }

  return 1;
}

/*
 * Fill parseFile with valid character array input
 * Remove all white space and comments
 * Return the size of the array
 */
int parseFile(int (*get_next_byte) (void *), void *get_next_byte_argument, char* parsedFile)
{
  int size = 0;
  unsigned int capacity = 256;

  //Allocate space in parsedFile
  if(!parsedFile)
  {
    error(1, 0, "Unexpected Null parsedFile");
    return 0;
  }

  //to hold next character
  char c;
  //For parantheses correctness
  int parenCount = 0;

  //Used to remove comments
  int isComment = 0;

  //Used to check for valid redirection
  int findSingleWord = 0;
  int singleWordStarted = 0;
  int foundLessThan = 0;
  int foundSingleWord = 0;

  while( (c = get_next_byte(get_next_byte_argument)) && c != 0 && !feof(get_next_byte_argument))
  {
    // If it is a comment
    // then ignore until new line is reached
    if(c == '#')
    {
      isComment = 1;
      continue;
    }

    if(isComment && c != '\n'){
      continue;
    }else if(isComment){
      isComment = 0;
    }

    //Remove Extra New Lines
    if(c == '\n')
    {
      int parsePointer = size-1;
      if(size == 0 || parsedFile[parsePointer] == '\n')
      {
        continue;
      }
      if(parsedFile[parsePointer] == ' ')
      {
        if(parsePointer-1 >= 0){
          if(parsedFile[parsePointer-1] == '&' || parsedFile[parsePointer-1] == '|' || parsedFile[parsePointer-1] == ';' )
          {
            continue;
          }
        }
        parsedFile[parsePointer] = '\n';
        continue;
      }
      if(parsedFile[parsePointer] == '&' || parsedFile[parsePointer] == '|' || parsedFile[parsePointer] == ';'){
        continue;
      }
    }
    //Remove Extra White Spaces
    else if(isspace(c))
    {
      if(c != ' ')
      {
        c = ' ';
      }
      if(size == 0 || parsedFile[size-1] == ' ' || parsedFile[size-1] == '\n')
      {
        continue;
      }
    }
    //Check Grammar
    else
    {
      if(!isProperGrammar(parsedFile, size, &parenCount, c))
      {
        error (1, 0, "Improper Syntax in File");
      }
    }

    if(findSingleWord)
    {
      if(foundSingleWord)
      {
        if( c == '|' || c== '&' || c == ';' || c == '(' || 
                 c == '\n')
        {
          findSingleWord = 0;
          foundSingleWord = 0;
          singleWordStarted = 0;
          foundLessThan = 0;
        }
        else if(c == '>' && foundLessThan)
        {
          foundLessThan = 0;
          findSingleWord = 1;
          foundSingleWord = 0;
          singleWordStarted = 0;
        }
        else if(isspace(c))
        {
          //do nothing
        }
        else
        {
          error (1, 0, "Improper Syntax in File: Improper Redirect");
          return 0;
        }
      }
      else if(singleWordStarted)
      {
        if( c== '(' || c== '<')
        {
          error (1, 0, "Improper Syntax in File: Improper Redirect");
          return 0;
        }
        else if( c == '|' || c== '&' || c == ';' || c == '\n')
        {
          findSingleWord = 0;
          foundSingleWord = 0;
          singleWordStarted = 0;
          foundLessThan = 0;
        }
        else if(isspace(c))
        {
          foundSingleWord = 1;
        }
        else if( c == '>' && foundLessThan )
        {
          findSingleWord = 1;
          singleWordStarted = 0;
          foundLessThan = 0;
          foundSingleWord = 0;
        }
        else if(c == '>')
        {
          error (1, 0, "Improper Syntax in File: Improper Redirect");
          return 0;
        }
      }
      else if( c == '|' || c== '&' || c == ';' || c == '(' || 
              c == '>' || c == '<' || c == '\n')
      {
        error (1, 0, "Improper Syntax in File: Improper Redirect");
        return 0;
      }
      else if(isspace(c))
      {
        //do nothing
      }
      else
      {
        singleWordStarted = 1;
      }
    }
    else if(c == '<')
    {
      findSingleWord = 1;
      foundLessThan = 1;
      foundSingleWord = 0;
      singleWordStarted = 0;
    }
    else if(c == '>')
    {
      findSingleWord = 1;
      foundLessThan = 0;
      foundSingleWord = 0;
      singleWordStarted = 0;
    }

    if(!(size < (signed int)capacity))
    {
      checked_grow_alloc((void*)parsedFile, &capacity);
    }

    //Add Valid Character to Array
    if(c == ')' || c == '\n')
    {
      int parsePointer = size-1;
      while(parsePointer >= 0)
      {
        if(isspace(parsedFile[parsePointer]))
        {
          size--;
        }
        else
        {
          break;
        }
        parsePointer--;
      }
    }
    parsedFile[size] = c;
    printf("size = %d char = %c\n",size, c);

    //Increment character count
    size++;
  }

  //Check if file ended properly
  if(parsedFile[size-1] != ';' && parsedFile[size-1]!='\n' && !isProperGrammar(parsedFile, size, &parenCount, ';'))
  {
    error (1, 0, "Improper Syntax in File");
  }

  //add end of file
  if(!(size < (signed int)capacity))
  {
    checked_grow_alloc(parsedFile, &capacity);
  }

  //Add Null Character to End of Array
  parsedFile[size] = '\0';
  //size++;

  if(parenCount > 0)
  {
    error (1, 0, "Improper Syntax in File: Unclosed Parentheses");
    return 0;
  }
  printf("\ncapacity = %d\n", capacity);
  return size;
}

/*
 * Use parsedFile to create a set of command structs 
 * for each proper command in order. 
 * Returns the number of commands found.
 */
int createCommandTree(char* parsedFile, int size, command_t* commands)
{

  //Checking for Errors
  if(parsedFile == NULL)
  {
    error(1, 0, "Unexpected Null parsedFile");
    return 0;
  }
  if(commands == NULL)
  {
    error(1, 0, "Unexpected Null commands");
    return 0;
  }



  int capacity = 256;
  int numCommands = 1;
/*
  //variables used to traverse through words in simple command
  int wordCount = -1;
  int letterCount = 0;
  unsigned int wordCapacity = 0;
  unsigned int letterCapacity = 0;
  int isOutput = 0;
  int isInput = 0;

  //Iterate over parsed file to create commands
  int i = 0;
  for(i=0; i < size; i++)
  {
    printf("iteration#%d\t",i);
    if(numCommands >= capacity)
    {
      unsigned int current_capacity = sizeof(command_t)*capacity;
      checked_grow_alloc((void*)commands, &current_capacity);
      capacity = current_capacity/sizeof(command_t);
    }

    command_t temp = (command_t)checked_malloc(sizeof(struct command));
    temp->status = -1;
    temp->input = NULL;
    temp->output = NULL;
    temp->u.command[0] = NULL;
    temp->u.command[1] = NULL;
    temp->u.word = NULL;
    temp->u.subshell_command = NULL;

    char c = parsedFile[i];
    if( c == '&')
    {
      temp->type = AND_COMMAND;
      commands[numCommands] = temp;
      numCommands++;
      wordCount = -1;
      letterCount = 0;
      wordCapacity = 0;
      letterCapacity = 0;
      isOutput = 0;
      isInput = 0;
    }
    else if(c == '|')
    {
      if(parsedFile[i+1] == '|')
      {
        temp->type = OR_COMMAND;
      }
      else
      {
        temp->type = PIPE_COMMAND;
      }
      commands[numCommands] = temp;
      numCommands++;
      wordCount = -1;
      letterCount = 0;
      wordCapacity = 0;
      letterCapacity = 0;
      isOutput = 0;
      isInput = 0;
    }
    else if(c == ';')
    {
      temp->type = SEQUENCE_COMMAND;
      commands[numCommands] = temp;
      numCommands++;
      wordCount = -1;
      letterCount = 0;
      wordCapacity = 0;
      letterCapacity = 0;
      isOutput = 0;
      isInput = 0;
    }
    else if(c == '(')
    {
      temp->type = SUBSHELL_COMMAND;
      commands[numCommands] = temp;
      numCommands++;
      wordCount = -1;
      letterCount = 0;
      wordCapacity = 0;
      letterCapacity = 0;
      isOutput = 0;
      isInput = 0;
    }
    else if(c == ')')
    {
      free(temp);
      //Point to empty command stored at the front of the array
      commands[numCommands] = commands[0]; 
      numCommands++;
      wordCount = -1;
      letterCount = 0;
      wordCapacity = 0;
      letterCapacity = 0;
      isOutput = 0;
      isInput = 0;
    }
    else if (c == ' ')
    {
      free(temp);
      if(isOutput || isInput)
      {
        isOutput = 0;
        isInput = 0;
      }
      else if(wordCount > -1)
      {
        wordCount++;
        if(wordCount >= (int)(wordCapacity-1))
        {
          unsigned int wordCapNew = wordCapacity*sizeof(char*);
          checked_grow_alloc(commands[numCommands-1]->u.word, &wordCapNew);
          wordCapacity = wordCapNew/sizeof(char*);
        }

        printf("\tword count increased to %d\n",wordCount);

        commands[numCommands-1]->u.word[wordCount] = (char*)checked_malloc(sizeof(char)*2);
        commands[numCommands-1]->u.word[wordCount][1] = 0;
        commands[numCommands-1]->u.word[wordCapacity+1]=NULL;
      }
      letterCapacity = 2;
      letterCount = 0;
      continue;
    }
    else if (c == '\n')
    {
      free(temp);
      commands[numCommands] = NULL;
      numCommands++;
      wordCount = -1;
      letterCount = 0;
      wordCapacity = 0;
      letterCapacity = 0;
      isOutput = 0;
      isInput = 0;
    }
    else
    {
      if(c == '>' || isOutput)
      {
        free(temp);
        isOutput = 1;
        if(letterCount >= (int)(letterCapacity-1))
        {
          checked_grow_alloc(commands[numCommands-1]->output, &letterCapacity);
        }
        commands[numCommands-1]->output[letterCount] = c;
        commands[numCommands-1]->input[letterCount+1] = 0;
        letterCount++;
      }
      else if(c == '<' || isInput)
      {
        if(c == '<')
        {
          isInput = 1;
        }
        else
        {
          printf("\n\tletter count = %d && letterCapacity = %d", letterCount, letterCapacity);
          if(letterCount >= (int)(letterCapacity-1))
          {
            checked_grow_alloc(commands[numCommands-1]->input, &letterCapacity);
          }
          commands[numCommands-1]->input[letterCount] = c;
          commands[numCommands-1]->input[letterCount+1] = 0;
          letterCount++;
        }
        free(temp);
      }
      else
      {
        if(wordCount == -1)
        {
          wordCount++;
          temp->type = SIMPLE_COMMAND;
          //initialize word
          temp->u.word = (char**)checked_malloc(sizeof(char*)*2);
          temp->u.word[0] = (char*)checked_malloc(sizeof(char)*2);
          temp->u.word[0][1] = 0;
          temp->u.word[1] = NULL;

          //initialize input and output
          temp->input  = (char*)checked_malloc(sizeof(char));
          temp->output = (char*)checked_malloc(sizeof(char));

          letterCapacity = 2;
          wordCapacity = 2;

          //add to list
          commands[numCommands] = temp;
          numCommands++;
        }
        else
        {
          free(temp);
        }
        if(letterCount >= (int)(letterCapacity-1))
        {
          checked_grow_alloc(commands[numCommands-1]->u.word[wordCount], &letterCapacity);
        }
        printf("\tcharacter = %c\n",c);
        commands[numCommands-1]->u.word[wordCount][letterCount] = c;
        commands[numCommands-1]->u.word[wordCount][letterCount+1] = 0;
        letterCount++;
      }
    }
  }*/
  return numCommands;
}

/*
 * Creates a forest structure of commands from the given
 * set of commands.
 * Parses the commands in order and links them using stack push
 * and pop.
 */
command_stream_t linkCommands(command_t* commands, int numCommands)
{
  command_stream_t commandForest = NULL;
  return commandForest;
}

command_stream_t make_command_stream (int (*get_next_byte) (void *), void *get_next_byte_argument)
{

  // Create a syntax parsed array of characters and get it size
  unsigned int capacity = 256;
  char* parsedFile = (char*) checked_malloc(capacity);
  int size = parseFile(get_next_byte, get_next_byte_argument, parsedFile);
  
  if(!parsedFile)
  {
    error (1, 0, "parsed file is null");
  }

  printf("size = %d\n%s\n\n", size, parsedFile);

  // Create an array of commands from the array of parsed characters
  command_t *initCommandTree = (command_t*)checked_malloc(capacity*sizeof(command_t));
  command_t closeParenPointer = (command_t)checked_malloc(sizeof(struct command));
  initCommandTree[0] = closeParenPointer;
  int numCommands = createCommandTree(parsedFile, size, initCommandTree);
  if(!initCommandTree)
  {
    error (1, 0, "initial command tree is null");
  }
  //printf("number of commands = %d", numCommands);

  // Create an array of trees with each tree representing a single command
  command_stream_t commandForest = linkCommands(initCommandTree, size);
  return commandForest;
}

command_t read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
  error (1, 0, "command reading not yet implemented");
  return 0;
}
