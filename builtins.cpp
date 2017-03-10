#include "shell.h"

/* Builtin function for export command */
int builtin_export(const char *argv[])
{
	int i;
	for (i = 1; argv[i] != 0; i++) {
		char *name = strdup(argv[i]);
		char *val = strchr(name, '=');
		if (val != NULL && !(*(val++) = '\0'))
			setenv(name, val, 1); 
		free(name);
	}

	return 0;
}

/* Builtin function for cd command */
int builtin_cd(const char *argv[])
{

	char *dest_dir = strdup((argv[1] == NULL) ? getenv("HOME") : argv[1]);
	char errname[3*MAXSTR] = {0};
	sprintf(errname,"shell: cd: %s", dest_dir); 
	int res = chdir(dest_dir);
	int sav = errno;
	if (res < 0) {
		free(dest_dir);
		errno = sav;
		perror(errname);
		return -1;
	} else {
		char *full_dest_dir = (char *)malloc(MAXPATH);
		getcwd(full_dest_dir, MAXPATH -1);
		setenv("PWD", full_dest_dir, 1);
		free(dest_dir);
		free(full_dest_dir);
		return 0;
	}
}

/* Builtin function for pwd command */
int builtin_pwd(const char *argv[])
{
	puts(getenv("PWD"));
	return 0;
}

/* Builtin function for echo command */
int builtin_echo(const char *argv[])
{
	const char **s = argv + 1;
	if (*s != NULL)
	{
		printf("%s", *s);
		
		while (*(++s) != NULL)
			printf(" %s", *s);
	}
	printf("\n");
	return 0;
}

/* Builtin function for echo command */
int builtin_exit(const char *argv[])
{
	int r = 0;
	if (argv[1] != NULL)
		sscanf(argv[1], "%d", &r);
	
	exit_code = r;
	exit_flag = TRUE;

	return 0;
}

/* Builtin function for history command */
int builtin_history(const char *argv[])
{
	int num;
	if (argv[1] == NULL) {
		std::cout << dump_last_nlines(0);
		return 0;
	} else {
		num = -1;
		char str[MAXLINE];
		int res = sscanf(argv[1], "%d", &num);
		if ((res != 1) || (num < 0)) {
			fprintf(stderr, "shell: history: invalid arguments %s\n", argv[1]);
			return 1;
		} else {
			sprintf(str, "%d", num);
			if (strcmp(str, argv[1])) {
				fprintf(stderr, "shell: history: non-numeric argument %s\n", argv[1]);
				return 1;
			} else {
				std::cout << dump_last_nlines(num);
				return 0;
			}
		}
	}
}

/* Resolve builtin name to function pointer : NULL if not a builtin */
bfptr builtin_fptr(const char *name)
{
	if (name == NULL)
		return NULL;
	else if (!strcmp(name, "echo"))
		return builtin_echo;
	else if (!strcmp(name, "cd"))
		return builtin_cd;
	else if (!strcmp(name, "pwd"))
		return builtin_pwd;
	else if (!strcmp(name, "export"))
		return builtin_export;
	else if (!strcmp(name, "exit"))
		return builtin_exit;
	else if (!strcmp(name, "history"))
		return builtin_history;
	else
		return NULL;
}

#ifdef TESTING
int main(int argc, char *args[])
{
	/* Test builtin_export() */
	char *str = NULL;
	
	if ((str = getenv("a")) != NULL)
		printf("%s\n", str);
	else
		printf("a does not exist!\n");

	if ((str = getenv("b")) != NULL)
		printf("%s\n", str);
	else
		printf("b does not exist!\n");
	
	const char *argv[] = { "export", "a=12", "b=c", 0 };

	builtin_export(argv);
	
	if ((str = getenv("a")) != NULL)
		printf("%s\n", str);
	else
		printf("a does not exist!\n");

	if ((str = getenv("b")) != NULL)
		printf("%s\n", str);
	else
		printf("b does not exist!\n");
	
	/* TEST: builtin_cd() */
	puts(getenv("PWD"));
	builtin_cd(args);
	puts(getenv("PWD"));
	system("ls");

	/* TEST: builtin_pwd() */
	builtin_pwd(args);

	/* TEST: builtin_echo() */
	builtin_echo(args);

	return 0;
}
#endif
