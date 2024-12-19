#include "job_control.h"   // remember to compile with module job_control.c 
#include "parse_redir.h"




#define MAX_LINE 256 /* 256 chars per line, per command, should be enough. */

// -----------------------------------------------------------------------
//                            MAIN          
// -----------------------------------------------------------------------

//
//				Manejador para sigchld
//


job *lista_jobs;  //creamos lista de procesos no terminados.


void manejador_sighup(int num)
{
	FILE *fp;
	fp = fopen("hup.txt", "a");
	fprintf(fp, "SIGHUP RECIBIDO.\n");
	fclose(fp);

}


void manejador_chld (int num)
{
	int pid;

	int wstatus;

	
	while(1)
	{
		pid = waitpid(-1, &wstatus, WNOHANG | WUNTRACED | WCONTINUED); // wait no bloqueante, solo recoge los hijos que están pendientes de recoger

		if (pid == -1)
		{
			return;
		}
		if (pid == 0)
		{
			return;
		}
		
		//actualizamos el estado

		job *actualizar_trabajo = get_item_bypid(lista_jobs, pid);

		if(WIFSTOPPED(wstatus)) // cambiamos el estado a stopped
		{
			printf("Proceso stopped con pid: %d (signal=%d)\n", pid, WSTOPSIG(wstatus));
			actualizar_trabajo->state = STOPPED;
		}
		else if(WIFCONTINUED(wstatus)) 
		{
			printf("Proceso reanudado con pid: %d \n", pid);
			actualizar_trabajo->state = BACKGROUND;
		}
		else if(WIFSIGNALED(wstatus)) //eliminamos de la lista
		{
			printf("Signaled proc pid: %d (signal = %d) (core? %d)\n", pid, WTERMSIG(wstatus), WCOREDUMP(wstatus));
			delete_job(lista_jobs, actualizar_trabajo);
		}
		else if(WIFEXITED(wstatus)) //eliminamos de la lista
		{
			printf("Ya ha terminado el proceso con pid: %d (status=%d)\n", pid, WEXITSTATUS(wstatus));
			delete_job(lista_jobs, actualizar_trabajo);
		}
	}
}


int main(void)
{
	

	char inputBuffer[MAX_LINE]; /* buffer to hold the command entered */
	int background = 0;             /* equals 1 if a command is followed by '&' */
	char *args[MAX_LINE/2];     /* command line (of 256) has max of 128 arguments */
	// probably useful variables:
	int pid_fork, pid_wait; /* pid for created and waited process */
	int status;             /* status returned by wait */
	enum status status_res; /* status processed by analyze_status() */
	int info;  /* info processed by analyze_status() */

	lista_jobs = new_list("Lista de trabajos");
	
	ignore_terminal_signals();

	signal(SIGCHLD, manejador_chld);
	signal(SIGHUP, manejador_sighup);
	

	while (1)   /* Program terminates normally inside get_command() after ^D is typed*/
	{		
		printf("COMMAND->");
		fflush(stdout);
		get_command(inputBuffer, MAX_LINE, args, &background);  /* get next command */
		
		char *file_in, *file_out;
		parse_redirections(args, &file_in, &file_out);


		if(args[0]==NULL)
		{
			continue;   // if empty command
		}
		
		if (strcmp(args[0], "cd") == 0)
		{
			if (args[1] == NULL)
				chdir("/");
			else if (chdir(args[1]) != 0)
				printf("No se puede cambiar al directorio %s\n", args[1]);
			continue;
		}
		else if (strcmp(args[0], "jobs") == 0)
		{
			mask_signal(SIGCHLD, SIG_BLOCK);
			print_list(lista_jobs, print_item);
			mask_signal(SIGCHLD, SIG_UNBLOCK);
		}
		else if(strcmp(args[0], "fg") == 0)
		{
			
			mask_signal(SIGCHLD, SIG_BLOCK);
			int pos;

			if ((args[1] != NULL && atoi(args[1]) <= 0))
			{
				printf("Parámetro incorrecto para el comando fg.\n");
			}
			else
			{
				if (args[1] != NULL)
				{
					pos = atoi(args[1]);
				}
				if (args[1] == NULL)
				{
					pos = 1;
				}
				job* fg_job = get_item_bypos(lista_jobs, pos);
				
				if (fg_job == NULL)
				{
					perror("No existe un trabajo en esa posición");
					mask_signal(SIGCHLD, SIG_UNBLOCK);
					continue;
				}

				int pgrp = fg_job->pgid;
				
				if (tcsetpgrp(STDIN_FILENO, pgrp) == -1)
				{
					perror("Error cediendo terminal");
					mask_signal(SIGCHLD, SIG_UNBLOCK);
					continue;
				}

				fg_job->state = FOREGROUND;
				
				killpg(pgrp, SIGCONT);
				int wstatus;
				pid_t pid_wait = waitpid(-pgrp, &wstatus, WUNTRACED);

				if (pid_wait < 0)
				{
					perror("Fork ha fallado");
					mask_signal(SIGCHLD, SIG_UNBLOCK);
					exit (EXIT_FAILURE);
				}

				if (tcsetpgrp(STDIN_FILENO, getpid()) == -1)
					{
						perror("Error recuperando terminal");
						mask_signal(SIGCHLD, SIG_UNBLOCK);
						continue;
					}

				analyze_status(wstatus, &info);

				if (WIFSTOPPED(wstatus)) // Si el proceso fue detenido
				{
					printf("Foreground job stopped. PID: %d, command: %s\n", pid_wait, fg_job->command);
					fg_job->state = STOPPED; // Actualizamos el estado en la lista de trabajos
				}
				else if(WIFSIGNALED(wstatus))
				{
					printf("Foreground pid: %d, command: %s, SIGNALED, info: %d\n", pid_wait, fg_job->command, info);
					if (delete_job(lista_jobs, fg_job) == 0)
					{
						perror("Fallo al eliminar trabajo que recibió señal");
						mask_signal(SIGCHLD, SIG_UNBLOCK);
						continue;
					}
				}
				else if(WIFEXITED(wstatus))
				{
					printf("Foreground pid: %d, command: %s, EXITED, info: %d\n", pid_wait, fg_job->command, info);
					if (delete_job(lista_jobs, fg_job) == 0)
					{
						perror("Fallo al eliminar trabajo exited");
						mask_signal(SIGCHLD, SIG_UNBLOCK);
						continue;
					}
				}
			}
			mask_signal(SIGCHLD, SIG_UNBLOCK);
		}
		else if(strcmp(args[0], "bg") == 0)
		{
			mask_signal(SIGCHLD, SIG_BLOCK);
			int pos;
			if ((args[1] != NULL && atoi(args[1]) <= 0))
			{
				perror("Parámetro incorrecto para el comando bg.\n");
			}
			else
			{
				if (args[1] != NULL)
				{
					pos = atoi(args[1]);
				}
				if (args[1] == NULL)
				{
					pos = 1;
				}
				job* bg_job = get_item_bypos(lista_jobs, pos);
				if (bg_job == NULL)
				{
					perror("Fallo obteniendo el trabajo (por posición)");
					mask_signal(SIGCHLD, SIG_UNBLOCK);
					continue;
				}
				if (bg_job->state!=STOPPED)
				{
					perror("No podemos reanudar un trabajo que no esté STOPPED");
					mask_signal(SIGCHLD, SIG_UNBLOCK);
					continue;
				}

				int pgrp = bg_job->pgid;
				bg_job->state = BACKGROUND;
				killpg(pgrp, SIGCONT);
			}
			mask_signal(SIGCHLD, SIG_UNBLOCK);
		}
		else if (strcmp(args[0], "currjob") == 0)
		{
			job* currjob = get_item_bypos(lista_jobs, 1);
			if (currjob)
			{
				printf("Trabajo actual: PID=%d command=%s\n", currjob->pgid,currjob->command);
			}else
			{
				printf("No hay trabajo actual");
			}
		}
		else if (strcmp(args[0], "deljob") == 0)
		{
			job* currjob = get_item_bypos(lista_jobs, 1);
			if(currjob)
			{
				if (currjob->state != STOPPED)
				{
					printf("Borrando trabajo actual de la lista de jobs: PID=%d command=%s\n", currjob->pgid, currjob->command);
					delete_job(lista_jobs, currjob);
				}else
				{
					printf("No se permiten borrar trabajos en segundo plano suspendidos\n");
				}
			}else
			{
				printf("No hay trabajo actual\n");
			}
		}
		else if (strcmp(args[0], "zjobs") == 0)
		{
			traverse_proc();
		}
		else if (strcmp(args[0], "bgteam") == 0)
		{
			if (args[1] != NULL && args[2]!= NULL)
			{
				int iters = atoi(args[1]);
				if (iters > 0)
				{
					int i = 2;
					while (args[i]!=NULL)
					{
						args[i-2] = args[i];
						i++;
					}
					args[i - 2] = '\0';
					{
						pid_t fork_bgteam = fork();
						if (fork_bgteam < 0)
						{
							perror("Fork ha fallado");
							exit(EXIT_FAILURE);
						}
						if (fork_bgteam == 0)
						{
							restore_terminal_signals();
							fork_bgteam = getpid();
							pid_t group_bgteam = fork_bgteam;
							if(setpgid(fork_bgteam, group_bgteam) != 0)
							{
								perror("No se ha establecido correctamente el grupo");
								exit(EXIT_FAILURE);
							}
							
							
							execvp(args[0], args);
							perror(args[0]);
							exit (EXIT_FAILURE);
						}

						if (fork_bgteam > 0)
						{
							printf("Background job running... pid: %d, command: %s\n", fork_bgteam, args[0]);
							mask_signal(SIGCHLD, SIG_BLOCK);
							job *nuevo_trabajo_2 = new_job(fork_bgteam, args[0], BACKGROUND);
							add_job(lista_jobs, nuevo_trabajo_2);
							mask_signal(SIGCHLD, SIG_UNBLOCK);
						}
						iters--;
					}
				}else{
					continue;
				}
			}else{
				printf("El comando bgteam requiere dos argumentos\n");
			}
		}
		else
		{
			pid_t pid_fork = fork();
		
			if (pid_fork < 0)
			{
				perror("Fork ha fallado");
				exit (EXIT_FAILURE);
			}

			if (pid_fork == 0)
			{
				restore_terminal_signals();

				pid_fork = getpid();
				pid_t group = pid_fork; 
				if(setpgid(pid_fork, group) != 0)
				{
					perror("No se ha establecido correctamente el grupo");
					exit(EXIT_FAILURE);
				}
				if (background == 0)
				{
					if (tcsetpgrp(STDIN_FILENO, getpid()) == -1) //getpgrp
					{
						perror("Error cediendo terminal");
						exit(EXIT_FAILURE);
					}
				}

				if (file_in != NULL)
				{
					int fd = open(file_in, O_RDONLY);
					if (fd == -1)
					{
						perror("No se ha podido abrir el fichero");
						continue;
					}
					if (dup2(fd, STDIN_FILENO)==-1)
					{
						perror("No se puede redirigir la entrada");
						continue;
					}
					close(fd);
				}

				if (file_out)
				{
					int fd = open(file_out, O_CREAT|O_WRONLY|O_TRUNC,  0644);
					if (fd == -1)
					{
						perror("No se ha podido crear el fichero");
						continue;
					}
					if (dup2(fd, STDOUT_FILENO)==-1)
					{
						perror("No se puede redirigir la entrada");
						continue;
					}
					close(fd);
				}


				execvp(args[0], args);
				perror(args[0]);
				exit (EXIT_FAILURE);
			}
			if (pid_fork > 0)
			{
				if (background == 0)
				{
					if (tcsetpgrp(STDIN_FILENO, pid_fork) == -1)
					{
						perror("Error cediendo terminal");
						continue;
					}
					//mask_signal(SIGCHLD, SIG_BLOCK);
					pid_wait = waitpid(pid_fork, &status, WUNTRACED);
					//mask_signal(SIGCHLD, SIG_UNBLOCK);

					if (pid_wait < 0)
					{
						perror("Fallo en wait");
						continue;
					}

					if (tcsetpgrp(STDIN_FILENO, getpid()) == -1)
					{
						perror("Error recuperando terminal");
						continue;
					}

					analyze_status(status, &info);

					if(WIFSTOPPED(status)) //añadimos a la lista
					{
						printf("Foreground pid: %d, command: %s, SUSPENDED, info: %d\n", pid_fork, args[0], info);	
						mask_signal(SIGCHLD, SIG_BLOCK);
						job *nuevo_trabajo_1 = new_job(pid_fork, args[0], STOPPED);
						add_job(lista_jobs, nuevo_trabajo_1);
						mask_signal(SIGCHLD, SIG_UNBLOCK);
					}
					else if(WIFSIGNALED(status))
					{
						printf("Foreground pid: %d, command: %s, SIGNALED, info: %d\n", pid_fork, args[0], info);
					}
					else if(WIFEXITED(status))
					{
						printf("Foreground pid: %d, command: %s, EXITED, info: %d\n", pid_fork, args[0], info);
					}
				}else
				{
					printf("Background job running... pid: %d, command: %s\n", pid_fork, args[0]);
					mask_signal(SIGCHLD, SIG_BLOCK);
					job *nuevo_trabajo_2 = new_job(pid_fork, args[0], BACKGROUND);
					add_job(lista_jobs, nuevo_trabajo_2);
					mask_signal(SIGCHLD, SIG_UNBLOCK);
				}
			}
		}
	}
}