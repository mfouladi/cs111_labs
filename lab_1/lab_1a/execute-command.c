// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


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
	  else{
	  enum command_type type = c->type;
	  if(type == AND_COMMAND){
	  	command_t left_command = c->u.command[0];
	  	execute_command(left_command,0);
	  	if(command_status(left_command) >= 0){
	  		command_t right_command = c->u.command[1];
	  		execute_command(right_command,0);
	  	}
	  }else if(type == SEQUENCE_COMMAND){
	  	command_t left_command = c->u.command[0];
	  	command_t right_command = c->u.command[1];
	  	execute_command(left_command,0);
	  	execute_command(right_command,0);
	  }else if(type == OR_COMMAND){
	  	command_t left_command = c->u.command[0];
	  	execute_command(left_command,0);
	  	if(command_status(left_command) < 0){
	  		command_t right_command = c->u.command[1];
	  		execute_command(right_command,0);
	  	}
	  }else if(type == PIPE_COMMAND){
	  	int pipe_arg[2];
	  	pipe(pipe_arg);

	  	c->status = 1;
	    pid_t pid = fork();
        if (pid == 0)
	  	{
	  		close(pipe_arg[0]);
	  		dup2(pipe_arg[1], 1);
	  		fprintf(stderr, "child\n");
	  		execute_command(c->u.command[0],0);
	  		exit(c->u.command[0]->status);
	  	}
	  	else
	  	{
	  		close(pipe_arg[1]);
	  		int pid_status;
	  		waitpid(pid, &pid_status,0);
	  		c->status =  WEXITSTATUS(pid_status);
	  		if(c->status >= 0)
	  		{
	  			int stdin_old = dup(0);
	  			dup2(pipe_arg[0], 0);
		  		execute_command(c->u.command[1],0);
		  		close(pipe_arg[0]);
		  		dup2(stdin_old,0);
	  		}
	  		else
	  		{
		  		fprintf(stderr, "left command failed\n");
	  		}
	  		return;
	  	}
	  }else if(type == SIMPLE_COMMAND){
	  	int pid_status;
	  	c->status = 1;
	  	fprintf(stderr, "before child\n");
	  	pid_t pid = fork();
	  	if (pid == 0)
	  	{
	  		int input_fd, output_fd;
	  		fprintf(stderr, "before input\n");
	  		if(c->input != NULL){
	  			input_fd = open(c->input, O_RDONLY);
	  			if(input_fd < 0)
	  				fprintf(stderr, "error on input file open\n");
	  			dup2(input_fd, 0);
	  		}
	  		fprintf(stderr, "before output\n");
	  		if(c->output != NULL){
	  			output_fd = open(c->output, O_WRONLY);
	  			if(output_fd < 0)
	  				output_fd = open(c->output, O_CREAT|O_WRONLY, S_IWUSR|S_IRUSR);
	  			if(output_fd < 0)
	  				fprintf(stderr, "error on output file open\n");
	  			dup2(output_fd, 1);
	  		}
	  		const char* command_name = c->u.word[0];
		  	int exec_error = execvp(command_name, c->u.word);
		  	exit(exec_error);
	  	}
	  	else
	  	{
	  		waitpid(pid, &pid_status,0);
	  		c->status =  WEXITSTATUS(pid_status);
	  		return;
	  	}
	  	
	  }else if(type == SUBSHELL_COMMAND){
	  	pid_t pid = fork();
	  	int pid_status;
	  	c->status = 1;
	  	if (pid == 0)
	  	{
	  		int input_fd, output_fd; 
	  		if(c->input != NULL){
	  			input_fd = open(c->input, O_RDONLY);
	  			dup2(input_fd, 0);
	  		}
	  		if(c->output != NULL){
	  			output_fd = open(c->input, O_WRONLY);
	  			if(output_fd < 0)
	  				output_fd = open(c->input, O_CREAT, 777);
	  			dup2(output_fd, 1);
	  		}
	  		execute_command(c->u.subshell_command,0);
	  		exit(c->u.subshell_command->status);
	  	}
	  	else
	  	{
	  		waitpid(pid, &pid_status,0);
	  		c->status =  WEXITSTATUS(pid_status);
	  		return;
	  	}
	  }
	}
}
