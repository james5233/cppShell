#include "shell.h"

/* dispatch a shell-builtin in the same shell */
int dispatch_builtin(proc_list_t *p)
{
	int fd[10], i;
	proc_list_t *cur = p->next;

	for (i = 0; i < 10; i++)
		fd[i] = dup(i);

	char errname[MAXSTR] = {0};
	int fd_temp, j;

	for (j = 0; j < 10; j++) {
		if (cur->r.fd_red[j] < 0) {
			if (cur->r.rw[j] == RW_WRITE)
				fd_temp = open(cur->r.s_red[j].c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
			else
				fd_temp = open(cur->r.s_red[j].c_str(), O_RDONLY);

			if (fd_temp < 0) {
				int sav = errno;
				strcpy(errname, "shell: ");
				strcat(errname, cur->r.s_red[j].c_str());
				errno = sav;
				perror(errname);
				return 0;
			}

			dup2(fd_temp, j);
			close(fd_temp);
		} else if (cur->r.fd_red[j] != j) {
			fd_temp = dup2(cur->r.fd_red[j], j);
			if (fd_temp < 0) {
				int sav = errno;
				sprintf(errname, "shell: %d", cur->r.fd_red[j]);
				errno = sav;
				perror(errname);
				return 0;
			}
		}
	}

	strcpy(errname, "shell: ");
	strcat(errname, cur->argv[0]);

	bfptr func = builtin_fptr(cur->argv[0]);
	if (func)
		func((const char **)cur->argv);

	for (i = 0; i < 10; i++) {
		if (fd[i] >= 0) {
			dup2(fd[i], i);
			close(fd[i]);
		}
	}

	return 0;
}	

/* Dispatch multiple processes in subshells */
int dispatch_procs(proc_list_t *p, int nprocs)
{
	int i;
	proc_list_t *cur = p->next;
	
	/* The first child derives its input from it's stdin */
	int in = 0;

	/* For all but the last process in multipipe*/
	for (i = 0; i < nprocs-1; i++) {

		/* Get a pipe */
		int fd[2];
		pipe(fd);

		/* Make a child */
		if((p->pid = fork()) == 0) {

			signal(SIGINT, SIG_DFL);
			signal(SIGTSTP, SIG_DFL);

			if(in != 0) /* First child need not change its stdin */
			{
				dup2(in, 0); /* Other children connect pipe read end to stdin */
				close(in); /* close read end copy of pipe, use 0 */
			}

			dup2(fd[1], 1); /* All children connect pipe write end to stdout, we deal with last one later */
			close(fd[1]); /* Close write end copy of pipe, use 1*/
			close(fd[0]); /* Close read end, we don't read from it */

			char errname[MAXSTR] = {0};
			int fd_temp, j;

			for (j = 0; j < 10; j++) {
				if (cur->r.fd_red[j] < 0) {
					if (cur->r.rw[j] == RW_WRITE)
						fd_temp = open(cur->r.s_red[j].c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
					else
						fd_temp = open(cur->r.s_red[j].c_str(), O_RDONLY);

					if (fd_temp < 0) {
						int sav = errno;
						strcpy(errname, "shell: ");
						strcat(errname, cur->r.s_red[j].c_str());
						errno = sav;
						perror(errname);
						exit(errno);
					}

					dup2(fd_temp, j);
					close(fd_temp);
				} else if (cur->r.fd_red[j] != j) {
					fd_temp = dup2(cur->r.fd_red[j], j);
					if (fd_temp < 0) {
						int sav = errno;
						sprintf(errname, "shell: %d", cur->r.fd_red[j]);
						errno = sav;
						perror(errname);
						exit(errno);
					}
				}
			}

			strcpy(errname, "shell: ");
			strcat(errname, cur->argv[0]);
			/* Become a new program, never return (Assuming you don't fail) */
			bfptr func = builtin_fptr(cur->argv[0]);
			if (func) {
				func((const char **)cur->argv);
				exit_flag = TRUE;
				return 0;
			} else {
				execvp(cur->argv[0], cur->argv);
				perror(errname);
				exit(errno);
			}
		}

		in = fd[0]; /* parent needs to pass this to next child */
		close(fd[1]); /* Close the write end, this is not needed anymore */

		cur = cur->next;
	}

	if((cur->pid = fork()) == 0) {
		signal(SIGINT, SIG_DFL);
		signal(SIGSTOP, SIG_DFL);

		if (in != 0) /* If last child is the only child, it shouldn't change stdin*/
		{
			dup2(in, 0); /* Otherwise connect read end of pipe to stdin */
			close(in);  /* close that copy */
		}
		/* Last Child doesn't change its stdout */

		char errname[MAXSTR] = {0};
		int fd_temp, j;

		for (j = 0; j < 10; j++) {
			if (cur->r.fd_red[j] < 0) {
				if (cur->r.rw[j] == RW_WRITE)
					fd_temp = open(cur->r.s_red[j].c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
				else
					fd_temp = open(cur->r.s_red[j].c_str(), O_RDONLY);

				if (fd_temp < 0) {
					int sav = errno;
					strcpy(errname, "shell: ");
					strcat(errname, cur->r.s_red[j].c_str());
					errno = sav;
					perror(errname);
					exit(errno);
				}

				dup2(fd_temp, j);
				close(fd_temp);
			} else if (cur->r.fd_red[j] != j) {
				fd_temp = dup2(cur->r.fd_red[j], j);
				if (fd_temp < 0) {
					int sav = errno;
					sprintf(errname, "shell: %d", cur->r.fd_red[j]);
					errno = sav;
					perror(errname);
					exit(errno);
				}
			}
		}

		strcpy(errname, "shell: ");
		strcat(errname, cur->argv[0]);

		bfptr func = builtin_fptr(cur->argv[0]);
		if (func) {
			func((const char **)cur->argv);
			exit_flag = TRUE;
			return 0;
		} else {
			execvp(cur->argv[0], cur->argv);
			perror(errname);
			exit(errno);
		}
	}

	/* Reap your zombies */
	while (wait(NULL) && errno != ECHILD);

	return 0;
}
	
