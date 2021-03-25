// Wesley Chong
// Chapter 8 Assignment 8.26 shell.c
// Dr David Owen
// gcc shell.c csapp.c -lpthread -o shell && ./shell
/*
 * Get csapp.h and csapp.c from http://csapp.cs.smu.edu/3e/code.html
 *
 * $ gcc shell.c csapp.c -lpthread -o shell
 */

#include "csapp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

#define MAXARGS 128

#define MAXCHARS 128

pid_t fg_pid = 0;
int next_jid = 1;

typedef struct list_t {
  pid_t pid;
  int jid;
  char *cmdline;
  struct list_t *next;
  int running; // variable to check if job is running or terminated
  int stopped; // variable to check if job is stopped
} list_t;

list_t *job_list = NULL, *e;

void add_element(list_t **list, pid_t pid, int jid, char *cmdline, int running, int stopped)
{
    list_t *e;

    if (*list == NULL)  // New empty list.
    {
        *list = (list_t *) malloc(sizeof(list_t));
        (*list)->pid = pid;
        (*list)->jid = jid;
        (*list)->cmdline = strndup(cmdline, MAXCHARS);
        (*list)->running = running; // 1 = running , 0 = terminated
        (*list)->stopped = stopped; // 1 = stopped/paused, 0 = running
        (*list)->next = NULL;
    }
    else  // List with at least one element.
    {

        for (e = *list; e->next != NULL; e = e->next){
            ;}
          //}
            e->next = (list_t *) malloc(sizeof(list_t));
            e = e->next;
            e->pid = pid;
            e->jid = jid;
            e->running = running;
            e->stopped = stopped;
            e->cmdline = strndup(cmdline, MAXCHARS);
            e->next = NULL;
          //}
        

       
    }
}


void sigint_handler(int signal) { // handles terminations

    Signal(SIGINT, SIG_DFL);

    if (fg_pid != 0){
      Kill(-fg_pid, SIGINT);
      printf("Job %d terminated by signal: SIGINT\n", fg_pid);
      Signal(SIGINT, sigint_handler);
    }else{
      raise(SIGINT);
    } 
    
}
void sigstp_handler(int signal) { // handler for stopping/pausing jobs
    
    Signal(SIGTSTP, SIG_DFL);
    // list_t *list = NULL, *e;
    // int sigStpJid = 0;
    // char *sigStpCmd = 0;

    if (fg_pid != 0){
      Kill(fg_pid, SIGTSTP);
      printf("Job %d stopped by signal: SIGTSTP\n", fg_pid);
    
    
    //   for (e = job_list; e != NULL; e = e->next){
    //     if (e->pid == fg_pid){
    //       sigStpJid = e->jid;
    //       sigStpCmd = e->cmdline;
    //     }
    //   }
    //  add_element(&job_list, fg_pid, sigStpJid, sigStpCmd);

    /* ^ add paused element to job_list because jobs started in the foreground and then stopped are not added to job_list. So this was an attempt to add a job to job_list when it was paused
    */


      Signal(SIGTSTP, sigstp_handler);
    }else{
      raise(SIGTSTP);
    }
}
// void sigcont_handler(int signal) { // handler to restart stopped/paused jobs

//     Signal(SIGCONT, SIG_DFL);
//     list_t *list = NULL, *e;

//     if (fg_pid != 0){
//       Kill(-fg_pid, SIGCONT);
//       printf("Stopped job %d resumed by signal: SIGCONT\n", fg_pid);
//       Signal(SIGCONT, sigcont_handler);
//     }else{
//       raise(SIGCONT);
//     } 
// }
// ^ realised I did not need this 

int parseline(char *line, char **argv) {
    char *cp;
    int in_arg = FALSE;
    int argc = 0;
    int bg = FALSE;

    // Go through line, one character at a time, until reaching the
    // newline character at the end.
    for (cp = line; *cp != '\n'; cp++) {
    
        if (in_arg) {
        
            // If at the end of an argument...
            if (*cp == ' ') {
                *cp = '\0'; // Mark end of argument.
                in_arg = FALSE;
            }
        } else if (*cp != ' ') { // If at beginning of new argument...
            argv[argc++] = cp;   // Set argv array element to point to
                                 // new argument.
            in_arg = TRUE;
        }
    }

    *cp = '\0';  // Mark end of last argument (which was probably
                 // followed by \n, not space).

    // If at least one argument, and last argument is &, process is
    // to be run in background.
    if (argc > 0 && *argv[argc - 1] == '&') {
        bg = TRUE;
        argv[argc - 1] = NULL; // Overwrite address of "&" to mark end of argv.
    
    } else {                   // (Process should run in foreground.)
        argv[argc] = NULL;     // Mark end of argv.
    }

    return bg;
}

int builtin_command(char **argv) {
    list_t *list = NULL, *e;
    int jid;
    if (strcmp(argv[0], "quit") == 0) {
        exit(0);
    
    } else if (strcmp(argv[0], "&") == 0) {
        return TRUE;
    }
    else if (strcmp(argv[0], "jobs") == 0) {
      // print list of running background jobs
      for (e = job_list; e != NULL; e = e->next){
        if (e->running == 1){ // print if job is currently running
          printf("[%d] %d %s %s\n", e->jid, e->pid, "Running", e->cmdline);
        }
        else if (e->stopped == 1){ // print if job is paused
          printf("[%d] %d %s %s\n", e->jid, e->pid, "Paused", e->cmdline);
        }
        else if (e->running == 0){ // do not print job if it is terminated
          ;
        }
        
        else {
          ;
        }
        // ^ Attempt to print distinctions between running and paused jobs
        // running and paused jobs print correctly
        // background jobs started as foreground jobs do not print if ctrlz-ed
        // background jobs started as background jobs print, but are always listed as "running"

      }

      return TRUE;
    }
    else if (!strcmp(argv[0], "fg")) { /* fg command */
      pid_t pid;
      pid = atoi(argv[1]);
      fg_pid = pid;
      Kill(pid, SIGCONT);
      Waitpid(fg_pid, NULL, WUNTRACED); 
      fg_pid = 0; // restart
      // e->running=1; < set running to 1 for when printing jobs
      return TRUE;
    }
    else if (!strcmp(argv[0], "bg")) { /* bg command */
      // pid_t pid;
      // pid = atoi(argv[1]);
      // pid = Fork();
      // if(pid == 0){
      //     Setpgid(pid, pid);
      //     Kill(pid, SIGCONT);
      //   }
      //   return TRUE;
      // ^ bg command does not work. but here is an attempt. 
      
      // pid_t pid;
      // int check = 0;
      // for (e = job_list; e != NULL; e = e->next){
      //   if((((atoi(argv[1]) == e->jid) && (strcmp(argv[2], "%") == 0)) || atoi(argv[1]) == e->pid) ){
      //     check = 1;
      //     pid = Fork();
      //     e->running = 1;
      //     if(pid == 0){
      //         Setpgid(pid, pid);
      //         Kill(e->pid, SIGCONT);
      //       }
      //       e->stopped = 0;
      //   }
      // }
      // if(check == 0){
      //     printf("The JID or PID you specified does not exist\n");
      //   }
      // ^ second attempt at bg
      return TRUE;
    }

    return FALSE;
}

void eval(char *cmdline, char **envp) {
    char *argv[MAXARGS];
    char buf[MAXLINE];
    int bg;
    pid_t pid;
    int jid;
    int x;

    strcpy(buf, cmdline);
    bg = parseline(buf, argv);


    if (argv[0] != NULL && !builtin_command(argv)) {
        pid = Fork();

        if (pid == 0) {   // In child.
            pid = getpid();
            Setpgid(pid, pid);
        
            if (execve(argv[0], argv, envp) < 0) {
              printf("%s: Command not found.\n", argv[0]);
              exit(0);
            }

        } else if (!bg) { // In parent, child running in foreground.
            fg_pid = pid;
            Waitpid(fg_pid, &x, WUNTRACED);
            if(WIFSTOPPED(x)){
              add_element(&job_list, pid, next_jid, cmdline, 0, 1); // job stopped and not running
            }else{
              add_element(&job_list, pid, next_jid,cmdline, 0, 0); // job terminated
            }
            next_jid += 1;
            fg_pid = 0;
            
        
        } else {          // In parent, child running in background.
            printf("[%d] %d %s", jid, pid, cmdline);
            add_element(&job_list, pid, next_jid, cmdline, 1, 0); // job running
            next_jid++;

        }
    }
}

int main(int argc, char **argv, char **envp) {
    char cmdline[MAXLINE];

    Signal(SIGINT, sigint_handler);
    Signal(SIGTSTP, sigstp_handler);
    //Signal(SIGCONT, sigcont_handler);

    while (TRUE) {      
        printf("> ");
        Fgets(cmdline, MAXLINE, stdin);
        eval(cmdline, envp);
    }
}
