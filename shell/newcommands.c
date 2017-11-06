#include "shell.h"
extern int i;
extern int condition;
extern struct job curr_job;

void runintegrateprograms()
{
    if(curr_job.number_of_programs == 1)
    {
        if(strcmp(curr_job.programs[0].name,"exit") == 0)
        {
            condition = END;
            return;
        }
        
        if(strcmp(curr_job.programs[0].name, "jobs") == 0)
        {
            print_procs();
            return;
        }
        if(strcmp(curr_job.programs[0].name, "fg") == 0)
        {
            mfg();
            return;
        }
        if(strcmp(curr_job.programs[0].name, "bg") == 0)
        {
            mbg();
            return;
        }
        if(strcmp(curr_job.programs[0].name, "cd") == 0)
        {
            mcd();
            return;
        }
        if(strcmp(curr_job.programs[0].name, "pwd") == 0)
        {
            printf("%s\n", getenv("PWD"));
            return;
        }
        if(strcmp(curr_job.programs[0].name, "history") == 0)
        {
            historyprint();
            return;
        }
        
    }
}

void mcat()
{
    int ch;
    FILE *f = stdin;
    if(curr_job.programs[i].number_of_arguments > 3)
    {
        fprintf(stderr,"mcat: invalid input\n");
        return;
    }
    if(curr_job.programs[i].number_of_arguments == 3)
    {
        if ((f = fopen(curr_job.programs[i].arguments[1], "r")) == NULL)
        {
            perror(curr_job.programs[i].arguments[1]);
            return;
        }
    }
    while (1)
    {
        ch = getc(f);
        if(ch == EOF)
            break;
        putchar(ch);
    }
    if(f != stdin)
        fclose(f);
    return;
}

void mgrep()
{
    int reverse = 0;
    int ch;
    int j;
    FILE *f = stdin;
    char *str;
    char *substr;
    char *temp_str;
    
    if((curr_job.programs[i].arguments[1]) == NULL)
    {
        fprintf(stderr, "mgrep: invalid input\n");
        return;
    }
    if(strcmp(curr_job.programs[i].arguments[1], "-v") == 0)
    {
        reverse = 1;
        if (curr_job.programs[i].arguments[2] == NULL)
        {
            fprintf(stderr, "mgrep: invalid input\n");
            return;
        }
        substr = curr_job.programs[i].arguments[2];
        if(curr_job.programs[i].arguments[3] != NULL)
        {
            if(curr_job.programs[i].arguments[4] == NULL)
            {
                if ((f = fopen(curr_job.programs[i].arguments[3], "r")) == NULL)
                {
                    perror(curr_job.programs[i].arguments[3]);
                    return;
                }
            }
            else
            {
                fprintf(stderr, "mgrep: invalid input\n");
                return;
            }
        }
    }
    else
    {
        substr = curr_job.programs[i].arguments[1];
        if(curr_job.programs[i].arguments[2] != NULL)
        {
            if(curr_job.programs[i].arguments[3] == NULL)
            {
                if ((f = fopen(curr_job.programs[i].arguments[2], "r")) == NULL)
                {
                    perror(curr_job.programs[i].arguments[2]);
                    return;
                }
            }
            else
            {
                fprintf(stderr, "mgrep: invalid input\n");
                return;
            }
        }
    }
    
    for(;;)
    {
        for(j = 0, temp_str = NULL, str = NULL; ; j++)
        {
            ch = getc(f);
            temp_str = (char *) realloc(temp_str, (j + 1) * sizeof(char));
            if (temp_str == NULL)
            {
                fprintf(stderr, "mgrep: failed to allocate memory\n");
                free(str);
                return;
            }
            str = temp_str;
            if (ch == EOF || ch == '\n')
            {
                str[j] = '\0';
                break;
            }
            else
                str[j] = ch;
        }
        
        if((reverse == 0 && strstr(str, substr) != NULL) || (reverse == 1 && strstr(str, substr) == NULL))
            printf("%s\n", str);
        
        free(str);
        
        if (ch == EOF)
            break;
    }
    
    if (f != stdin)
        fclose(f);
    
    return;
}

void msed()
{
    int regime = 0;
    int ch;
    int j;
    long int shift;
    FILE *f = stdin;
    char *str;
    char *str1, *str2;
    char *temp_str;
    char *str_found;
    if((curr_job.programs[i].arguments[1]) == NULL)
    {
        fprintf(stderr, "msed: invalid input\n");
        return;
    }
    
    if(strcmp(curr_job.programs[i].arguments[1], "^") == 0)
        regime = 1;
    else if(strcmp(curr_job.programs[i].arguments[1], "$") == 0)
        regime = 2;
    else
        str1 = curr_job.programs[i].arguments[1];
    
    if (curr_job.programs[i].arguments[2] == NULL)
    {
        fprintf(stderr, "msed: invalid input\n");
        return;
    }
    
    str2 = curr_job.programs[i].arguments[2];
    
    if(curr_job.programs[i].arguments[3] != NULL)
    {
        if(curr_job.programs[i].arguments[4] == NULL)
        {
            if ((f = fopen(curr_job.programs[i].arguments[3], "r")) == NULL)
            {
                perror(curr_job.programs[i].arguments[3]);
                return;
            }
        }
        else
        {
            fprintf(stderr, "msed: invalid input\n");
            return;
        }
    }
    
    for(;;)
    {
        for(j = 0, temp_str = NULL, str = NULL; ; j++)
        {
            ch = getc(f);
            temp_str = (char *) realloc(str, (j + 1) * sizeof(char));
            if (temp_str == NULL)
            {
                fprintf(stderr, "msed: failed to allocate memory\n");
                free(str);
                return;
            }
            str = temp_str;
            if (ch == EOF || ch == '\n')
            {
                str[j] = '\0';
                break;
            }
            else
                str[j] = (char) ch;
        }
        if(j == 0 && ch == EOF)
        {
            free(str);
            break;
        }
        str_found = str;
        switch(regime)
        {
            case 0:
            {
                while((str_found = strstr(str_found, str1)) != NULL)
                {
                    shift = str_found - str;
                    
                    if(strlen(str2) == strlen(str1))
                    {
                        memcpy(str_found, str2, strlen(str2) * sizeof(char));
                        str_found+=strlen(str2);
                    }
                    else if(strlen(str2) > strlen(str1))
                    {
                        temp_str = realloc(str, (strlen(str) + strlen(str2) - strlen(str1) + 1) * sizeof(char));
                        if(temp_str == NULL)
                        {
                            fprintf(stderr, "msed: failed to allocate memory\n");
                            free(str);
                            return;
                        }
                        str = temp_str;
                        str_found = &str[shift];
                        memmove(str_found + strlen(str2), str_found + strlen(str1), str + strlen(str) - str_found - strlen(str1) + 1);
                        strncpy(str_found, str2, strlen(str2));
                        str_found += strlen(str2);
                    }
                    else
                    {
                        strncpy(str_found, str2, strlen(str2));
                        memmove(str_found + strlen(str2), str_found + strlen(str1), str + strlen(str) - str_found - strlen(str1) + 1);
                        temp_str = realloc(str, (strlen(str) + 1) * sizeof(char));
                        if(temp_str == NULL)
                        {
                            fprintf(stderr, "msed: failed to allocate memory\n");
                            free(str);
                            return;
                        }
                        str = temp_str;
                        str_found = &str[shift + strlen(str2)];
                    }			
                }
                break;
            }
                
            case 1:
            {
                temp_str = realloc(str, (strlen(str) + strlen(str2) + 1) * sizeof(char));
                if(temp_str == NULL)
                {
                    fprintf(stderr, "msed: failed to allocate memory\n");
                    free(str);
                    return;
                }
                str = temp_str;
                memmove(str + strlen(str2), str,(strlen(str) + 1) * sizeof(char));
                memcpy(str, str2, strlen(str2) * sizeof(char));
                break;
            }
                
            case 2:
            {
                temp_str = realloc(str, (strlen(str) + strlen(str2) + 1) * sizeof(char));
                if(temp_str == NULL)
                {
                    fprintf(stderr, "msed: failed to allocate memory\n");
                    free(str);
                    return;
                }
                str = temp_str;
                strcat(str, str2);
                break;
            }
                
            default:
                break;
        }
        printf("%s\n", str);
        free(str);
        
        if (ch == EOF)
            break;
    }
    
    if (f != stdin)
        fclose(f);
    
    return;
}
