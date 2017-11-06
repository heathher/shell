#include "shell.h"

int c = '\n';
int temp_c;
char *token = NULL;
char *some;
char *some_some;
int condition = IN_PROG;
int conveyor = 0;
int error = 0;
int last_condition = 0;
struct job curr_job;
struct program *temp_programs;
char **temp_arguments;
char *temp_str;
char **history = NULL;
int hist = 0;
int thist = 0;
int hist_size = 0;
int hist_curr = 0;
int hist_rec = 0;
int temp = 0;
extern int margc;
extern char **margv;

void getsymbol()
{
    token = NULL;
    while((c == ' ') || (c == '\t'))
        c = mgetchar();
    
    /* ! */
    if(c == '!')
    {
        thist = 0;
        temp = 1;
        c = mgetchar();
        
        if(!isspecsymbol(c))
        {
            temp++;
            while(c >= '0' && c <= '9')
            {
                thist *= 10;
                thist += c - '0';
                temp++;
                c = mgetchar();
            }
            if(!isspecsymbol(c))
            {
                skipjob(DEF);
                fprintf(stderr, "error: event not found\n");
                return;
            }
        }
        if(thist >= hist_size || thist == 0)
        {
            mungetchar(temp);
            putinhist(c);
            skipjob(DEF);
            fprintf(stderr, "error: event not found\n");
            return;
        }
        temp_c = c;
        mungetchar(temp);
        hist = thist;
        hist_rec = 0;
        c = mgetchar();
        return;
    }
    
    /* # */
    if(c == '#')
    {
        while ((c != '\n') && (c != EOF))
        {
            mungetchar(1);
            c = mgetchar();
        }
        return;
    }
    
    /* < */
    if(c == '<')
    {
        if(condition == IN_ARGS || condition == IN_OUT)
        {
            condition = READ;
            c = mgetchar();
            return;
        }
        else
        {
            skipjob(DEF);
            fprintf(stderr, "error: unexpected '<'\n");
            return;
        }
        
    }
    
    /* > */
    if(c == '>')
    {
        if(condition == IN_ARGS || condition == IN_OUT)
        {
            c = mgetchar();
            if(c == '>')
            {
                c = mgetchar();
                condition = APPEND;
            }
            else
                condition = WRITE;
            return;
        }
        else
        {
            skipjob(DEF);
            fprintf(stderr, "error: unexpected '>'\n");
            return;
        }
    }
    
    /* & */
    if(c == '&')
    {
        if(condition == IN_ARGS || condition == IN_OUT)
        {
            c = mgetchar();
            condition = END_JOB;
            curr_job.background = 1;
            return;
        }
        else
        {
            skipjob(DEF);
            fprintf(stderr, "error: unexpected '&'\n");
            return;
        }
    }
    
    /* | */
    if(c == '|')
    {
        if(condition != IN_PROG && condition != READ && condition != WRITE && condition != APPEND && condition != END_JOB)
        {
            c = mgetchar();
            conveyor = 1;
            condition = IN_PROG;
            return;
        }
        else
        {
            skipjob(DEF);
            if(condition == IN_PROG)
                fprintf(stderr,"error: unexpected '|'\n");
            if(condition == END_JOB)
                fprintf(stderr,"error: unexpected '|'\n");
            else
                fprintf(stderr,"error: file name expected\n");
            return;
        }
        
    }
    
    /* ; */
    if(c == ';')
    {
        if(condition != IN_PROG && condition != READ && condition != WRITE && condition != APPEND)
        {
            c = mgetchar();
            if(c == '\n')
                mungetchar(1);
            conveyor = 0;
            condition = IN_PROG;
            return;
        }
        else
        {
            if(condition == IN_PROG)
                fprintf(stderr, "error: empty job (unexpected ';')\n");
            else
                fprintf(stderr, "error: file name expected\n");
            c = mgetchar();
            if(c == '\n')
                mungetchar(1);
            error = 1;
            condition = IN_PROG;
            return;
        }
    }
    
    /* \n */
    if(c == '\n')
    {
        
        mungetchar(1);
        
        if(condition != READ && condition != WRITE && condition != APPEND)
        {
            condition = IN_PROG;
            conveyor = 0;
            return;
        }
        else
        {
            fprintf(stderr, "error: file name expected\n");
            error = 1;
            condition = IN_PROG;
            return;
        }
        
    }
    
    /* EOF */
    if (c == EOF)
    {
        if(condition != READ && condition != WRITE && condition != APPEND)
        {
            condition = END;
            return;
        }
        else
        {
            fprintf(stderr, "error: file name expected\n");
            error = 1;
            condition = END;
            return;
        }
        
    }
    token = (char*) malloc(sizeof(char));
    if(token == NULL)
    {
        mem_error();
        return;
    }
    token[0] = '\0';
    
    while(!isspecsymbol(c))
    {
        if(c == '\'')
        {
            c = mgetchar();
            some = (char*) malloc(sizeof(char));
            if(some == NULL)
            {
                mem_error();
                return;
            }
            some[0] = '\0';
            
            while(c != '\'' && c != EOF)
            {
                if(c == '\\')
                {
                    c = mgetchar();
                    if(c == EOF)
                    {
                        free(token);
                        free(some);
                        fprintf(stderr, "error: unexpected EOF in quoting\n");
                        error = 1;
                        condition = END;
                        return;
                    }
                    if(c == '\n')
                    {
                        c = mgetchar();
                        continue;
                    }
                }
                temp_str = (char*) realloc(some, (strlen(some) + 2) * sizeof(char));
                if(temp_str == NULL)
                {
                    free(some);
                    mem_error();
                    return;
                }
                
                some = temp_str;
                some[strlen(some) + 1] = '\0';
                some[strlen(some)] = c;
                c = mgetchar();
            }
            
            if(c == EOF)
            {
                free(token);
                free(some);
                fprintf(stderr, "error: unexpected EOF in quoting\n");
                error = 1;
                condition = END;
                return;
            }
            
            c = mgetchar();
        }
        else if(c == '\"')
        {
            c = mgetchar();
            some = (char*) malloc(sizeof(char));
            if(some == NULL)
            {
                mem_error();
                return;
            }
            some[0] = '\0';
            
            while(c != '\"' && c != EOF)
            {
                if(c == '$')
                {
                    some_some = mgetenv(DQ);
                    if(some_some == NULL)
                    {
                        free(some);
                        return;
                    }
                    
                }
                else
                {
                    if(c == '\\')
                    {
                        c = mgetchar();
                        if(c == EOF)
                        {
                            free(token);
                            free(some);
                            fprintf(stderr, "\nerror: unexpected EOF in quoting\n");
                            error = 1;
                            condition = END;
                            return;
                        }
                        if(c == '\n')
                        {
                            c = mgetchar();
                            continue;
                        }
                    }
                    temp_str = (char*) malloc(2 * sizeof(char));
                    if(temp_str == NULL)
                    {
                        free(some);
                        mem_error();
                        return;
                    }
                    some_some = temp_str;
                    some_some[0] = c;
                    some_some[1] = '\0';
                    c = mgetchar();
                }
                
                temp_str = (char*) realloc(some, (strlen(some) + strlen(some_some) + 1) * sizeof(char));
                if(temp_str == NULL)
                {
                    free(some);
                    free(some_some);
                    mem_error();
                    return;
                }
                some = temp_str;
                strcat(some, some_some);
                free(some_some);
            }
            
            if(c == EOF)
            {
                free(token);
                free(some);
                fprintf(stderr, "\nerror: unexpected EOF in quoting\n");
                error = 1;
                condition = END;
                return;
            }
            
            c = mgetchar();
        }
        else
        {
            if(c == '$')
            {
                some = mgetenv(DEF);
                if(some == NULL)
                    return;
            }
            else
            {
                if(c == '\\')
                {
                    c = mgetchar();
                    if(c == EOF)
                    {
                        fprintf(stderr, "\nerror: you can't escape EOF\n");
                        error = 1;
                        free(token);
                        token = NULL;
                        condition = END;
                        return;
                    }
                    if(c == '\n')
                    {
                        c = mgetchar();
                        continue;
                    }
                    
                }
                some = (char*) malloc(2 * sizeof(char));
                if(some == NULL)
                {
                    mem_error();
                    return;
                }
                some[0] = c;
                some[1] = '\0';
                c = mgetchar();
            }
        }
        temp_str = (char*) realloc(token, (strlen(token) + strlen(some) + 1) * sizeof(char));
        if(temp_str == NULL)
        {
            free(some);
            mem_error();
            return;
        }
        token = temp_str;
        strcat(token, some);
        free(some);
    }
    if(condition == IN_PROG)
    {
        temp_programs = (struct program*) realloc(curr_job.programs, (curr_job.number_of_programs + 1) * sizeof(struct program));
        if(temp_programs == NULL)
        {
            mem_error();
            return;
        }
        curr_job.number_of_programs++;
        curr_job.programs = temp_programs;
        
        
        temp_arguments = (char**) malloc(2 * sizeof(char*));
        if(temp_arguments == NULL)
        {
            mem_error();
            return;
        }
        
        CURR_PROG.arguments = temp_arguments;
        CURR_PROG.arguments[0] = token;
        CURR_PROG.arguments[1] = NULL;
        CURR_PROG.name = token;
        token = NULL;
        CURR_PROG.number_of_arguments = 2;
        CURR_PROG.input_file = NULL;
        CURR_PROG.output_file = NULL;
        CURR_PROG.output_type = 0;
        
        condition = IN_ARGS;
        
        return;
    }

    if(condition == IN_ARGS)
    {
        temp_arguments = (char**) realloc(CURR_PROG.arguments, (CURR_PROG.number_of_arguments + 1) * sizeof(char*));
        if(temp_arguments == NULL)
        {
            mem_error();
            return;
        }
        CURR_PROG.number_of_arguments++;
        CURR_PROG.arguments = temp_arguments;
        (CURR_PROG.arguments)[CURR_PROG.number_of_arguments - 2] = token;
        (CURR_PROG.arguments)[CURR_PROG.number_of_arguments - 1] = NULL;
        token = NULL;
        return;
    }
    if(condition == IN_OUT)
    {
        free(token);
        token = NULL;
        skipjob(DEF);
        fprintf(stderr, "error: unexpected arguement after input/output redirection\n");
        return;
    }
    if(condition == READ)
    {
        free(CURR_PROG.input_file);
        CURR_PROG.input_file = token;
        token = NULL;
        condition = IN_OUT;
        return;
    }
    if(condition == WRITE)
    {
        free(CURR_PROG.output_file);
        CURR_PROG.output_file = token;
        token = NULL;
        CURR_PROG.output_type = 1;
        condition = IN_OUT;
        return;
    }
    if(condition == APPEND)
    {
        free(CURR_PROG.output_file);
        CURR_PROG.output_file = token;
        token = NULL;
        CURR_PROG.output_type = 2;
        condition = IN_OUT;
        return;
    }
    if(condition == END_JOB)
    {
        skipjob(DEF);
        fprintf(stderr, "error: nothing is expected after '&'\n");
        return;
    }
}

void getjob()
{
    freejob();
    conveyor = 0;
    curr_job.background = 0;
    curr_job.number_of_programs = 0;
    curr_job.programs = NULL;
    error = 0;
    if(c == '\n')
    {
        
        if(hist_size == 0 || history[hist_size - 1][0] != '\0')
        {
            history = (char **) realloc(history, (++hist_size) * sizeof(char*));
            history[hist_size - 1] = (char *) malloc(sizeof(char));
            history[hist_size - 1][0] = '\0';
            hist_curr = 0;
        }
        printf("vasia$ ");
        c = mgetchar();
    }
    do
        getsymbol();
    while(condition != END && (condition != IN_PROG || conveyor != 0));
    
    if(error == 1)
    {
        freejob();
        curr_job.background = 0;
        curr_job.number_of_programs = 0;
        curr_job.programs = NULL;
    }
    return;
}

int isspecsymbol(int c)
{
    if(c == ';'\
       || c == '|'\
       || c == '&'\
       || c == '\n'\
       || c == ' '\
       || c == '\t'\
       || c == EOF\
       || c == '>'\
       || c == '<')
        return 1;
    return 0;
}

void printjob()
{
    int i, j;
    if(curr_job.number_of_programs == 0)
        return;
    printf("Job:\nBackground = %d\n", curr_job.background);
    for(i = 0; i < curr_job.number_of_programs; i++)
    {
        printf("\tProgram #%d: %s\n", i, (curr_job.programs[i]).name);
        for(j = 0; j < (curr_job.programs[i]).number_of_arguments; j++)
        {
            printf("\t\tArguement #%d: %s\n", j, (curr_job.programs[i]).arguments[j]);
        }
        if((curr_job.programs[i]).input_file != NULL)
            printf("\tFrom \"%s\"\n", (curr_job.programs[i]).input_file);
        else
            printf("\tFrom stdin\n");
        if((curr_job.programs[i]).output_file != NULL)
            printf("\tTo \"%s\" (in mode %d)\n\n", (curr_job.programs[i]).output_file, (curr_job.programs[i]).output_type);
        else
            printf("\tTo stdout\n\n");
    }
}

void freejob()
{
    int i, j;
    for(i = 0; i < curr_job.number_of_programs; i++)
    {
        for(j = 1; j < (curr_job.programs[i]).number_of_arguments; j++)
        {
            free((curr_job.programs[i]).arguments[j]);
        }
        free((curr_job.programs[i]).arguments);
        free((curr_job.programs[i]).name);
        free((curr_job.programs[i]).input_file);
        free((curr_job.programs[i]).output_file);
    }
    free(curr_job.programs);
}

void mem_error()
{
    fprintf(stderr, NO_MEM);
    free(token);
    error = 1;
    condition = END;
}

void skipjob(int reg)
{
    error = 1;
    condition = END_JOB;
    
    if(reg == SQ)
        goto S;
    if(reg == DQ)
        goto D;
    
    while(c != '\n' && c != EOF && c != ';')
    {
        if(c == '\\')
        {
            c = mgetchar();
            if(c != EOF)
                c = mgetchar();
            continue;
        }
        
        if(c == '\'')
        {
            c = mgetchar();
        S:	while(c != '\'' && c != EOF)
        {
            c = mgetchar();
            if(c == '\\')
            {
                c = mgetchar();
                if(c != EOF)
                    c = mgetchar();
            }
        }
            if(c == '\'')
                c = mgetchar();
            continue;
        }
        
        if(c == '\"')
        {
            c = mgetchar();
        D:	while(c != '\"' && c != EOF)
        {
            c = mgetchar();
            if(c == '\\')
            {
                c = mgetchar();
                if(c != EOF)
                    c = mgetchar();
            }
        }
            if(c == '\"')
                c = mgetchar();
            continue;
        }
        
        if(c == ' ' || c == '\t')
        {
            c = mgetchar();
            if(c == '#')
                while(c != '\n' && c != EOF)
                {
                    mungetchar(1);
                    c = mgetchar();
                }
            continue;
        }
        c = mgetchar();
    }
}

void init()
{
    setenv("USER", "vasia", 1);
    temp_str = malloc((SIZEOFPID + 1) * sizeof(char));
    if(temp_str == NULL)
    {
        condition = END;
        return;
    }
    snprintf(temp_str, SIZEOFPID, "%d", getpid());
    setenv("PID", temp_str, 0);
    free(temp_str);
    temp_str = malloc((SIZEOFUID + 1) * sizeof(char));
    if(temp_str == NULL)
    {
        condition = END;
        return;
    }
    snprintf(temp_str, SIZEOFUID, "%d", getuid());
    setenv("UID", temp_str, 0);
    free(temp_str);
    temp_str = getcwd(NULL, 0);
    if(temp_str == NULL)
    {
        condition = END;
        return;
    }
    setenv("PWD", temp_str, 1);
    free(temp_str);
    signal(SIGTTOU, SIG_IGN);
}

char *mgetenv(int reg)
{
    char *var = NULL;
    char *name = NULL;
    unsigned int num = 0;
    int i = 1;
    c = mgetchar();
    if(c == '?')
    {
        c = mgetchar();
        for(num = last_condition, i = 1; (num /= 10) > 0; i++);
        var = (char*) malloc((i + 1) * sizeof(char));
        if(var == NULL)
        {
            mem_error();
            return NULL;
        }
        for(num = last_condition, var[i--] = '\0'; i >= 0; num /= 10, i--)
            var[i] = num % 10 + '0';
        return var;
    }
    if(c == '#')
    {
        c = mgetchar();
        for(num = margc, i = 1; (num /= 10) > 0; i++);
        var = (char*) malloc((i + 1) * sizeof(char));
        if(var == NULL)
        {
            mem_error();
            return NULL;
        }
        for(num = margc, var[i--] = '\0'; i >= 0; num /= 10, i--)
            var[i] = num % 10 + '0';
        return var;
    }
    if(c >= '0' && c <= '9')
    {
        while(c >= '0' && c <= '9')
        {
            num *= 10;
            num += c - '0';
            c = mgetchar();
        }
        if(num >= margc)
        {
            var = (char*) malloc(sizeof(char));
            if(var == NULL)
            {
                mem_error();
                return NULL;
            }
            var[0] = '\0';
            return var;
        }
        var = (char*) malloc((strlen(margv[num]) + 1) * sizeof(char));
        if(var == NULL)
        {
            mem_error();
            return NULL;
        }
        strcpy(var, margv[num]);
        return var;
    }
    if(c == '{')
    {
        c = mgetchar();
        name = (char*) malloc(sizeof(char));
        if (name == NULL)
        {
            mem_error();
            return NULL;
        }
        name[0] = '\0';
        
        while(isalpha(c) || isdigit(c) || c == '_')
        {
            temp_str = (char*) realloc(name, (strlen(name) + 2) * sizeof(char));
            if(temp_str == NULL)
            {
                free(name);
                mem_error();
                return NULL;
            }
            name = temp_str;
            name[strlen(name) + 1] = '\0';
            name[strlen(name)] = c;
            c = mgetchar();
        }
        
        temp_str = NULL;
        
        if(c != '}' || isdigit(name[0]))
        {
            skipjob(reg);
            if(c == EOF)
                putchar('\n');
            fprintf(stderr, "error: bad ${%s} substitution\n", name);
            free(name);
            free(token);
            return NULL;
        }
        
        c = mgetchar();
        temp_str = getenv(name);
        free(name);
        
        if(temp_str == NULL)
        {
            var = (char*) malloc(sizeof(char));
            if(var == NULL)
            {
                mem_error();
                return NULL;
            }
            var[0] = '\0';
            return var;
        }
        
        var = malloc((strlen(temp_str) + 1) * sizeof(char));
        if(var == NULL)
        {
            mem_error();
            return NULL;
        }
        strcpy(var, temp_str);
        temp_str = NULL;
        return var;
    }
    else
    {
        var = (char*) malloc(2 * sizeof(char));
        if (var == NULL)
            return NULL;
        var[0] = '$';
        var[1] = '\0';
        return var;
    }
}

void mcd()
{
    if(curr_job.programs[0].number_of_arguments != 3)
        printf("cd: invalid input\n");
    else if(chdir(curr_job.programs[0].arguments[1]) < 0)
        perror("cd");
    else
    {
        temp_str = getcwd(NULL, 0);
        if(temp_str == NULL)
        {
            condition = END;
            return;
        }
        setenv("PWD", temp_str, 1);
        free(temp_str);
    }
}

int mgetchar()
{
    int ch;
    if(hist == 0)
        ch = getchar();
    else
    {
        ch = history[hist - 1][hist_rec++];
        if(ch == '\0')
        {
            /* putinhist(temp_c); */
            hist = 0;
            ch = temp_c;
        }
        
        
    }	
    temp_str = realloc(history[hist_size - 1], (hist_curr + 2) * sizeof(char));
    if(temp_str == NULL)
    {
        mem_error();
        return EOF;
    }
    history[hist_size - 1] = temp_str;
    if(ch != EOF)
        history[hist_size - 1][hist_curr++] = (char) ch;
    else
        history[hist_size - 1][hist_curr++] = '\n';	
    history[hist_size - 1][hist_curr] = '\0';
    return ch;
}

void putinhist(int ch)
{
    temp_str = realloc(history[hist_size - 1], (hist_curr + 2) * sizeof(char));
    if(temp_str == NULL)
    {
        mem_error();
        return;
    }
    history[hist_size - 1] = temp_str;
    if(ch != EOF)
        history[hist_size - 1][hist_curr++] = (char) ch;
    else
        history[hist_size - 1][hist_curr++] = '\n';	
    history[hist_size - 1][hist_curr] = '\0';
}

void mungetchar(int step)
{
    if((hist_curr -= step) < 0)
        return;
    history[hist_size - 1][hist_curr] = '\0';
    history[hist_size - 1] = realloc(history[hist_size - 1], (hist_curr + 1) * sizeof(char));
}

void historyprint()
{
    int i, lim;
    
    lim = hist_size;
    if(c != '\n')
        lim--;
    for(i = 0; i < lim; i++)
        printf("*%d %s\n", i + 1, history[i]);
}

void deletehistory()
{
    int i;
    for(i = 0; i < hist_size; i++)
        free(history[i]);
    free(history);
    hist_size = 0;
}
