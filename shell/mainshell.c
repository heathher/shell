#include "shell.h"

int margc;
char **margv;
extern int condition;
extern struct job curr_job;

int main(int argc, char **argv)
{
    margc = argc;
    margv = argv;
    init();
    while(condition != END)
    {
        background();
        getjob();
        background();
        runintegrateprograms();
        running();
    }
    deletehistory();
    deleteprocs();
    freejob();
    return 0;
}
