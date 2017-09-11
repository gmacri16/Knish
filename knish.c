/*
Gregory Macri
Knish Shell


This is a shell program that currently can take commands with or without
arguments. The shell has 6 built in commands: help, cd, jobs, kill, history  
and exit. This shell program also has the capability of input and output 
redirection along with the ability to append a file. This shell is also able 
to run background processes and multiple processes specified on one command 
line. This shell is also able to use pipe redirection if specified. 

The extra functionality I have added to the shell is the history command 
along with the ability of the user to scroll through their command line 
history by pushing up or down on the arrow keys. If the user enters the 
history command on their command line, they will get back the history of 
the entire shell session.

One of the major design decisions that I made with this program was the use of 
conditionals that calls the code in the main body of the loop instead of 
calling functions inside the conditionals. The reason I chose to do this 
was to avoid dealing with local variable problems inside the main body and a 
seperate function. This also limits the amount of global variables needed
for the program to function because I don't need to go outside of the main
function.

In terms of assumptions I had going into the program, from the start of 
development I had assumed it was going to be fairly straightforward. It was
a fairly easy, but long development process. I did have doubts initially 
on the severity of the project based on the cpu simulator, though.  
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <readline/readline.h>
#include <readline/history.h>


int status, jobLimit;//holds process status and jobLimit 
int *jobPid;//list of pids for background jobs 
char **jobName;//list of names for background jobs

//signal handler for child processes that finish in the background
void handler(int sig)
{
  pid_t pid;
  int i;

  if(sig == SIGCHLD)//if signal is SIGCHLD
  {
    pid = waitpid(-1,&status,WCONTINUED|WNOHANG|WUNTRACED);//get pid
  
    for(i = 0; i < jobLimit; ++i)//search lists
    {
      if(pid == jobPid[i] && pid != 0)//if pid matches pid in array
      {
	printf("\n[%d]     Done     %s\n", i + 1, jobName[i]);//print
	jobPid[i] = NULL;//clear arrays of entry
	jobName[i] = NULL;//
	break;
      }
    }
  }
}



int main(int argc, char* argv[])
{
  const char delimiters[]= " ,:!";//define the delimiters for strsep 
  char *line, *command, *temp, *inputFile, *outputFile;
  char **args;//creates char pointer array
  pid_t pid;//intialize the pid variable for later use with fork
  int i, count, size, input, output, append, background, more, history;
  int piping, piping2, pipeOut, pipeOut2, pipeIn, pipeIn2;//pipe variables

  int tube[2];//for pipes
  int tube2[2];//
  
  args = (char*)malloc(100*sizeof(char*));//mallocs the size of args as 100
  jobName = (char*)malloc(100*sizeof(char*));//holds background job names
  jobPid = (int)malloc(100*sizeof(int));//holds background job pids
  size = 100;//sets size to 100
  input = -1;//set all variables to false
  output = -1;
  append = -1;
  background = -1;
  more = -1;
  piping = -1;
  piping2 = -1;
  pipeIn = -1;
  pipeIn2 = -1;
  pipeOut = -1;
  pipeOut2 = -1;

  inputFile = NULL;//set strings to NULL
  outputFile = NULL;//

  jobLimit = 100;//set starting job limit to 100

  history = 0;//set history counter to 0

  using_history();//used for history

  signal(SIGCHLD, handler);//set up signal handler for SIGCHLD
  signal(SIGINT, SIG_IGN);//ignore SIGINT signals
  
  for(i = 0; i < jobLimit; ++i)//set all of jobPids and jobNames to NULL
  {
    jobPid[i] = NULL;
    jobName[i] = NULL;
  }





  for(;;)//infinite for loop for shell 
  {
    i = 0;//intialize i and count to zero
    count = 0;


    if(more == -1)//if more false
    {
      line = readline("knish# ");//wait for user input
      add_history(line);//add input to history
      ++history;//increase history count
      temp = strsep(&line, delimiters);//seperate input string
    }


 
    if(pipeOut == 0)//if pipeOut true
    {
      pipeIn = 0;//set pipeIn to true
      pipeOut = -1;//set pipeOut to false
    }

    if(pipeOut2 == 0)//if pipeOut2 true
    {
      pipeIn2 = 0;//set pipeIn2 to true
      pipeOut2 = -1;//set pipeOut2 to true
    }


    more = -1;//set more to false

    

    while(temp != NULL)//while not end of string
    {
      if(strcmp(temp,"") != 0)//if temp not a delimiter
      {
	if(count == size - 1)//if more space needed for args, realloc
	{
	  args = (char*)realloc(args,(size+100)*sizeof(char*));
	  size = size + 100;
	}


	if(strcmp(temp,">") == 0)//if output specified
        {
          output = 0;//set output to true
        }
	else if(strcmp(temp,">>") == 0)//if appending specified
	{
	  output = 0;//set output to true
	  append = 0;//set append to true
	}
        else if(strcmp(temp,"<") == 0)//if input specified
	{
	  input = i - 1;//set input to previous index
	  if(input == -1)//if no previous index, error
	  {
	    printf("No input to read from!\n");
	    count = 0;
	    break;
	  }
	  input = 0;//set input to true
        }
	else if(output == 0)//if output true
	{
	  outputFile = temp;//get outputFile
	  output = 1;//set to true with ouputFile
	}
	else if(input == 0)//if input true
	{
	  inputFile = temp;//get input file
	  input = 1;//set to true with inputFile
	}
	else if(strcmp(temp,"&") == 0)//if & specified
	{
	  background = 0;//set background to true
	  temp = strsep(&line, delimiters);// get next input value
	  if(temp != NULL)//if not end of input
	  {
	    more = 0;//set more to true
	    break;
	  }
	}
	else if(strcmp(temp,";") == 0)//if semicolon specified
	{
	  temp = strsep(&line, delimiters);//get next input value
	  if(temp != NULL)//if not end of input
	  {
	    more = 0;//set more to true
	    break;
	  }
	}
	else if(strcmp(temp,"|") == 0)//if pipe specifier found
	{
	  temp = strsep(&line, delimiters);//get next input value
      
	  if(piping == -1)//if not already using tube
	  { 
	    if(temp != NULL)// if not end of string
	    { 
	      more = 0;//set more to true
	      piping = 0;//set piping to true
	      pipeOut = 0;//set pipeOut to true
	      if(pipe(tube) == -1)//create pipe
		perror("Cannot create pipe\n");
	    }
	    break;
	  }
	  else
	  {
	    if(temp != NULL)
	    {
	      more = 0;//set more to true
	      piping2 = 0;//set piping2 to true
	      pipeOut2 = 0;//set pipeOut2 to true
	      if(pipe(tube2) == -1)//create pipe
		perror("Cannot create pipe\n");
	    }
	    break;
	  }
	}
	else
	{
	  args[i] = temp;//add temp to argument array
	  ++i;//increment i
	  ++count;//increment count
	}
      }      
      temp = strsep(&line,delimiters);//get next input value
    }





    if(count == 0)//if the string was empty
    {
      command = "";//set command to blank string
      remove_history(history - 1);//remove entry from history
      --history;//decrement counter
    }
    else
    {
      command = args[0];//set command to first argument
    }




        
    if(strcmp(command,"exit") == 0)//if command is exit
    {
      return 0;//close shell
    }
    else if(strcmp(command,"help") == 0)//if command is help, print message  
    {
      printf("KNISH SHELL reference implementation by Gregory Macri.\n");
      printf("Available commands:\n");
      printf("exit -- exit from shell\n");
      printf("help -- print this help message\n");
      printf("<cmd> <arg1> <arg2> ... -- run <cmd> in foreground with args\n");
      printf("<cmd> <arg1> <arg2> ... & -- run <cmd> in background with args\n");
      printf("cd <directory> -- change working directory\n");
      printf("jobs -- list background jobs\n");
      printf("kill <pid> <pid> ... -- kill a background process\n\n");
    }
    else if(strcmp(command,"cd") == 0)//if command is cd
    {
      if(chdir(args[1]) == -1)//input argument to chdir, if returns -1, error
	printf("Usage: cd <newdir>\n");
    }
    else if(strcmp(command,"kill") == 0)//if command is kill
    {
      int num;//used to store pid to be killed

      if(count > 1)//if number specified
      {
	for(i = 1; i < count; ++i)
	{
	  num = atoi(args[i]);//get number
	  if(jobPid[num - 1] == NULL)//if job doesn't exist
	  {
	    printf("kill: %d: no such process id\n",num);
	  }
	  else
	  {
	    kill(jobPid[num - 1],SIGTERM);//terminate specified job
	    printf("[%d] \"%s\" Killed\n",num,jobName[num - 1]);
	    jobName[num - 1] = NULL;//clean name from jobName
	    jobPid[num - 1] = NULL;//clean pid from jobPid
	  }
	}
      }
      else
	printf("Usage: kill <pid> <pid> ... <pid>\n");
    }
    else if(strcmp(command,"jobs") == 0)//if command is job
    {
      for(i = 0; i < jobLimit; ++i)//print all jobs
      {
	if(jobPid[i] != NULL)
	  printf("[%d]     %s\n",i + 1,jobName[i]);
      }
    }
    else if(strcmp(command,"history") == 0)
    {
      HIST_ENTRY **list;
      int h;

      list = history_list();
      for(h = 0; h < history; ++h)
	printf("%d: %s\n",h + history_base, list[h]->line);
    }
    else if(strcmp(command,"") == 0)//if command line blank
    {
      //eats input
    }
    else
    {
      pid = fork();//create child process
	
      if(pid > 0)//if in parent
      {
	if(background == -1)//if foreground process
	{
	  waitpid(pid,&status,0);//wait for child to finish
	  
	  if(output == 0)//if output true
	    close(output);//close output
	  
	  if(input == 0)//if input true
	    close(input);//close input
	    
	  if(pipeOut == 0)//if pipeOut true
	    close(tube[1]);//close write end of pipe

	  if(pipeOut2 == 0)//if pipeOut2 true
	    close(tube2[1]);//close write end of pipe
	  
	  if(pipeIn == 0)//if pipeIn true
	    close(tube[0]);//close read end of pipe

	  if(pipeIn2 == 0)//if pipeIn2 true
	    close(tube2[0]);//close read end of pipe
        }
	else//if background process
	{
          for(i = 0; i < jobLimit; ++i)//look through jobs list for open slot
          {
            if(jobPid[i] == NULL)//if slot open, store name and pid
            {
	      jobPid[i] = pid;
	      jobName[i] = command;
	      break;
	    }
	  }
	  if(i == (jobLimit - 1))//if only one available slot, increase size
	  {
	    jobLimit += 100;
	    jobPid = (int)realloc(jobPid,jobLimit*sizeof(int));
	    jobName = (char*)realloc(jobName,jobLimit*sizeof(char*));
	    
	    ++i;
	    while(i < jobLimit)
	    {
	      jobPid[i] = NULL;
	      jobName[i] = NULL;
	      ++i;
	    }
	  }
	}
      }
      




      if(pid == 0)//if in child
      {
	if(background == -1)//if foreground process
	  signal(SIGINT,SIG_DFL);//dont ignore SIGINT

	if(output != -1)//if output specified
        {
	  if(append == -1)//if append false
	    output = open(outputFile, O_CREAT|O_WRONLY, 0644);
	  else 
	    output = open(outputFile, O_APPEND|O_WRONLY, 0644);
	  
	  if(output == -1)//if file could not be opened, error
	  {
	    perror("File could not be opened");
            exit(1);
          }
	  
          if(dup2(output,1) == -1)//if stream could not be changed, error
          {
            perror("dup2 failed");
            exit(1);
          }
	}

        if(input != -1)//if input specified
        {
          input = open(inputFile, O_RDONLY, 0644);
          
	  if(input < 0)//if file could not be opened, error
          {
            perror("File could not be opened");
            exit(1);
          }

	  if(dup2(input,0) == -1)//if stream could not be changed, error
          {
            perror("dup2 failed");
            exit(1);
          }
        }

	if(pipeOut == 0)//if pipeOut true
	{
	  close(tube[0]);//close read end of pipe
	  if(dup2(tube[1],1) == -1)//change stream, if can't, error
	  {
	    perror("Cannot write pipe");
	    exit(1);
	  }
	}

	if(pipeOut2 == 0)//if pipeOut2 true
	{
	  close(tube2[0]);//close read end of pipe
	  if(dup2(tube2[1],1) == -1)//change stream, if can't, error
	  {
	    perror("Cannot write pipe2");
	    exit(1);
	  }
	}

	if(pipeIn == 0)//if pipeIn true
	{
	  close(tube[1]);//close write end of pipe
	  if(dup2(tube[0],0) == -1)//change stream, if can't, error
	  {
	    perror("Cannot read pipe");
	    exit(1);
	  }
	}

	if(pipeIn2 == 0)//if pipeIn2 true
	{
	  close(tube2[1]);//close write end of pipe
	  if(dup2(tube2[0],0) == -1)//change stream, if can't, error
	  {
	    perror("Cannot read pipe2");
	    exit(1);
	  }
	}
	  
        if(count == 1)//if no arguments given 
        {
          execvp(command,NULL);//run command
          printf("Invalid command\n");
          exit(1);
        }
        else//if arguments given
	{
          args[count] = NULL;//set last argument to NULL
          execvp(command,args);//run command with arguments
          printf("Invalid command or arguments\n");
          exit(1);
        }
      }
    }




    //reset to false
    input = -1;
    output = -1;
    append = -1;
    background = -1;
    inputFile = NULL;
    outputFile = NULL;
    
    if(pipeIn == 0)//if pipeIn true
    {
      pipeIn = -1;//set pipeIn to false
      piping = -1;//set piping to false
    }

    if(pipeIn2 == 0)//if pipeIn2 true
    {
      pipeIn2 = -1;//set pipeIn2 to false
      piping2 = -1;//set piping2 to false
    }


    if(more == -1)//if more false
      free(line);//free input string pointer
  
    for(i = 0; i < count; ++i)//set all used indexes in args to NULL
    {
      args[i] = NULL;
    }
  }


  return 0;
}
