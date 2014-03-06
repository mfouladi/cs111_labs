// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include "alloc.h"
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#include <error.h>

/*Helper Functions*/

int isWordCharacter(char c)
{
  return (isalnum(c)  || c == '!' || c == '%'
	  || c == '+' || c == ',' || c == '-' 
	  || c == '.' || c == '/' || c == ':' 
	  || c == '@' || c == '^' || c == '_');
}

int isSpecialToken(char c)
{
  return (c == ')' || c == ';' || c == '(' 
	  || c == '>' || c == '<' 
	  || c == '&' || c == '|');
}

/*
 * Checks if the given character is within the set of all
 * valid characters allowed by our interpreter.
 */
int isValidCharacter(char c)
{
  return (isWordCharacter(c) || isspace(c) || isSpecialToken(c));
}

int checkOpenParen(char* parsedFile, char c, int initSize)
{
  if( c != '(')
  {
    error(1, 0, "Improper call to checkOpenParen");
    return 0;
  }

  int i;
  for(i = initSize-1; i >= 0; i--)
  {
    if(parsedFile[i] == ';' || parsedFile[i] == '|' || parsedFile[i] == '&' || parsedFile[i] == '(' || parsedFile[i] == '\n')
      return 1;
    else if(parsedFile[i] == ' ')
      continue;
    else
      return 0;
  }
  return 0;
}

int checkCloseParen(char* parsedFile, char c, int initSize, int parenCount)
{
  if( c != ')')
  {
    error(1, 0, "Improper call to checkCloseParen");
    return 0;
  }

  if (parenCount == 0)
    return 0;

  int i;
  for(i = initSize-1; i >= 0; i--)
  {
    if(isWordCharacter(parsedFile[i]) || parsedFile[i] == ')' || parsedFile[i] == ';')
      return 1;
    else if(isspace(parsedFile[i]))
      continue;
    else
      return 0;
  }
  return 0;
}

int checkSpecialToken(char* parsedFile, char c, int initSize)
{
  if(c != '&' && c != '|' && c != ';' && c != '<' && c != '>')
  {
    error(1, 0, "Improper call to checkSpecialToken");
    return 0;
  }

  // The previously added char has already been checked
  if ((c == '|' || c == '&') && parsedFile[initSize-1] == c && parsedFile[initSize-2] != c)
    return 1;

  //printf("DEBUG: checkSpecialToken: initSize=%i\n", initSize);
  int i;
  for (i = initSize-1; i >= 0; i--) 
    {
      //printf("DEBUG: checkSpecialToken: checking %c\n", parsedFile[parsePointer]);
      if (isWordCharacter(parsedFile[i]) || parsedFile[i] == ')')
	return 1;
      else if (parsedFile[i] == ' ')
	continue;
      else
	return 0;
    }
  //printf("DEBUG: checkSpecialToken: error 1\n");
  return 0;
}

/*
 * Checks if next char value is valid in respect to current set of values
 * if next char is valid, it returns 1
 * if next char is invalid, it returns 0
 */
int isProperGrammar(char* parsedFile, int initSize, int parenCount, char nextInput)
{
  if(!isValidCharacter(nextInput))
    {
      return 0;
    }
  
  if (isWordCharacter(nextInput) || isspace(nextInput))
    return 1;

  // if its the first character, 
  // then it must be a non-special character
  if(initSize == 0)
    {
      if(isSpecialToken(nextInput) && nextInput != '(')
	{
	  return 0;
	}
      else
	return 1;
    }
  
  if(initSize > 1 && nextInput != '&' && parsedFile[initSize-1] == '&' && parsedFile[initSize-2] != '&')
    {
      return 0;
    }

  // If & or | follows a different special character
  // or there are three of these back to back without
  // intermediate valid characters
  // Then return false  
  if(nextInput == '&' || nextInput == '|' || nextInput == ';' || nextInput == '<' || nextInput == '>')
    return checkSpecialToken(parsedFile, nextInput, initSize);

  if (nextInput == '(')
    return checkOpenParen(parsedFile, nextInput, initSize);

  if (nextInput == ')')
    return checkCloseParen(parsedFile, nextInput, initSize, parenCount);

  return 1;
}

/*
 * Fill parseFile with valid character array input
 * Remove all white space and comments
 * Return the size of the array
 */
int parseFile(int (*get_next_byte) (void *), void *get_next_byte_argument, char* parsedFile, int script_varc, char** script_varv)
{
  //printf("DEBUG: parseFile\n");

  int size = 0;
  size_t capacity = 512;

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

  int isVariable = 0;

  //Used to check for valid redirection
  int findSingleWord = 0;
  int singleWordStarted = 0;
  int foundLessThan = 0;
  int foundSingleWord = 0;

  while( (c = get_next_byte(get_next_byte_argument)) && c != 0 && !feof(get_next_byte_argument))
    {
      //printf("DEBUG: parseFile: checking %c\n", c);
      
      // If comment flag is set and the current character is not
      // a new line, then skip character and continue loop
      // Otherwise, if it is a new line, unset the comment flag
      if(isComment)
	{
	  if (c == '\n')
	    isComment = 0;
	  else
	    continue;
	}

      if (isVariable)
	{
	  if (c < '0' || c > '9')
	    {
	      error (1, 0, "Error: variables must be integers");
	      return 0;
	    }

	  int i = c - '0';
	  if (i > script_varc)
	    {
	      error (1, 0, "Error: not enough args");
	      return 0;
	    }

	  char *arg = script_varv[i];
	  // DEBUG
	  //printf("arg%i:\n", i);
	  while (*arg != '\0')
	    {
	      // DEBUG
	      //printf("%c", *arg);

	      if (isWordCharacter(*arg) || isspace(*arg)) 
		parsedFile[size++] = *(arg++);
	      else if (isSpecialToken(*arg))
		error (1, 0, "Error: script arg contains special tokens! \nWhat exactly are you trying to do here...?");
	      else
		error (1, 0, "Error: script arg contains invalid characters");

	      if(size >= (signed int) capacity)
		{
		  parsedFile = (char*)checked_grow_alloc((void*)parsedFile, &capacity);
		}
	    }
	  // DEBUG
	  //printf("\n");
	  isVariable = 0;
	  continue;
	}

      if (c == '$')
	{
	  isVariable = 1;
	  continue;
	}

      // If the beginning of a comment is reached
      // set the comment flag to one and continue 
      // through the loop
      if(c == '#')
	{
	  isComment = 1;
	  continue;
	}

      if (c == ';' && parenCount == 0)
	c = '\n';

      // If character is a new line, check if it is an 
      // extra or arbitrary new line. If it is, then
      // remove it. Otherwise, if its any other arbitrary
      // white space, also remove it. If not a white space,
      // then check grammar.
      if (isspace(c))
	{
	  if(c == '\n')
	    {
	      if (size == 0 || parsedFile[size-1] == '\n'
		  || parsedFile[size-1] == '&' || parsedFile[size-1] == '|' || parsedFile[size-1] == ';')
		continue;
	      if (parsedFile[size-1] == ' ')
		{
		  if (parsedFile[size-2] == '&' || parsedFile[size-2] == '|' || parsedFile[size-2] == ';')
		      continue;
		  else
		    size--;
		}
	      if (parenCount > 0)
		c = ';';
	    }
	  else
	    {
	      if(size == 0 || parsedFile[size-1] == ' ' || parsedFile[size-1] == '\n')
		continue;
	      c = ' ';
	    }
	}

      // Main grammar checker
      if(!isProperGrammar(parsedFile, size, parenCount, c))
	{
	  error (1, 0, "Improper Syntax in File: bad grammar");
	  return 0;
	}
      
      if (c == '(')
	parenCount++;
      else if (c == ')')
	parenCount--;
  
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

      // Add space before and after any special characters
      if (((isSpecialToken(c) && parsedFile[size-1] != c) 
	   || (isWordCharacter(c) && isSpecialToken(parsedFile[size-1])))
	  && !isspace(parsedFile[size-1])) 
	{
	  parsedFile[size] = ' ';
	  size++;
	  if(size >= (signed int) capacity)
	    {
	      parsedFile = (char*)checked_grow_alloc((void*)parsedFile, &capacity);
	    }
	}
      
      //Add Valid Character to Array
      parsedFile[size] = c;
      
      //Increment character count
      size++;
      
      if(size == (signed int)capacity)
	parsedFile = (char*)checked_grow_alloc((void*)parsedFile, &capacity);
    }
  
  //Check if file ended properly
  if(parsedFile[size-1] != ';' && parsedFile[size-1]!='\n' && !isProperGrammar(parsedFile, size, parenCount, ';'))
    {
      error (1, 0, "Improper Syntax in File: File terminated incorrectly");
    }

  //Add Null Character to End of Array
  parsedFile[size] = '\0';
  //size++;
  
  if(parenCount > 0)
    {
      error (1, 0, "Improper Syntax in File: Unclosed Parentheses");
      return 0;
    }
  
  // DEBUG
  //printf("%s\n", parsedFile);
  return size;
}

/*
 * Use parsedFile to create a set of command structs 
 * for each proper command in order. 
 * Returns the number of commands found.
 */
int createCommandTree(char* parsedFile, int size, command_t* commands)
{
  // Design
  //printf("DEBUG: CreateCommandTree\n");
  
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
  command_t temp;
  for(i=0; i < size; i++)
  {
    //printf("%c", parsedFile[i]);

    temp = (command_t)checked_malloc(sizeof(struct command));
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
      parsedFile[i] = '\0';
      if(isOutput || isInput)
      {
        isOutput = 0;
        isInput = 0;
      }
      else if(wordCount > -1)
      {
        wordCount++;
        if((i+1)<size)
        {
          if(isWordCharacter(parsedFile[i+1]))
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
    else if (c == '\n' || c == EOF)
    {
      free(temp);
      parsedFile[i] = '\0';
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
        if(commands[numCommands-1] == commands[0])
        {
          int lastCommand;
          for(lastCommand = numCommands-2; lastCommand>0; lastCommand--)
	    {
	      if(commands[lastCommand]->type == SUBSHELL_COMMAND)
		{
		  commands[lastCommand]->output = &parsedFile[i]; 
		}
	    }
        }
        else
        {
          commands[numCommands-1]->output = &parsedFile[i]; 
        }
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
        if(commands[numCommands-1] == commands[0])
        {
          int lastCommand;
          for(lastCommand = numCommands-2; lastCommand>0; lastCommand--){
            if(commands[lastCommand]->type == SUBSHELL_COMMAND){
              commands[lastCommand]->input = &parsedFile[i]; 
            }
          }
        }
        else
        {
          commands[numCommands-1]->input = &parsedFile[i]; 
        }
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
          commands[numCommands++] = temp;
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
  // End for loop
  if(commands[numCommands-1] != NULL)
  {
      commands[numCommands] = NULL;
      numCommands++;
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
  command_stream_t commandForest = (command_stream_t) checked_malloc (sizeof(struct command_stream));
  commandForest->commands = (command_t*) checked_malloc (numCommands*sizeof(command_stream_t));
  commandForest->cst_it = commandForest->commands;
  commandForest->size = 0;

  commandForest->io_files = (char**) checked_malloc(numCommands*sizeof(char*));
  commandForest->file_dependencies = (int*) checked_malloc(numCommands*sizeof(int));
  commandForest->io_files_size = 0;

  command_t operatorStack[numCommands];
  int operators = 0;

  command_t operandStack[numCommands];
  int operands = 0;

  command_t closedParan = commands[0];

  int i;
  int subshell_depth = 0;

  for (i=1; i<numCommands; i++)
    {
      if (commands[i] != NULL)
	{
	  if (commands[i] == closedParan)
	    subshell_depth--;
	  else if (commands[i]->type == SUBSHELL_COMMAND)
	    subshell_depth++;
	  else if (subshell_depth == 0 && commands[i]->type == SEQUENCE_COMMAND)
	    commands[i] = NULL;
	}
    }


  //printf("%i\n", numCommands);
  
  for (i=1; i<numCommands; i++)
    {
      // This command points to the closed parenthesis
      if (commands[i] == closedParan)
	{
	  while (1)
	    {
	      command_t op = operatorStack[--operators];
	      if (op->type == SUBSHELL_COMMAND)
		{
		  op->u.subshell_command = operandStack[operands-1];
		  operandStack[operands-1] = op;
		  break;
		}
	      else
		{
		  op->u.command[1] = operandStack[--operands];
		  op->u.command[0] = operandStack[--operands];
		  operandStack[operands++] = op;
		}
	    }
	}
      // This is the end of commands for this tree
      else if (commands[i] == NULL) 
	{
	  while (operators > 0)
	    {
	      command_t op = operatorStack[--operators];
	      op->u.command[1] = operandStack[--operands];
	      op->u.command[0] = operandStack[--operands];
	      operandStack[operands++] = op;
	    }
	  if (operands != 1) printf("Linking algorithm error!\n");
	  commandForest->commands[commandForest->size++] = operandStack[--operands];
	}
      // This command points to an actual command
      else
	{
	  switch (commands[i]->type)
	    {
	    case SIMPLE_COMMAND:
	      operandStack[operands++] = commands[i];
	      break;
	    case AND_COMMAND:
	    case SEQUENCE_COMMAND:
	    case OR_COMMAND:
	      if (operators > 0 && operatorStack[operators-1]->type != SUBSHELL_COMMAND)
		{
		  command_t op = operatorStack[--operators];
		  op->u.command[1] = operandStack[--operands];
		  op->u.command[0] = operandStack[--operands];
		  operandStack[operands++] = op;
		}
	      operatorStack[operators++] = commands[i];
	      break;
	    case PIPE_COMMAND:
	      if (operators > 0 && operatorStack[operators-1]->type == PIPE_COMMAND)
		{
		  command_t op = operatorStack[--operators];
		  op->u.command[1] = operandStack[--operands];
		  op->u.command[0] = operandStack[--operands];
		  operandStack[operands++] = op;
		}
	      operatorStack[operators++] = commands[i];
	      break;
	    case SUBSHELL_COMMAND:
	      operatorStack[operators++] = commands[i];
	      break;
	    default:
	      printf("Error: linkCommands could not read from initCommandTree");
	    }
	}
    }

  return commandForest;
}

command_stream_t make_command_stream (int (*get_next_byte) (void *), void *get_next_byte_argument, int script_varc, char** script_varv)
{
  //printf("DEBUG\n");

  // Create a syntax parsed array of characters and get it size
  size_t capacity = 256;
  char* parsedFile = (char*) checked_malloc(capacity);
  int size = parseFile(get_next_byte, get_next_byte_argument, parsedFile, script_varc, script_varv);
  
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
  // Create an array of trees with each tree representing a single command
  command_stream_t commandForest = linkCommands(initCommandTree, numCommands);

  return commandForest;
}

command_t read_command_stream (command_stream_t s)
{
  if (s->cst_it - s->commands >= s->size)
    return 0;
  return *(s->cst_it++);
}
