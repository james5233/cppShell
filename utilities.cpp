#include "shell.h"

char *get_PS1()
{
	char *pc;
	if ((pc = getenv("PS1")) != NULL)
		strcpy(shell_PS1, pc);
	else
		strcpy(shell_PS1, "$ ");
	
	return shell_PS1;
}

char *get_PS2()
{
	char *pc;
	if ((pc = getenv("PS2")) != NULL)
		strcpy(shell_PS2, pc);
	else
		strcpy(shell_PS2, "> ");
	
	return shell_PS2;
}
