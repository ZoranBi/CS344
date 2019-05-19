/********************************************************
Name: biz.buildrooms.c
Author: Zongzhe Bi
Date: 10/18/2018
********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define MAX_ARG 512
#define MAX_LEN 2048

char *buffer;
char *command_line[MAX_ARG];
size_t bufsize = MAX_LEN;
size_t characters;
int status = -5;
int input_loc;
int output_loc;
int last_loc;
int flag_in_direc = 0;
int flag_out_direc = 0;
int flag_bg = 0;
int flag_pid = 0;
int in_directory;
int out_directory;
int i = 0;
int num_process = 0;
pid_t pid[10];
int backStatus = 0;
int fg_mode;

/*******************************************************************
                           Functions
*******************************************************************/
void background_handle(){
   for(i = 0; i < num_process; i ++){
      if(pid[i] == 0 || pid[i] == -1) continue;
      int temp = pid[i];
      pid[i] = waitpid(pid[i], &status, WNOHANG);
      if(pid[i] == 0){
         if(WIFEXITED(status))
            printf("back ground pid %d is done, exit value %d\n",temp,WEXITSTATUS(status));
         else if (WIFSIGNALED(status))
            printf("background pid %d is done: terminated by signal %d\n", temp, WTERMSIG(status));
         else
            printf("back ground pid %d is done, terminate by signal %d\n",temp,WTERMSIG(status));
      }
   }
}


/*
NAME: func_call
DESCRIBES: use fork to create child process and handle non-buildin funcions
*/
void func_call(struct sigaction SIGINT_action, struct sigaction SIGINT_parent_action){
   pid[num_process] = fork();
   //check error
   if(pid[num_process] < 0) {
      printf("ERROR: FAIL TO CREATE CHILD PROCESS");
      exit(1);
   }
   else if(pid[num_process] == 0){
      //handle background
      if(flag_bg && backStatus == 0) {
         int in, out;
         in = open("/dev/null", O_RDONLY);
         out = open("/dev/null", O_WRONLY);
         dup2(in, 0);
         dup2(out, 1);
      }
      else {
			SIGINT_action.sa_handler = SIG_DFL;
			sigaction(SIGINT,&SIGINT_action,NULL);
		}
      if(flag_in_direc && !flag_out_direc) {
         int in;
         in = open(command_line[input_loc], O_RDONLY|O_CREAT);

         if (in_directory == -1){
            perror("ERROR: CANNOT OPEN IN DIRECTORY\n");
            exit(1);
         }
         if (dup2(in, 0) == -1){
            perror("ERROR: dup2()");
            exit(2);
         }
         execvp(command_line[0], command_line);
         exit(0);
      }
      else if(!flag_in_direc && flag_out_direc) {
         int out;
         out = open(command_line[output_loc], O_WRONLY | O_CREAT | O_TRUNC, 0644);

         if (out_directory < 0){
            perror("ERROR: CANNOT OPEN OUT DIRECTORY\n");
            exit(1);
         }
         if (dup2(out, 1) == -1){
            perror("ERROR: dup2()");
            exit(2);
         }
         execvp(command_line[0], command_line);
         exit(0);
      }
      else if(flag_in_direc && flag_out_direc) {
         int in, out;
         in = open(command_line[input_loc], O_RDONLY);
         if(in < 0){
           printf("ERROR: CANNOT OPEN INPUT FILE\n");
           exit(1);
         }
         dup2(in, 0);
         out = open(command_line[output_loc], O_WRONLY | O_CREAT | O_TRUNC, 0660);
         dup2(out, 1);
         execvp(command_line[0], command_line);
         exit(0);
      }
      else{
         execvp(command_line[0], command_line);
         exit(0);
      }
   }
   else{
      if(flag_bg != 1 || backStatus == 1){
			sigaction(SIGINT,&SIGINT_parent_action,NULL);
		}
      fflush(stdout);
      if(flag_bg == 1 && backStatus == 0){
        printf("background pid is %d\n", pid[num_process]);
      }
      else
      {
			waitpid(pid[num_process], &status, 0);	//wait fo process
		}
      num_process++;
   }
}

/*
NAME: buildin_handle
DESCRIBES: buildin function handle. cd, status, exit
*/
void buildin_handle(struct sigaction SIGINT_action, struct sigaction SIGINT_parent_action){
   if (buffer[0] != '#' && strcmp(buffer, "")){
      // handle 'exit' command...
      if(!strcmp(command_line[0], "exit"))
      {
         int i=0;
         for(i = 0; i < num_process; i ++){
     			if(pid[i] == 0 || pid[i] == -1)
     				continue;
           kill(pid[i], SIGTERM);
         }
         exit(0);
      }
      // handle 'cd' command...
      else if(!strcmp(command_line[0], "cd"))
      {
         if(command_line[1] == NULL) chdir(getenv("HOME"));
         // change directory to wherever saved in command_line[1]
         else chdir(command_line[1]);
      }
      // handle 'status' command...
      else if(!strcmp(command_line[0], "status"))
      {
         // check if the child process terminated normally with exit or _exit
         if (WIFEXITED(status))
         {
         // get the low-order 8 bits of the exit status value from the child process.
            printf("exit value %d\n", WEXITSTATUS(status));
         }
         // check if the child process terminated because it received a signal that was not handled
         else if (WIFSIGNALED(status))
         {
         // get the signal number of the signal that terminated the child process
            printf("terminated by signal %d\n", WTERMSIG(status));
         }
      }
      else{
         func_call(SIGINT_action, SIGINT_parent_action);
      }
   }
}


/*
NAME: user_input
DESCRIBES: read user input and store command in a one array
*/
void user_input(){
   printf(": ");
   fflush(stdout);
   characters = getline(&buffer, &bufsize, stdin);
   if (characters > 0) buffer[characters-1] = '\0';

   i = 1;
   flag_in_direc = 0;
   flag_out_direc = 0;
   flag_bg = 0;

   command_line[0] = strtok(buffer, " ");
   while (command_line[i-1] != NULL){
      command_line[i] = strtok(NULL, " ");
      if(command_line[i] != NULL){
         if (!strcmp(command_line[i], "<")){
            memset(command_line[i], '\0', 1);
            input_loc = i;
            i --;
            flag_in_direc = 1;
         }
         if(!strcmp(command_line[i], ">")){
            memset(command_line[i], '\0', 1);
            output_loc = i;
            i --;
            flag_out_direc = 1;
         }
      }
      i++;
   }
   last_loc = i-1;
   if(!strcmp(command_line[last_loc-1], "&")){
      flag_bg = !flag_bg;
      memset(command_line[last_loc-1], '\0', 1);
      command_line[last_loc-1] = NULL;
      last_loc --;
   }
   for (i = 0; i < last_loc; i++){
      char *temp;
      char str_pid[10];
      memset(str_pid, '\0', 10);
      if((temp = strstr(command_line[i], "$$"))){
         sprintf(str_pid, "%d", getpid());
         strncpy (temp, str_pid ,strlen(str_pid));
      }
   }
}

/*
NAME: catchSIGINT
DESCRIBES: signcatch function to process the signal
*/
void catchSIGSTP(int signo){
   write(STDOUT_FILENO, "here is catchSIGSTP\n", 32);
   if (fg_mode == 0){
      const char message[] = "Entering foreground-only mode (& is now ignored)\n";
      write(STDOUT_FILENO, message, strlen(message));
      fg_mode = 1;
   }
   else{
      const char message[] = "Exiting foreground-only mode\n";
      write(STDOUT_FILENO, message, strlen(message));
      fflush(stdout);
      fg_mode = 0;
   }
}


void catchSIGINT(int signo){
   write(STDOUT_FILENO, "here is catchSIGINT\n", 32);
   wait(&status);
   const char message[] = "\n: ";
   write(STDOUT_FILENO, message, strlen(message));
   fflush(stdout);
}

/*******************************************************************
                           Main Functions
*******************************************************************/
int main(int argc, char const *argv[]) {
   //signal handle part
   struct sigaction SIGSTP_action = {0};
   SIGSTP_action.sa_handler = catchSIGSTP;
   sigfillset(&SIGSTP_action.sa_mask);
   SIGSTP_action.sa_flags = 0;

   struct sigaction SIGINT_action = {0};
   SIGINT_action.sa_handler = catchSIGINT;
   sigfillset(&SIGINT_action.sa_mask);
   SIGINT_action.sa_flags = 0;

   do {
      SIGSTP_action.sa_handler = SIG_IGN;
      SIGINT_action.sa_handler = catchSIGINT;
		sigaction(SIGINT,&SIGINT_action,NULL);

      background_handle();
      user_input();
      buildin_handle(SIGINT_action, SIGINT_action);
   } while(1);

   return 0;
}
