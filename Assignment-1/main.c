#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#define MAX_TOKENS 100
#define MAX_STRING_LEN 100
size_t MAX_LINE_LEN = 10000;
#define EXIT_STR "exit"
#define EXIT_CMD 0
#define UNKNOWN_CMD 99
FILE *fp;
char **tokens;
char *line;
char **tok;
char *filename;
int inputFlag=0;
int outputFlag=0;
int redirect=0;



void initialize()
{
    // allocate space for the whole line
    assert( (line = malloc(sizeof(char) * MAX_STRING_LEN)) != NULL);

    // allocate space for individual tokens
    assert( (tokens = malloc(sizeof(char*)*MAX_TOKENS)) != NULL);

    // open stdin as a file pointer
    assert( (fp = fdopen(STDIN_FILENO, "r")) != NULL);

}

void execCommand() {
    pid_t pid;
    int status;
    /**
     * Regular input Commands
     */
    if(redirect==0)
    {
        if ((pid = fork()) < 0)
        {
            fprintf(stderr, "fork failed\n");
            exit(1);
        }

        if (pid == 0)
        {
            if (execvp(*tokens, tokens) < 0)
            exit(2);

        }
        else
            while (wait(&status) != pid)
                continue;
    }
    /**
     * Input Redirections
     */
    else if (redirect==1)
    {

        if ((pid = fork()) < 0)
        {
            fprintf(stderr, "fork failed\n");
            exit(1);
        }
        if (pid == 0 )
        {
            int fd0 = open(filename, O_RDONLY, 0);
            dup2(fd0,STDIN_FILENO);
            close(fd0);
            printf("input redirection");
            if (execvp(*tokens, tokens) < 0)
                exit(2);
//            redirect=-99;
        }
        else
            while (wait(&status) != pid)
                continue;

    }
    /**
     *Output Redirections
     */
    else if (redirect==2){
        printf("Entered output redirection, fliename is : %s",filename);
        if ((pid = fork()) < 0)
        {
            fprintf(stderr, "fork failed\n");
            exit(1);
        }
        if (pid == 0)
        {
            printf("output redirection");
            int fd= creat(filename, 0644);
            dup2(fd, STDOUT_FILENO);
            close(fd);
            if (execvp(*tokens, tokens) < 0)
                exit(2);
        }
        else
            while (wait(&status) != pid)
                continue;
//        redirect =-99;
    }
    else{
        printf("Something");
    }
}
void tokenize (char * string)
{
    int token_count = 0;
    int size = MAX_TOKENS;
    char *this_token;
    while ( (this_token = strsep( &string, " \t\v\f\n\r")) != NULL)
    {
        if (*this_token == '\0') continue;
        tokens[token_count] = this_token;
        execCommand();
        printf("Token %d: %s\n", token_count, tokens[token_count]);
        token_count++;
        if(token_count >= size){
            size*=2;
            assert ( (tokens = realloc(tokens, sizeof(char*) * size)) != NULL);
        }
    }
}
void read_command()
{

    assert( getline(&line, &MAX_LINE_LEN, fp) > -1);
    printf("Shell read this line: %s\n", line);
    if(strchr(line,'<'))
        redirect=1;
    else if(strchr(line,'>'))
        redirect=2;
    else
        redirect=0;

    char * pch;
    pch = strtok (line," ");
    while (pch != NULL)
    {
        if(strcmp(pch,"<")==0||strcmp(pch,">")==0)
        {
            pch = strtok (NULL, " ");
            filename=pch;
        }
        else
        {
            pch = strtok(NULL, " ");
        }
    }

    tokenize(line);
}


int run_command() {

    if (strcmp( tokens[0], EXIT_STR ) == 0)
        return EXIT_CMD;

    return UNKNOWN_CMD;
}

int main()
{
    initialize();
    do {
        printf("sh550> ");
        read_command();
    } while( run_command() != EXIT_CMD );

    return 0;
}
