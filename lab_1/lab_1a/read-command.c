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
  size_t capacity = 256;

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
    // If the beginning of a comment is reached
    // set the comment flag to one and continue 
    // through the loop
    if(c == '#')
    {
      isComment = 1;
      continue;
    }

    // If comment flag is set and the current character is not
    // a new line, then skip character and continue loop
    // Otherwise, if it is a new line, unset the comment flag
    if(isComment && c != '\n'){
      continue;
    }else if(isComment){
      isComment = 0;
    }

    // If character is a new line, check if it is an 
    // extra or arbitrary new line. If it is, then
    // remove it. Otherwise, if its any other arbitrary
    // white space, also remove it. If not a white space,
    // then check grammar.
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
    else
    {
      if(!isProperGrammar(parsedFile, size, &parenCount, c))
      {
        error (1, 0, "Improper Syntax in File");
      }
    }

    // If an I/O flag has been set to find
    // single file name, then check for valid
    // file name. Otherwise check if current character is
    // I/O character 
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
      parsedFile = (char*)checked_grow_alloc((void*)parsedFile, &capacity);
    }

    // Remove any extra white space before 
    // New lines and close parentheses
    if(c == '>' || c == '\n')
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

    // Add space before any special characters
    if(size > 0)
    {
      if(c == ')' || c == ';' || c == '(' || c == '\n' || c == '>' || c == '<' )
      {
        if(parsedFile[size-1] != ' ')
        {
          parsedFile[size] = ' ';
          size++;
        }
      }
      else if( c == '&' || c == '|')
      {
        if(parsedFile[size-1] != ' ' && parsedFile[size-1] != c)
        {
          parsedFile[size] = ' ';
          size++;
        }
      }
      if(!(size < (signed int)capacity))
      {
        parsedFile = (char*)checked_grow_alloc((void*)parsedFile, &capacity);
      }
    }
    //Add Valid Character to Array
    parsedFile[size] = c;

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
    parsedFile = (char*)checked_grow_alloc((void*)parsedFile, &capacity);
  }

  //Add Null Character to End of Array
  parsedFile[size] = '\0';
  //size++;

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

  //Command Count - First command is close parentheses
  int numCommands = 1;

  //variables used to traverse through words in simple command
  int wordCount = -1;
  size_t wordCapacity = 0;
  int isOutput = 0;
  int isInput = 0;

  //Iterate over parsed file to create commands
  int i = 0;
  for(i=0; i < size; i++)
  {
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
      wordCapacity = 0;
      isOutput = 0;
      isInput = 0;
      //skip next &
      i++;
    }
    else if(c == '|')
    {
      if(parsedFile[i+1] == '|')
      {
        temp->type = OR_COMMAND;
        //skip next |
        i++;
      }
      else
      {
        temp->type = PIPE_COMMAND;
      }
      commands[numCommands] = temp;
      numCommands++;
      wordCount = -1;
      wordCapacity = 0;
      isOutput = 0;
      isInput = 0;
    }
    else if(c == ';')
    {
      temp->type = SEQUENCE_COMMAND;
      commands[numCommands] = temp;
      numCommands++;
      wordCount = -1;
      wordCapacity = 0;
      isOutput = 0;
      isInput = 0;
    }
    else if(c == '(')
    {
      temp->type = SUBSHELL_COMMAND;
      commands[numCommands] = temp;
      numCommands++;
      wordCount = -1;
      wordCapacity = 0;
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
      wordCapacity = 0;
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
        parsedFile[i] = 0;
      }
      else if(wordCount > -1)
      {
        parsedFile[i] = 0;
        wordCount++;
        if((i+1)<size)
        {
          if( parsedFile[i+1] != '&' && parsedFile[i+1] != '|' && parsedFile[i+1] != ';' &&
              parsedFile[i+1] != '(' && parsedFile[i+1] != ')' && parsedFile[i+1] != '>' &&
              parsedFile[i+1] != '<' && !isspace(parsedFile[i+1]))
          {
            if(wordCount >= (int)(wordCapacity-1))
            {
              size_t wordCapNew = wordCapacity*sizeof(char*);
              commands[numCommands-1]->u.word = (char**)checked_grow_alloc(commands[numCommands-1]->u.word, &wordCapNew);
              wordCapacity = wordCapNew/sizeof(char*);
            }
            commands[numCommands-1]->u.word[wordCount] = &(parsedFile[i+1]);
            commands[numCommands-1]->u.word[wordCount+1] = NULL;
          }
        }
      }
    }
    else if (c == '\n')
    {
      free(temp);
      commands[numCommands] = NULL;
      numCommands++;
      wordCount = -1;
      wordCapacity = 0;
      isOutput = 0;
      isInput = 0;
    }
    else
    {
      if(c == '>')
      {
        free(temp);
        isOutput = 1;
        i++;
        while(i < size && isspace(parsedFile[i]))
        {
          i++;
        }
        commands[numCommands-1]->output = &parsedFile[i];
      }
      else if(c == '<')
      {
        free(temp);
        isInput = 1;
        i++;
        while(i < size && isspace(parsedFile[i]))
        {
          i++;
        }
        commands[numCommands-1]->input = &parsedFile[i];
      }
      else
      {
        if(wordCount == -1 && !isOutput && !isInput)
        {
          //initialize word
          temp->type = SIMPLE_COMMAND;
          temp->u.word = (char**)checked_malloc(sizeof(char*)*2);
          temp->u.word[0] = &(parsedFile[i]);
          temp->u.word[1] = NULL;
          commands[numCommands] = temp;
          numCommands++;
          wordCount = 0;
          wordCapacity = 2;
        }
        else
        {
          free(temp);
        }
      }
    }
  }
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
  size_t capacity = 256;
  char* parsedFile = (char*) checked_malloc(capacity);
  int size = parseFile(get_next_byte, get_next_byte_argument, parsedFile);
  
  if(!parsedFile)
  {
    error (1, 0, "parsed file is null");
  }

  // Create an array of commands from the array of parsed characters
  command_t *initCommandTree = (command_t*)checked_malloc(size*sizeof(command_t));
  command_t closeParenPointer = (command_t)checked_malloc(sizeof(struct command));
  initCommandTree[0] = closeParenPointer;
  int numCommands = createCommandTree(parsedFile, size, initCommandTree);
  if(!initCommandTree)
  {
    error (1, 0, "initial command tree is null");
  }

  printf("number of commands = %d\n\n", numCommands);
  int i =0;
  for(i=1; i<numCommands; i++){
    if(initCommandTree[i] == NULL){
      printf("\n");
    }else if(initCommandTree[i] == initCommandTree[0]){
      printf(" ) ");
    }
    else{
      enum command_type type = initCommandTree[i]->type;
      if(type == AND_COMMAND){
        printf(" AND ");
      }else if(type == SEQUENCE_COMMAND){
        printf(" ; ");
      }else if(type == OR_COMMAND){
        printf(" OR ");
      }else if(type == PIPE_COMMAND){
        printf(" PIPE ");
      }else if(type == SIMPLE_COMMAND){
        int j = 0;
        while(initCommandTree[i]->u.word[j] != NULL){
          printf(" %s ", initCommandTree[i]->u.word[j]);
          j++;
        }
        if(initCommandTree[i]->input != NULL){
          printf(" < %s ", initCommandTree[i]->input);
        }
        if(initCommandTree[i]->output != NULL){
          printf(" > %s ", initCommandTree[i]->output);
        }
      }else if(type == SUBSHELL_COMMAND){
        printf(" ( ");
      }
    }
  }
  printf("finished implementation\n");
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
