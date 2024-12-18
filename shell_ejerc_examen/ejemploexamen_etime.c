#include "job_control.h"   // remember to compile with module job_control.c 
#define MAX_LINE 256 /* 256 chars per line, per command, should be enough. */

// -----------------------------------------------------------------------
//                            MAIN          
// -----------------------------------------------------------------------

//
//				Manejador para sigchld
//

#define time_sub(a,b) \
	a.tv_sec -= b.tv_sec; \
	a.tv_nscec -= b.tv_nsec; \
	if (altv_nsec < 0) {a.tv_nsec += 1000000000;} \



void manejador_chld (int num)
{
	int pid;

	int wstatus;

	
	while(1)
	{
		pid = waitpid(-1, &wstatus, WNOHANG); // wait no bloqueante, solo recoge los hijos que están pendientes de recoger

		if (pid == -1)
			return;

		if (pid == 0)
		{
			printf("Ningún proceso pendiente\n");
			break;
		}
		
		if(WIFSTOPPED(wstatus))
			printf("Proceso stopped con pid: %d (signal=%d)\n", pid, WSTOPSIG(wstatus));	
		else if(WIFCONTINUED(wstatus))
			printf("Proceso reanudado con pid: %d \n", pid);
		else if(WIFSIGNALED(wstatus))
			printf("Signaled proc pid: %d (signal = %d) (core? %d)\n", pid, WTERMSIG(wstatus), WCOREDUMP(wstatus));
		else if(WIFEXITED(wstatus))
			printf("Ya ha terminado el proceso con pid: %d (status=%d)\n", pid, WEXITSTATUS(wstatus));	
	}
}


struct *job lista;


int main(void)
{
	

	char inputBuffer[MAX_LINE]; /* buffer to hold the command entered */
	int background = 0;             /* equals 1 if a command is followed by '&' */
	char *args[MAX_LINE/2];     /* command line (of 256) has max of 128 arguments */
	// probably useful variables:
	int pid_fork, pid_wait; /* pid for created and waited process */
	int status;             /* status returned by wait */
	enum status status_res; /* status processed by analyze_status() */
	int info;
					/* info processed by analyze_status() */

	int etime; //
	
	signal(SIGINT,  SIG_IGN); // crtl+c interrupt tecleado en el terminal
    signal(SIGQUIT, SIG_IGN); // ctrl+\ quit tecleado en el terminal
    signal(SIGTSTP, SIG_IGN); // crtl+z Stop tecleado en el terminal
    signal(SIGTTIN, SIG_IGN); // proceso en segundo plano quiere leer del terminal 
    signal(SIGTTOU, SIG_IGN); // proceso en segundo plano quiere escribir en el terminal



	while (1)   /* Program terminates normally inside get_command() after ^D is typed*/
	{
		printf("COMMAND->");
		fflush(stdout);
		get_command(inputBuffer, MAX_LINE, args, &background);  /* get next command */
		
		etime = 0;

		if(args[0]==NULL) continue;   // if empty command
		
		if (strcmp(args[0], "cd") == 0)
		{
			if (args[1] == NULL)
				chdir("/");
			else if (chdir(args[1]) != 0)
				printf("No se puede cambiar al directorio %s\n", args[1]);
			continue;
		}
		else if (strcmp(args[0], "etime") == 0)
		{
			etime = 1;
			if(!args[1]) continue;
			char *args = &args[1]; //preguntar por esto
			printf("Comando etime Detectado\n");

			struct timespec start_time;
			struct timespec end_time;

			clock_gettime(CLOCK_MONOTONIC, &start_time);

		}
		pid_fork = fork();
		if(setpgid(pid_fork, pid_fork) != 0)
		{
			fprintf(stderr, "No se ha establecido correctamente el grupo");
			exit(1);
		}
		if (pid_fork < 0)
		{
			perror("Fork ha fallado");
			exit(1);
		}
		if (pid_fork == 0)
		{
			signal(SIGINT,  SIG_DFL); // crtl+c interrupt tecleado en el terminal
    		signal(SIGQUIT, SIG_DFL); // ctrl+\ quit tecleado en el terminal
    		signal(SIGTSTP, SIG_DFL); // crtl+z Stop tecleado en el terminal
    		signal(SIGTTIN, SIG_DFL); // proceso en segundo plano quiere leer del terminal 
    		signal(SIGTTOU, SIG_DFL); // proceso en segundo plano quiere escribir en el terminal

			if (background == 0)
			{
				if (tcsetpgrp(STDIN_FILENO, getpgid(getpid())) == -1)   //preguntar si es necesario getpgid ***
				{
					fprintf(stderr,"Error cediendo terminal");
					exit(1);
				}
			}

			if(execvp(args[0], args) == -1)
			{
				fprintf(stderr, "Error, command not found: %s\n", args[0]);
				exit(255);
			}

			if (background == 0)
			{
				if (tcsetpgrp(STDIN_FILENO, getpgid(getppid())) == -1)
				{
					fprintf(stderr,"Error recuperando terminal");
					exit(1);
				}
			}

		}
		if (pid_fork > 0)
		{
			if (background == 0)
			{
				if (tcsetpgrp(STDIN_FILENO, getpgid(pid_fork)) == -1)
				{
					fprintf(stderr,"Error cediendo terminal");
					exit(1);
				}
				pid_wait = waitpid(pid_fork, &status, WUNTRACED);
				
				if (pid_wait < 0)
				{
					perror("Fallo en wait");
					exit(1);
				}

				if (tcsetpgrp(STDIN_FILENO, getpgid(getpid())) == -1)
				{
					fprintf(stderr,"Error recuperando terminal");
					exit(1);
				}
				analyze_status(status, &info);
				if(WIFSTOPPED(status))
					printf("Foreground pid: %d, command: %s, Stopped, info: %d\n", pid_fork, args[0], info);	
				else if(WIFCONTINUED(status))
					printf("Foreground pid: %d, command: %s, Continued, info: %d\n", pid_fork, args[0], info);
				else if(WIFSIGNALED(status))
					printf("Foreground pid: %d, command: %s, Signaled, info: %d\n", pid_fork, args[0], info);
				else if(WIFEXITED(status))
					printf("Foreground pid: %d, command: %s, Exited, info: %d\n", pid_fork, args[0], info);
				if (etime == 1)
				{
					clock_gettime(CLOCK_MONOTINIC, &end_time);
					time_sub(end_time, start_time);
				}
			}
			else
				printf("Background job running... pid: %d, command: %s\n", pid_fork, args[0]);
		}
	}
}
