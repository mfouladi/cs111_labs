// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"
#include "alloc.h"

#include <string.h>
#include <error.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int command_status (command_t c)
{
  return c->status;
}

void
execute_command (command_t c, int timetravel)
{
  if (timetravel == 0)
    {
      enum command_type type = c->type;
      if(type == AND_COMMAND){
	command_t left_command = c->u.command[0];
	execute_command(left_command, 0);
	c->status = left_command->status;
	if(c->status == 0){
	  command_t right_command = c->u.command[1];
	  execute_command(right_command, 0);
	  c->status = right_command->status;
	}
      }else if(type == SEQUENCE_COMMAND){
	command_t left_command = c->u.command[0];
	command_t right_command = c->u.command[1];
	execute_command(left_command,0);
	execute_command(right_command,0);
	c->status = right_command->status;
      }else if(type == OR_COMMAND){
	command_t left_command = c->u.command[0];
	execute_command(left_command,0);
	c->status = left_command->status;
	if(command_status(left_command) > 0){
	  command_t right_command = c->u.command[1];
	  execute_command(right_command,0);
	  c->status = right_command->status;
	}
      }else if(type == PIPE_COMMAND){
	command_t left_command = c->u.command[0];
	command_t right_command = c->u.command[1];
	int pipe_arg[2];
	pipe(pipe_arg);
	
	int stdout_old = dup(1);
	dup2(pipe_arg[1], 1);
	execute_command(left_command, 0);
	dup2(stdout_old, 1);
	close(pipe_arg[1]);

	int stdin_old = dup(0);
	dup2(pipe_arg[0], 0);
	execute_command(right_command, 0);
	dup2(stdin_old, 0);
	close(pipe_arg[0]);
	c->status = right_command->status;
	
      }else if(type == SUBSHELL_COMMAND){

	int input_fd, output_fd, stdout_old, stdin_old; 
	if(c->input != NULL){
	  stdin_old = dup(0);
	  input_fd = open(c->input, O_RDONLY);
	  if(input_fd < 0)
	    {
	      fprintf(stderr, "-timetrash: couldn't open %s\n", c->input);
	      c->input = NULL;
	    }
	  else 
	    dup2(input_fd, 0);
	}
	
	if(c->output != NULL){
	  stdout_old = dup(1);
	  output_fd = open(c->output, O_WRONLY);
	  if(output_fd < 0)
	    output_fd = open(c->output, O_CREAT|O_WRONLY, S_IWUSR|S_IRUSR);
	  if(output_fd < 0)
	    {
	      fprintf(stderr, "execute_command: couldn't open %s\n", c->output);
	      c->output = NULL;
	    }
	  else
	    dup2(output_fd, 1);
	}

	execute_command(c->u.subshell_command,0);
	
	if (c->input != NULL) {
	    dup2 (stdin_old, 0);
	    close (input_fd);
	}
	if (c->output != NULL) {
	    dup2 (stdout_old, 1);
	    close (output_fd);
	}
	c->status = c->u.subshell_command->status;

      }else if(type == SIMPLE_COMMAND){
	//fprintf(stderr, "before child\n");
	pid_t pid = fork();
	if (pid == 0)
	  {
	    int input_fd, output_fd;
	    //fprintf(stderr, "before input\n");
	    if(c->input != NULL){
	      input_fd = open(c->input, O_RDONLY);
	      if(input_fd < 0)
		{
		  fprintf(stderr, "execute_command: couldn't open %s\n", c->input);
		}
	      else
		dup2(input_fd, 0);
	    }

	    if(c->output != NULL){
	      output_fd = open(c->output, O_WRONLY);
	      if(output_fd < 0)
		output_fd = open(c->output, O_CREAT|O_WRONLY, S_IWUSR|S_IRUSR);
	      if(output_fd < 0)
		{
		  fprintf(stderr, "execute_command: couldn't open %s\n", c->output);
		}
	      else
		dup2(output_fd, 1);
	    }

	    execvp(c->u.word[0], c->u.word);

	    // execvp should not return
	    exit(errno);
	  }
	else if (pid > 0)
	  {
	    int pid_status;
	    waitpid(pid, &pid_status,0);
	    //fprintf(stderr, "child done\n");
	    if (WIFEXITED(pid_status))
	      c->status =  WEXITSTATUS(pid_status);
	    else
	      c->status = 1;
	  }
	else
	  {
	    fprintf(stderr, "execute_command: fork() failed!\n");
	  }
      }
    }
}


void make_time_travel_stream (command_stream_t s)
{
  //printf("make_time_travel_stream\n");
  s->requirement_matrix = (int**) checked_malloc (s->size*sizeof(int*));
  s->requirement_array = (int*) checked_malloc (s->size*sizeof(int));
  s->pid_array = (pid_t*) checked_malloc (s->size*sizeof(pid_t));
  s->started = (int*) checked_malloc (s->size*sizeof(int));

  int i, j, sum;
  //printf("cmds: %i\n", s->size);
  for (i=0; i < s->size; i++)
    {
      s->requirement_matrix[i] = (int*) checked_calloc (s->size*sizeof(int));

      build_dependencies(s->commands[i], s, i);

      sum = 0;
      for (j=0; j<s->size; j++)
	{
	  sum += s->requirement_matrix[i][j];
	}

      if (sum > i) fprintf(stderr, "requirement matric error!\n");
      s->requirement_array[i] = sum;

    }
  /*
  printf("DEBUG: io_files:\n");
  for (i=0; i < s->io_files_size; i++)
    {
      printf("%s\n", s->io_files[i]);
    }

  printf("DEBUG: requirement_matrix:\n");
  for (i=0; i < s->size; i++)
    {
      for (j=0; j < s->size; j++)
	{
	  printf("%i ", s->requirement_matrix[i][j]);
	}
      printf("= %i\n", s->requirement_array[i]);
    }
  */
}

void
build_dependencies (command_t c, command_stream_t s, int index)
{
  //printf("build_dependencies\n");
  if (c->type == SUBSHELL_COMMAND || c->type == SIMPLE_COMMAND)
    {
      int i;
      if (c->input != NULL)
	{
	  for (i = 0; i < s->io_files_size; i++)
	    {
	      // This file is already in the array
	      if (strcmp(s->io_files[i], c->input) == 0)
		{
		  // Mark the requirement matrix
		  if (s->file_dependencies[i] != index)
		    s->requirement_matrix[index][s->file_dependencies[i]] = 1;
		  // Update this file's most recent dependency
		  s->file_dependencies[i] = index;
		  break;
		}
	    }
	  // This file is not in the array
	  if (i == s->io_files_size)
	    {
	      // Create an entry in the dependency array
	      s->io_files[s->io_files_size] = c->input;
	      s->file_dependencies[s->io_files_size++] = index;
	    }
	}

      if (c->output != NULL)
	{
	  for (i=0;i<s->io_files_size;i++)
	    {
	      if (strcmp(s->io_files[i],c->output)==0)
		{
		  if (s->file_dependencies[i] != index)
		    s->requirement_matrix[index][s->file_dependencies[i]] = 1;
		  s->file_dependencies[i] = index;
		  break;
		}
	    }
	  if (i==s->io_files_size)
	    {
	      s->io_files[s->io_files_size] = c->output;
	      s->file_dependencies[s->io_files_size++] = index;
	    }

	}
      if (c->type == SUBSHELL_COMMAND)
	{
	  build_dependencies(c->u.subshell_command, s, index); 
	}
    }
  else if (c->type == AND_COMMAND || c->type == SEQUENCE_COMMAND || c->type == OR_COMMAND || c->type == PIPE_COMMAND)
    {
      build_dependencies(c->u.command[0], s, index);
      build_dependencies(c->u.command[1], s, index);
    }
}

void
execute_command_timetravel (command_t c, command_stream_t s, int i)
{
  //printf("execute_command_timetravel\n");
  if (s->requirement_array[i] != 0)
    {
      int j;
      for (j=0; j < i; j++)
	{
	  if (s->requirement_matrix[i][j] != 0 && s->commands[j]->status == -1 && s->pid_array[j] != 0)
	    {
	      int status;
	      //printf("DEBUG: cmd%i waiting for pid=%i\n", i, s->pid_array[j]);
	      
	      int wait = waitpid(s->pid_array[j], &status, WNOHANG);
	      if (wait == s->pid_array[j])
		{
		  //printf("DEBUG: cmd%i unblocked by cmd%i\n", i, j);
		  if (WIFEXITED(status))
		    s->commands[j]->status = WEXITSTATUS(status);
		  else
		    s->commands[j]->status = 1;
		  s->pid_array[j] = 0;
		  
		  int k;
		  for (k=0; k < s->size; k++)
		    if (s->requirement_matrix[k][j] == 1)
		      s->requirement_array[k]--;
		}
	      else if (wait == 0)
		{
		  //printf("DEBUG: cmd%i blocked by cmd%i\n", i, j);
		}
	      else
		fprintf(stderr, "execute_command_timetravel: waitpid() failed\n");
	    }
	}
    }

  if (s->requirement_array[i] == 0)
    {
      pid_t cmd_pid = fork();
      if (cmd_pid == 0)
	{
	  execute_command(c,0);
	  exit (0);
	}
      else if (cmd_pid > 0)
	{
	  //printf("DEBUG: cmd%i pid=%i\n", i, cmd_pid);
	  s->pid_array[i] = cmd_pid;
	  s->started[i] = 1;
	}
      else
	fprintf(stderr, "execute_command_timetravel: fork() failed!\n");
    }
}

void
finish_timetravel (command_stream_t s)
{
  //printf("finish_timetravel\n");
  int blocked = 1; 
  int i;
  while (blocked)
    {
      blocked = 0;
      for (i=0; i < s->size; i++)
	{
	  if (s->started[i] == 0)
	    {
	      blocked = 1;
	      execute_command_timetravel (s->commands[i], s, i);
	    }
	}
      //sleep(1);
    }

  for (i=0; i < s->size; i++)
    {
      int status;
      if (s->pid_array[i] > 0)
	{
	  if (waitpid(s->pid_array[i], &status, 0) > 0)
	    {
	      if (WIFEXITED(status))
		s->commands[i]->status = WEXITSTATUS(status);
	      else
		s->commands[i]->status = 1;
	    }
	  else
	    {
	      fprintf(stderr, "finish_timetravel: waitpid() failed!\n");
	    }
	}
    }
}
