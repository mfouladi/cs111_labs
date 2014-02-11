// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <error.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

int command_status (command_t c)
{
  return c->status;
}

void
execute_command (command_t c, int time_travel)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  if (time_travel == 1)
    print_command (c);
  
  enum command_type type = initCommandTree[i]->type;
  if(type == AND_COMMAND){
  	struct command left_command = initCommandTree[i]->u.command[0];
  	execute_command(left_command);
  	if(command_status(left_command) > 0){
  		struct command right_command = initCommandTree[i]->u.command[1];
  		execute_command(right_command);
  	}
  }else if(type == SEQUENCE_COMMAND){
  	struct command left_command = initCommandTree[i]->u.command[0];
  	struct command right_command = initCommandTree[i]->u.command[1];
  	execute_command(left_command);
  	execute_command(right_command);
  }else if(type == OR_COMMAND){
  	struct command left_command = initCommandTree[i]->u.command[0];
  	execute_command(left_command);
  	if(command_status(left_command) < 0){
  		struct command right_command = initCommandTree[i]->u.command[1];
  		execute_command(right_command);
  	}
  }else if(type == PIPE_COMMAND){

  }else if(type == SIMPLE_COMMAND){
  	const char* command_name = initCommandTree[i]->u.word;
  	char** execut_args;

    initCommandTree[i]->status = 1;
  	//int exec_error = execvp(command_name, );
  	//initCommandTree[i]->status = exec_error;
  }else if(type == SUBSHELL_COMMAND){
  }
	


  error (1, 0, "command execution not yet implemented");
}
