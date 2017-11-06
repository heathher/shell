#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

#define IN_PROG 0
#define IN_ARGS 1
#define IN_OUT 2
#define READ 3
#define WRITE 4
#define APPEND 5
#define END_JOB 6
#define END 7
#define DEF 0
#define SQ 1
#define DQ 2
#define SIZEOFPID 16
#define SIZEOFUID 16
#define CURR_PROG (curr_job.programs[curr_job.number_of_programs - 1])
#define NO_MEM "error: failed to allocate memory\n"

struct program
{
    char *name;
    int number_of_arguments;
    char **arguments;
    char *input_file, *output_file;
    int output_type; /* 1 - rewrite, 2 - append */
};

struct job
{
    int background;
    struct program *programs;
    int number_of_programs;
};

struct proc
{
    pid_t pid;
    char* name;
    char condition[8];
    struct proc *next;
};

int isspecsymbol();
void getjob();
void getsymbol();
void freejob();
void printjob();
void mem_error();
void skipjob();
char *mgetenv();
void init();
int running();
int add_proc();
int del_proc();
void deleteprocs();
void print_procs();
void print_proc();
pid_t proc_pid();
char *proc_name();
void mfg();
void mbg();
void mcd();
void wait_in_bg();
int find_proc();
void background();
void proc_stat();
void mcat();
void mgrep();
void msed();
void historyprint();
void deletehistory();
int mgetchar();
void mungetchar();
void putinhist();
char *jobstr();
void runintegrateprograms();