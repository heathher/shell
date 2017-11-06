#include "shell.h"

extern struct job curr_job;
extern int condition;
int fd[2];
pid_t pid, pgid;
int status;
int left_pipe[2];
int right_pipe[2];
int i;
pid_t Wait;
int proc_num = 0;
struct proc *proc_list = NULL;
extern int last_condition;

char *jobstr()
{
    char *str = NULL;
    int i, j;
    str = (char *) malloc(sizeof(char));
    str[0] = '\0';
    for(i = 0; i < curr_job.number_of_programs; i++)
    {
        for(j = 0; j < curr_job.programs[i].number_of_arguments - 1; j++)
        {
            str = (char *) realloc(str, (strlen(str) + strlen(curr_job.programs[i].arguments[j]) + 1 + 1) * sizeof(char));
            strcat(str, curr_job.programs[i].arguments[j]);
            strcat(str, " ");
        }
        
        if(curr_job.programs[i].input_file != NULL)
        {
            str = (char *) realloc(str, (strlen(str) + strlen(curr_job.programs[i].input_file) + 2 + 1) * sizeof(char));
            strcat(str, "<");
            strcat(str, curr_job.programs[i].input_file);
            strcat(str, " ");
        }
        
        if(curr_job.programs[i].output_file != NULL)
        {
            if(curr_job.programs[i].output_type == 1)
            {
                str = (char *) realloc(str, (strlen(str) + strlen(curr_job.programs[i].output_file) + 2 + 1) * sizeof(char));
                strcat(str, ">");
            }
            else
            {
                str = (char *) realloc(str, (strlen(str) + strlen(curr_job.programs[i].output_file) + 3 + 1) * sizeof(char));
                strcat(str, ">>");
            }
            
            strcat(str, curr_job.programs[i].output_file);
            strcat(str, " ");
        }
        
        if(i != (curr_job.number_of_programs - 1))
        {
            str = (char *) realloc(str, (strlen(str) + 2 + 1) * sizeof(char));
            strcat(str, "| ");
        }
    }
    if(curr_job.background == 0)
    {
        str[strlen(str) - 1] = '\0';
        str = (char *) realloc(str, (strlen(str) + 1) * sizeof(char));
    }
    else
    {
        str = (char *) realloc(str, (strlen(str) + 1 + 1) * sizeof(char));
        strcat(str, "&");
    }
    return str;
}

int running()
{
    if(curr_job.number_of_programs == 0)
        return 0;
    
    for(i = 0; i < curr_job.number_of_programs; i++)
    {
        if(curr_job.number_of_programs != 1)
            pipe(right_pipe);
        
        if(curr_job.programs[i].input_file != NULL)
        {
            fd[0] = open(curr_job.programs[i].input_file,O_RDONLY, 0666);
            if(fd[0] < 0)
            {
                perror(curr_job.programs[i].input_file);
                return -2;
            }
        }
        
        if(curr_job.programs[i].output_file != NULL)
        {
            if(curr_job.programs[i].output_type == 1)
                fd[1] = open(curr_job.programs[i].output_file, O_CREAT | O_WRONLY | O_TRUNC, 0666);
            else
                fd[1] = open(curr_job.programs[i].output_file, O_CREAT | O_WRONLY | O_APPEND, 0666);
            
            if(fd[1] < 0)
            {
                perror(curr_job.programs[i].output_file);
                return -2;
            }
        }
        
        pid = fork();
        
        if(pid == -1)
        {
            while(wait(&status) != -1);
            perror("shell");
            return -3;
        }
        
        if(i == 0)
            pgid = pid;
        
        setpgid(pid, pgid);
        
        if(i == 0 && curr_job.background == 0)
            tcsetpgrp(0, pgid);
        
        if(pid == 0)
        {
            signal(SIGTTOU, SIG_DFL);
            signal(SIGTTIN, SIG_DFL);
            if(i > 0)
            {
                dup2(left_pipe[0], 0);
                close(left_pipe[0]);
                close(left_pipe[1]);
            }
            
            if(curr_job.programs[i].input_file != NULL)
            {
                dup2(fd[0], 0);
                close(fd[0]);
            }
            
            if(i < (curr_job.number_of_programs - 1))
            {
                dup2(right_pipe[1], 1);
                close(right_pipe[0]);
                close(right_pipe[1]);
            }
            
            if(curr_job.programs[i].output_file != NULL)
            {
                dup2(fd[1],1);
                close(fd[1]);
            }
            
            if (strcmp(curr_job.programs[i].name, "mcat") == 0)
            {
                mcat();
                condition = END;
                return 0;
            }
            
            if (strcmp(curr_job.programs[i].name, "mgrep") == 0)
            {
                mgrep();
                condition = END;
                return 0;
            }
            
            if  (strcmp(curr_job.programs[i].name, "msed") == 0)
            {
                msed();
                condition = END;
                return 0;
            }
            
            execvp(curr_job.programs[i].name, curr_job.programs[i].arguments);
            
            perror(curr_job.programs[i].name);
            condition = END;
            return -2;
        }
        
        if(i > 0)
        {
            close(left_pipe[0]);
            close(left_pipe[1]);
        }
        
        if(curr_job.number_of_programs > 1)
        {
            left_pipe[0] = right_pipe[0];
            left_pipe[1] = right_pipe[1];
        }
        
        if(curr_job.programs[i].input_file != NULL)
            close(fd[0]);
        if(curr_job.programs[i].output_file != NULL)
            close(fd[1]);
    }
    add_proc(pgid, jobstr());
    wait_in_bg();
    
    return 0;
}

void mfg()
{
    int number;
    char* strptr;
    if(curr_job.programs[0].number_of_arguments != 3)
    {
        printf("fg: invalid input\n");
        return;
    }
    
    number = (int) strtol(curr_job.programs[0].arguments[1], &strptr, 10);
    if((strptr[0] != '\0') || (number <= 0))
    {
        printf("fg: invalid input\n");
        return;
    }
    
    if(number > proc_num)
    {
        printf("fg: no such job\n");
        return;
    }
    proc_stat(number, 0);
    print_proc(number);
    tcsetpgrp(0, proc_pid(number));
    killpg(proc_pid(number), SIGCONT);
    wait_in_bg();
}

void mbg()
{
    int number;
    char* strptr;
    if(curr_job.programs[0].number_of_arguments != 3)
    {
        printf("bg: invalid input\n");
        return;
    }
    
    number = (int) strtol(curr_job.programs[0].arguments[1], &strptr, 10);
    if((strptr[0] != '\0') || (number <= 0))
    {
        printf("bg: invalid input\n");
        return;
    }
    
    if(number > proc_num)
    {
        printf("bg: no such job\n");
        return;
    }
    proc_stat(number, 0);
    print_proc(number);
    killpg(proc_pid(number), SIGCONT);
}



int add_proc(pid_t pid, char *name)
{
    struct proc *curr_proc = NULL;
    if(proc_list == NULL)
    {
        proc_list = (struct proc*) malloc(sizeof(struct proc));
        if(proc_list == NULL)
        {
            printf("shell: failed to allocate memory\n");
            condition = END;
            return -2;
        }
        
        proc_list->name = name;
        proc_list->pid = pid;
        strcpy(proc_list->condition, "Running");
        proc_list->next = NULL;
        proc_num++;
        return 0;
    }
    curr_proc = proc_list;
    while(curr_proc->next != NULL)
        curr_proc = curr_proc->next;
    
    curr_proc->next = (struct proc*) malloc(sizeof(struct proc));
    if (curr_proc->next == NULL)
    {
        printf("shell: failed to allocate memory\n");
        condition = END;
        return -2;
    }
    
    curr_proc->next->name = name;
    (curr_proc->next)->next = NULL;
    strcpy(curr_proc->next->condition, "Running");
    (curr_proc->next)->pid = pid;
    proc_num++;
    return 0;
}

int del_proc(int number)
{
    struct proc *curr_proc = proc_list;
    struct proc *temp_proc = NULL;
    int i;
    
    for(i = 1; i < (number - 1) && curr_proc != NULL; i++)
        curr_proc = curr_proc->next;
    
    if(curr_proc == NULL)
    {
        printf("no job\n");
        return -2;
    }
    
    if(number == 1)
    {
        temp_proc = proc_list->next;
        free(proc_list->name);
        free(proc_list);
        proc_list = temp_proc;
        proc_num--;
        return 0;
    }
    
    if(curr_proc->next == NULL)
    {
        printf("no job\n");
        return -2;
    }
    
    temp_proc = curr_proc->next->next;
    free(curr_proc->next->name);
    free(curr_proc->next);
    curr_proc->next = temp_proc;
    proc_num--;
    return 0;
}


void deleteprocs()
{
    struct proc *temp_proc;
    
    while(proc_list != NULL)
    {
        temp_proc = proc_list;
        proc_list = proc_list->next;
        free(temp_proc->name);
        free(temp_proc);
    }
    proc_num = 0;
}

void print_proc(int number)
{
    struct proc *curr_proc = proc_list;
    int i = 1;
    for(i = 1; i < number && curr_proc != NULL; i++)
        curr_proc = curr_proc->next;
    
    if(curr_proc == NULL)
    {
        printf("no job\n");
        return;
    }
    
    printf("[%d]\t%s\t%s\n", number, curr_proc->condition, curr_proc->name);
}

void print_procs()
{
    struct proc *curr_proc = proc_list;
    int i = 1;
    while(curr_proc != NULL)
    {
        printf("[%d]\t%s\t%s\n", i++, curr_proc->condition, curr_proc->name);
        curr_proc = curr_proc->next;
    }
}

pid_t proc_pid(int number)
{
    struct proc *curr_proc = proc_list;
    int i = 1;
    for(i = 1; i < number && curr_proc != NULL; i++)
        curr_proc = curr_proc->next;
    
    if(curr_proc == NULL)
    {
        printf("no job\n");
        return -1;
    }
    
    return curr_proc->pid;
}

char* proc_name(int number)
{
    struct proc *curr_proc = proc_list;
    int i = 1;
    for(i = 1; i < number && curr_proc != NULL; i++)
        curr_proc = curr_proc->next;
    
    if(curr_proc == NULL)
    {
        printf("no job\n");
        return NULL;
    }
    
    return curr_proc->name;
}

int find_proc(pid_t tpid)
{
    struct proc *curr_proc = proc_list;
    int i;
    for(i = 1; i < proc_num && curr_proc->pid != tpid; i++)
        curr_proc = curr_proc->next;
    
    if(curr_proc->pid != tpid)
    {
        printf("no job\n");
        return 0;
    }
    
    return i;
}

void proc_stat(int number, int arg)
{	
    struct proc *curr_proc = proc_list;
    int i = 1;
    for(i = 1; i < number && curr_proc != NULL; i++)
        curr_proc = curr_proc->next;
    
    if(curr_proc == NULL)
    {
        printf("no job\n");
        return;
    }
    
    if(arg > 0)
        strcpy(curr_proc->condition, "Stopped");
    else
        strcpy(curr_proc->condition, "Running");
}	

void wait_in_bg()
{
    int tcjob;
    if(tcgetpgrp(0) != getpgrp())
    {
        tcjob = find_proc(tcgetpgrp(0));
        while(waitpid(-proc_pid(tcjob), &status, WUNTRACED) != -1)
        {
            if(WIFEXITED(status))
                last_condition = WEXITSTATUS(status);
            if(WIFSTOPPED(status))
            {
                proc_stat(tcjob, 1);
                tcsetpgrp(0, getpgrp());
                return;
            }
            
        }
        del_proc(tcjob);
        tcsetpgrp(0, getpgrp());
    }	
}

void background()
{
    pid_t result;
    
    for(i = 1; i <= proc_num; i++)
    {	
        while((result = waitpid(-proc_pid(i), &status, WNOHANG | WUNTRACED | WCONTINUED)) != 0)
        {
            if(result == -1)
            {
                if(tcgetpgrp(0) == proc_pid(i))
                    tcsetpgrp(0, getpgrp());
                del_proc(i--);
                break;
            }
            if(WIFSTOPPED(status))
            {
                if(tcgetpgrp(0) == getpgid(result))
                    tcsetpgrp(0, getpgrp());
                proc_stat(i, 1);
            }
            if(WIFCONTINUED(status))
                proc_stat(i, 0);
        }
    }
}


