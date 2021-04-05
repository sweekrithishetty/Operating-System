#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/wait.h>
#include <sys/types.h>

//limits
#define MAX_TOKENS 100
#define MAX_STRING_LEN 100

size_t MAX_LINE_LEN = 10000;

// builtin commands
#define EXIT_STR "exit"
#define EXIT_CMD 0
#define UNKNOWN_CMD 99

FILE *fp; // file struct for stdin
char **tokens;
char *line;

//Declaration type
typedef struct proc_job{
	char  *name;
	pid_t pid; 
	int var; 
}proc_job ;

proc_job *back_jobs;



void initialize()
{
    //Allocate space for the whole line
	assert( (line = malloc(sizeof(char) * MAX_STRING_LEN)) != NULL);
	//Allocate space for individual tokens
	assert( (tokens = malloc(sizeof(char*)*MAX_TOKENS)) != NULL);
	//Open stdin as a file pointer 
	assert( (fp = fdopen(STDIN_FILENO, "r")) != NULL);
	//Allocate space for background jobs
	assert((back_jobs = malloc(sizeof( proc_job) * MAX_TOKENS)) != NULL);
}

int tokenize (char *string)
{
	int token_count = 0;
	int size = MAX_TOKENS;
	char *this_token;

	while ((this_token = strsep( &string, " \t\v\f\n\r")) != NULL) {

		if(*this_token == '\0')
		{
			continue;
		} 
		//Storing the number of counts intiated
        tokens[token_count] = this_token;
        printf("Token %d: %s\n", token_count, tokens[token_count]);
		token_count++;
		// if there are more tokens than space ,reallocate more space
		if(token_count >= size){
			size*=2;
			assert ((tokens = realloc(tokens, sizeof(char*) * size))!= NULL);
		}
	}
	return token_count;
}

int read_command() 
{
	// getline will reallocate if input exceeds max length
	assert( getline(&line, &MAX_LINE_LEN, fp) > -1); 
	printf("Shell read this line: %s\n", line);
	return tokenize(line);
}
void  listjobs()
{
	int counter = 0; 
	for (int i = 0; i<MAX_TOKENS; ++i)
	{
		pid_t id = back_jobs[i].pid;
			int status, child_1;
			//returns the status of the child process
			child_1 = waitpid(id, &status, WNOHANG);
		if(back_jobs[i].name)
		{
			counter++;
			if (child_1 > 0)
			{
				printf("Command %s with PID %d Status: %s \n", back_jobs[i].name, back_jobs[i].pid, "Finished");	
			}
			else if (child_1 <0)
			{
				printf("Command %s with PID %d Status: %s \n", back_jobs[i].name, back_jobs[i].pid, "Finished");	
			}
			else
			{	
				printf("Command %s with PID %d Status: %s \n", back_jobs[i].name, back_jobs[i].pid, "Running");
			}
		}
	}

}
void bg_to_fg(int pid){
	int status = 0;
	waitpid(pid, &status, 0);
		
}


void execute(int total_tokens)
{
	pid_t pid; 
    //creating  a new process by duplicating the calling process
	pid = fork();
    //Condition
    if (pid < 0){
		perror("Fork failed:");
		exit(1);
	}
	else if(pid ==0){	
		if(execvp(tokens[0],tokens) == -1)
		{
			fprintf(stderr, "Command not found \n");
		}
		exit(0);
	}
	else{
		int child_2; 
		int status;
		child_2 = waitpid(pid,&status,0);	
		if (child_2 > 0){
			free(tokens);
			assert( (tokens = malloc(sizeof(char*)*MAX_TOKENS)) != NULL);
		}
	}	
}
int run_command(int total_tokens) {
	//Verifying if token is empty
	while(!tokens[0]){
     return 1;
	}
	// tokens[0] will not  return empty
	if(strcmp(tokens[0],"fg")== 0)
	{
		int pid = atoi(tokens[1]);
		bg_to_fg(pid);
		return 1;
	}
	else if(strcmp( tokens[0], EXIT_STR )== 0)
	{
		return EXIT_CMD;
	}
	else if (strcmp(tokens[0],"listjobs")== 0)
	{
		listjobs();
		return 1;
	}
	else{
		//Token must be greater than 0 and less than or equal to 1
		if(total_tokens>0 && total_tokens<=1)
		{
			execute(total_tokens);
		}
		//Token must be greater than 1
		else if(total_tokens>1)
		{
			char * str = tokens[total_tokens - 1];
			if (strcmp(str, "&") == 0)
			{
				char *temp[total_tokens];
				for (int i = 0; i < total_tokens-1; ++i)
				{
					temp[i] = tokens[i];
				}
				temp[total_tokens-1] = NULL; 
				pid_t  pid; 
				pid = fork();
				if (pid < 0)
				{
					perror("Fork failed:");
					exit(1);
				}
				else if(pid ==0)
				{
					//Inside child process
				    if(execvp(tokens[0],temp) == -1)
					{
						fprintf(stderr, "Invalid command \n");
					}
					exit(0);
				}
				else
				{ 
					char *s1; 
					s1= malloc(sizeof(char *) * MAX_TOKENS);
					proc_job  jobs  = {s1, pid, 0};
					strcpy(s1, tokens[0]);
					int i = 0; 
					while(back_jobs[i].name)
					{
						i++;
					}
					back_jobs[i] = jobs;	
				}
			}
			else
			{
				execute(total_tokens);
			}
		}
	}
	return UNKNOWN_CMD;
}

int main()
{
	initialize();
	int total_tokens = 0;
	do
	{
		printf("sh550> ");
		total_tokens = read_command();		
	} while(run_command(total_tokens) != EXIT_CMD );
     return 0;
}