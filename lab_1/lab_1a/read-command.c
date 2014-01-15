// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include "alloc.h"
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

int isValidCharacter(char c)
{
  if( isalnum(c) || isspace(c) || c == '!' || c == '%'
       || c == '+' || c == ',' || c == '-' || c == '.'
       || c == '/' || c == ':' || c == '@' || c == '^'
       || c == '_' || c == '|' || c == '&' || c == '('
       || c == ')' || c == '<' || c == '>')
  {
    return 1;
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
  if(nextInput == '&')
  {
    int tempCounter = 0;
    int parsePointer = initSize-1;
    int spaceFound = 0;
    while(parsePointer > 0)
    {
      if( parsedFile[parsePointer] == '|' || parsedFile[parsePointer] == ';')
      {
        return 0;
      }
      else if( isspace(parsedFile[parsePointer]))
      {
        spaceFound = 1;
      }
      else if(parsedFile[parsePointer] == '(')
      {
        return 0;
      }
      else if( parsedFile[parsePointer] == '&')
      {
        tempCounter++;
        if(spaceFound)
        {
          return 0;
        }
        else if(tempCounter > 1)
        {
          return 0;
        }
      }
      else if( isValidCharacter(parsedFile[parsePointer]) )
      {
        break;
      }
      parsePointer--;
    }
  }
  else if(nextInput == '|')
  {
    int tempCounter = 0;
    int parsePointer = initSize-1;
    int spaceFound = 0;
    while(parsePointer > 0)
    {
      if( parsedFile[parsePointer] == '&' || parsedFile[parsePointer] == ';')
      {
        return 0;
      }
      else if( isspace(parsedFile[parsePointer]) )
      {
        spaceFound = 1;
      }
      else if(parsedFile[parsePointer] == '(')
      {
        return 0;
      }
      else if( parsedFile[parsePointer] == '|')
      {
        tempCounter++;
        if(spaceFound)
        {
          return 0;
        }
        else if(tempCounter > 1)
        {
          return 0;
        }
      }
      else if( isValidCharacter(parsedFile[parsePointer]) )
      {
        break;
      }
      parsePointer--;
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

  int parenCount = 0;
  char c;
  int isComment = 0;
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
      if(parsedFile[parsePointer] == ' '){
        if(parsePointer-1 >= 0){
          if(parsedFile[parsePointer-1] == '&' || parsedFile[parsePointer-1] == '|')
          {
            continue;
          }
        }
        parsedFile[parsePointer] = '\n';
        continue;
      }
      if(parsedFile[parsePointer] == '&' || parsedFile[parsePointer] == '|'){
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

    if(!(size < (signed int)capacity))
    {
      checked_grow_alloc((void*)parsedFile, &capacity);
    }

    //Add Valid Character to Array
    parsedFile[size] = c;
    printf("size = %d char = %c\n",size, c);

    //Increment character count
    size++;
  }

  //add end of file
  //if(!(size < (signed int)capacity))
  //{
  //  checked_grow_alloc(parsedFile, &capacity);
  //}

  //Add Valid Character to Array
  //parsedFile[size] = c;

  if(parenCount > 0)
  {
    error (1, 0, "Improper Syntax in File: Unclosed Parentheses");
    return 0;
  }

  return size;
}

/*
 * Use parsedFile to create a set of command structs 
 * for each proper command in order. 
 * Returns the number of commands found.
 */
int createCommandTree(char* parsedFile, int size, command_t commands)
{
  return 0;
}

/*
 * Creates a forest structure of commands from the given
 * set of commands.
 * Parses the commands in order and links them using stack push
 * and pop.
 */
command_stream_t linkCommands(command_t commands, int numCommands)
{
  command_stream_t commandForest = NULL;
  return commandForest;
}

command_stream_t make_command_stream (int (*get_next_byte) (void *), void *get_next_byte_argument)
{

  // Create a syntax parsed array of characters and get it size
  unsigned int capacity = 256;
  char* parsedFile = parsedFile = (char*) checked_malloc(capacity);
  int size = parseFile(get_next_byte, get_next_byte_argument, parsedFile);
  
  printf("size = %d\n%s", size, parsedFile);
  if(!parsedFile)
  {
    error (1, 0, "parsed file is null");
  }

  // Creeate an array of commands from the array of parsed characters
  command_t initCommandTree = NULL;
  int numCommands = createCommandTree(parsedFile, size, initCommandTree);
  if(!initCommandTree)
  {
    error (1, 0, "initial command tree is null");
  }

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
