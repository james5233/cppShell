#include <iostream>
#include <cstdio>
#include <cctype>
#include <string>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <deque>
#include <fstream>
#include <vector>

#define IN_WORD			1
#define IN_STR_DQ		2
#define IN_STR_SQ		4
#define IN_VARNAME	8

#define TT_INCOMPL	1
#define TT_WORD			2
#define TT_SPWORD 	4
#define TT_SQWORD		8
#define TT_DQWORD		16
#define TT_PIPE			32
#define TT_AMP			64

#define MAXARGS 50

/*lexerp_parser.cpp */
typedef struct
{
	int st;
	int flg;
} sttab_t;

#define CHARSZ 256
#define MAXSTATES 10

extern sttab_t tab[MAXSTATES][CHARSZ];

enum lexer_states{ ST_START=0, ST_WORD, ST_ESC_WORD, ST_SPWORD, ST_ESC_SPWORD, ST_SQWORD, ST_DQWORD, ST_END };
enum lexer_flags{ FL_NONE = 0, FL_START_OF_WORD = 1, FL_END_OF_WORD = 2, FL_END_OF_TOKEN = 4, FL_PIPE = 8, FL_AMP = 16, FL_SPACE = 32, FL_REDIRN = 64 };

typedef struct word_t
{
	std::string str;
	int wtype;
	int cont;
	int redirn;
	struct word_t *prev;
	struct word_t *next;
} word_t;
	
typedef struct parser_t
{
	std::string rem;
	std::string str;
	int st;
	int flg;
	int cont;
	int cw_type;
	int cw_st;
	int redirn;
} parser_t;

#define IS_SET_FLG(n, flg)  ((n)&(flg))
#define INV_CHAR 300
#define FALSE 0
#define TRUE 1
#define COMPL 1
#define INCOMPL 0

extern void mk_jmp_table();
extern void pr_word_t(word_t *head);
extern int parse(parser_t *p, word_t *head, word_t **tail);

/* proc_list.cpp */
typedef struct redir_t
{
	std::string s_red[10];
	int fd_red[10];
	int rw[10];
} redir_t;

#define RW_READ 0
#define RW_WRITE 1

typedef struct proc_list_t
{
	int index;
	word_t head;
	redir_t r;
	char *argv[MAXARGS];
	int pid;
	proc_list_t *prev, *next;
} proc_list_t;

#define MAXSTR 200
#define MAXLINE 2000
#define MAXPATH MAXLINE

extern void pr_proc_list(proc_list_t *pl);
extern int split_proc_list(word_t *head, proc_list_t *pl);
extern void process_sqwords(word_t *head);
extern void process_dqwords(word_t *head);
extern void process_spwords(word_t *head);
extern void fix_redirn(word_t *head);
extern void build_redir(word_t *head, redir_t *r);
extern void pr_redir_t(redir_t *r);
extern void consolidate_tokens(word_t *head);
extern void build_argv(word_t *head, char *argv[]);
extern void free_argv(char *argv[]);
extern void pr_argv(char *argv[]);
extern void dealloc_space(proc_list_t *p);

/* dispatch.cpp */
int dispatch_builtin(proc_list_t *p);
int dispatch_procs(proc_list_t *p, int nprocs);

/* builtins.cpp */
typedef int (*bfptr)(const char **);
extern int builtin_export(const char *argv[]);
extern int builtin_cd(const char *argv[]);
extern int builtin_pwd(const char *argv[]);
extern int builtin_echo(const char *argv[]);
extern int builtin_exit(const char *argv[]);
extern int builtin_history(const char *argv[]);
extern bfptr builtin_fptr(const char *name);

/* history.cpp */
#define HISTORY_FILE "shell_history"
#define HISTSZ 1000

typedef struct history_t
{
	int i;
	std::string s;
} history_t;

extern void load_history(const char *filepath);
extern int getline_from_history(int i, std::string &s);
extern void pushline_in_history(std::string &s);
extern int search_in_history(std::string &s, std::string &res);
extern void write_history(const char *filepath);
extern std::string dump_last_nlines(unsigned int n);
extern int substitute_bang(std::string &s, std::string &err);

/* utilities.cpp */
extern char *get_PS1();
extern char *get_PS2();

/* shell.cpp */
extern int exit_code;
extern int exit_flag;
extern char shell_PS1[MAXSTR], shell_PS2[MAXSTR];

