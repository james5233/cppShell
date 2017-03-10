#include "shell.h"

#ifdef DEBUG
	#define D(x) x
#else
	#define D(x)
#endif

char shell_PS1[MAXSTR], shell_PS2[MAXSTR];

void sigint_handler(int sig_num)
{
	if (sig_num == SIGINT) {
		fsync(1);
	}
}

void sigtstp_handler(int sig_num)
{
	if (sig_num == SIGTSTP) {
		printf(" Job Cntl Not Implmented\n");
		fsync(1);
	}
}

int exit_code = 0;
int exit_flag = FALSE;

int main()
{
	std::string S, acc;
	word_t head, *tail;
/*	char PS1[MAXSTR], PS2[MAXSTR], *pc;
	
	if ((pc = getenv("PS1")) != NULL)
		strcpy(PS1, pc);
	else
		strcpy(PS1, "$ ");

	if ((pc = getenv("PS2")) != NULL)
		strcpy(PS2, pc);
	else
		strcpy(PS2, "> ");
*/	
	signal(SIGINT, sigint_handler);
	signal(SIGTSTP, sigtstp_handler);

	char s[MAXLINE];
	parser_t p;
	
	mk_jmp_table();	
	int inp_ct;
	
	/* Create history file and load it */
	std::string hist_flname = getenv("PWD");
	hist_flname += "/";
	hist_flname += HISTORY_FILE;
	load_history(hist_flname.c_str());

	do {
		p.rem = p.str = "";
		p.st = p.flg = p.cont = 0;
		p.cw_st = -1;
		p.cw_type = 0;
		p.redirn = 0;

		head.prev = head.next = NULL;
		tail = &head;

		acc = "";
		int res;
		int ps = 0;
		int bang_retval = 0;

		do {
			ps++;
			s[0] = 0;
			fsync(1);
			printf("%s", (ps == 1) ? get_PS1() : get_PS2());
			inp_ct = scanf("%[^\n]", s);
			acc += s;
			std::string bang_res = acc , bang_err = "";

			if (inp_ct == EOF)
				break;
			else if(inp_ct != 1) {
				acc = "";
			}

			D(printf("Waiting for newline.."));
			int c = getchar();
			
			bang_retval = substitute_bang(bang_res, bang_err);
			if (bang_retval != 0)
			{
				bang_err = "shell: " + bang_err;
				fprintf(stderr, "%s\n", bang_err.c_str());
				break;
			}
			
			acc = bang_res;
			acc += c;

			S = bang_res;
			p.str = S;
			res = parse(&p, &head, &tail);
			if(res == INCOMPL && (p.st == ST_WORD || p.st == ST_SPWORD || (tail != NULL && tail->wtype == TT_PIPE)))
				acc.pop_back(), acc.pop_back();

			D(printf("res = %s\n", res == INCOMPL ? "INCOMPL" : "COMPL"));
		} while (res == INCOMPL);
		
		if (bang_retval != 0)
			continue;

		if (inp_ct == EOF)
			break;

		/* Write line to history */
		acc.pop_back();
		D(std::cout<<"For history:[" << acc <<"]" << std::endl);
		std::string hist_last_str = "";
		int hist_write_res = getline_from_history(-1, hist_last_str);
		if(!(hist_write_res >= 0 && hist_last_str == acc) && acc != "")
			pushline_in_history(acc);

		proc_list_t pl;
		pl.head.prev = pl.head.next = NULL;
		pl.prev = pl.next = NULL;
		int nprocs = split_proc_list(&head, &pl);
		if (nprocs == 0) {
			dealloc_space(&pl);
			continue;
		}

		D(pr_proc_list(&pl));
		
		if (nprocs == 1 && builtin_fptr(pl.next->argv[0]) != NULL)
			dispatch_builtin(&pl);
		else
			dispatch_procs(&pl, nprocs);
		
		dealloc_space(&pl);
		
		if (exit_flag == TRUE) {
			write_history(hist_flname.c_str());
			return exit_code;
		}
	
	} while (inp_ct != EOF);
	
	write_history(hist_flname.c_str());
	printf("exit\n"); /* Bye Message */
	return 0;
}
