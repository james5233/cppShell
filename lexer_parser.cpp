#include "shell.h"

#ifdef DEBUG
	#define D(x) x
#else
	#define D(x)
#endif

sttab_t tab[MAXSTATES][CHARSZ];
static const char spaces[] = " \t\n\v\f\r";

/* Make the state to state jump table for the State-Machine */
void mk_jmp_table()
{
	int c, i;

	for (c = 0; c < CHARSZ; c++) {
		tab[ST_WORD  ][c].st = ST_SPWORD;
		tab[ST_SPWORD][c].st = ST_SPWORD;
		tab[ST_SQWORD][c].st = ST_SQWORD;
		tab[ST_DQWORD][c].st = ST_DQWORD;
		tab[ST_START ][c].st = ST_SPWORD;
		
		tab[ST_ESC_WORD  ][c].st = ST_WORD;
		tab[ST_ESC_SPWORD][c].st = ST_SPWORD;
		
		tab[ST_WORD  ][c].flg = FL_NONE;
		tab[ST_SPWORD][c].flg = FL_NONE;
		tab[ST_SQWORD][c].flg = FL_NONE;
		tab[ST_DQWORD][c].flg = FL_NONE;
		tab[ST_START ][c].flg = FL_START_OF_WORD;
		
		tab[ST_ESC_WORD  ][c].flg = FL_NONE;
		tab[ST_ESC_SPWORD][c].flg = FL_NONE;
	}
		
	for (c = 'a'; c <= 'z'; c++) {
		tab[ST_START ][c].st = ST_WORD;
		tab[ST_START ][c].flg = FL_START_OF_WORD;
		
		tab[ST_WORD  ][c].st = ST_WORD;
		tab[ST_SPWORD][c].st = ST_SPWORD;
	}
	
	for (c = 'A'; c <= 'Z'; c++) {
		tab[ST_START ][c].st = ST_WORD;
		tab[ST_START ][c].flg = FL_START_OF_WORD;
		
		tab[ST_WORD  ][c].st = ST_WORD;
		tab[ST_SPWORD][c].st = ST_SPWORD;
	}

	for (c = '0'; c <= '9'; c++) {
		tab[ST_START ][c].st = ST_WORD;
		tab[ST_START ][c].flg = FL_START_OF_WORD;
		
		tab[ST_WORD  ][c].st = ST_WORD;
		tab[ST_SPWORD][c].st = ST_SPWORD;
	}

	tab[ST_START ]['\\'].st = ST_ESC_WORD;
	tab[ST_START ]['\\'].flg = FL_START_OF_WORD;
	
	tab[ST_WORD  ]['\\'].st = ST_ESC_WORD;
	tab[ST_SPWORD]['\\'].st = ST_ESC_SPWORD;
	
	tab[ST_START ]['\''].st = ST_SQWORD;
	tab[ST_START ]['\''].flg = FL_START_OF_WORD;
	
	tab[ST_WORD  ]['\''].st = ST_SQWORD;
	tab[ST_WORD  ]['\''].flg = FL_START_OF_WORD | FL_END_OF_WORD;

	tab[ST_SPWORD]['\''].st = ST_SQWORD;
	tab[ST_SPWORD]['\''].flg = FL_START_OF_WORD | FL_END_OF_WORD;
	
	tab[ST_SQWORD]['\''].st = ST_START;
	tab[ST_SQWORD]['\''].flg = FL_END_OF_WORD;

	tab[ST_START ]['\"'].st = ST_DQWORD;
	tab[ST_START ]['\"'].flg = FL_START_OF_WORD;

	tab[ST_WORD  ]['\"'].st = ST_DQWORD;
	tab[ST_WORD  ]['\"'].flg = FL_START_OF_WORD | FL_END_OF_WORD;

	tab[ST_SPWORD]['\"'].st = ST_DQWORD;
	tab[ST_SPWORD]['\"'].flg = FL_START_OF_WORD | FL_END_OF_WORD;
	
	tab[ST_DQWORD]['\"'].st = ST_START;
	tab[ST_DQWORD]['\"'].flg = FL_END_OF_WORD;

	tab[ST_START ]['|'].st = ST_START;
	tab[ST_START ]['|'].flg = FL_PIPE | FL_END_OF_WORD | FL_END_OF_TOKEN;

	tab[ST_WORD  ]['|'].st = ST_START;
	tab[ST_WORD  ]['|'].flg = FL_PIPE | FL_END_OF_WORD | FL_END_OF_TOKEN;

	tab[ST_SPWORD]['|'].st = ST_START;
	tab[ST_SPWORD]['|'].flg = FL_PIPE | FL_END_OF_WORD | FL_END_OF_TOKEN;

	tab[ST_START ]['&'].st = ST_START;
	tab[ST_START ]['&'].flg = FL_AMP | FL_END_OF_WORD | FL_END_OF_TOKEN;

	tab[ST_WORD  ]['&'].st = ST_START;
	tab[ST_WORD  ]['&'].flg = FL_AMP | FL_END_OF_WORD | FL_END_OF_TOKEN;

	tab[ST_SPWORD]['&'].st = ST_START;
	tab[ST_SPWORD]['&'].flg = FL_AMP | FL_END_OF_WORD | FL_END_OF_TOKEN;
	
	tab[ST_START ]['>'].st = ST_START;
	tab[ST_WORD  ]['>'].st = ST_START;
	tab[ST_SPWORD]['>'].st = ST_START;

	tab[ST_START ]['<'].st = ST_START;
	tab[ST_WORD  ]['<'].st = ST_START;
	tab[ST_SPWORD]['<'].st = ST_START;

	tab[ST_START ]['>'].flg |= FL_REDIRN | FL_END_OF_WORD | FL_END_OF_TOKEN;
	tab[ST_WORD  ]['>'].flg |= FL_REDIRN | FL_END_OF_WORD | FL_END_OF_TOKEN;
	tab[ST_SPWORD]['>'].flg |= FL_REDIRN | FL_END_OF_WORD | FL_END_OF_TOKEN;
	
	tab[ST_START ]['<'].flg |= FL_REDIRN | FL_END_OF_WORD | FL_END_OF_TOKEN;
	tab[ST_WORD  ]['<'].flg |= FL_REDIRN | FL_END_OF_WORD | FL_END_OF_TOKEN;
	tab[ST_SPWORD]['<'].flg |= FL_REDIRN | FL_END_OF_WORD | FL_END_OF_TOKEN;
	
	int len = sizeof(spaces);
	for (i = 0; i < len; i++) {
		int c = spaces[i];
		tab[ST_START ][c].st = ST_START;
		tab[ST_START ][c].flg = FL_SPACE;

		tab[ST_WORD  ][c].st = ST_START;
		tab[ST_WORD  ][c].flg = FL_END_OF_WORD | FL_END_OF_TOKEN | FL_SPACE;

		tab[ST_SPWORD][c].st = ST_START;
		tab[ST_SPWORD][c].flg = FL_END_OF_WORD | FL_END_OF_TOKEN | FL_SPACE;
	}
}


/* print a word */
void pr_word_t(word_t *head)
{
	word_t *p = head->next;

	while (p) {
		printf(" [%s, %d, %d, %d]-->", p->str.c_str(), p->wtype, p->cont, p->redirn);
		p = p->next;
	}
	printf(" NULL\n");
}

/* Parse input and break into words */
int parse(parser_t *p, word_t *head, word_t **tail)
{
	p->str = p->rem + p->str;

	int i = p->rem.size(), res = COMPL;
	int st = p->st, flg = p->flg, cont = p->cont, redirn = p->redirn;
	int c;
	int cw_st = p->cw_st, cw_len = -1, cw_type = p->cw_type;
	word_t *cur = *tail;

	while (1) {
		while ((c = p->str[i]) != '\0')
		{
			int n_st = tab[st][c].st;
			int n_flg = tab[st][c].flg;

			if (n_flg == FL_NONE) {
				i++;
				flg = n_flg;
				st = n_st;
				continue;
			}

			cw_len = (cw_st >= 0) ? i - cw_st + 1 : -1;

			if (IS_SET_FLG(n_flg, FL_END_OF_WORD | FL_END_OF_TOKEN)) {
				if (st == ST_WORD)
					cw_type = TT_WORD;
				else if (st == ST_SPWORD)
					cw_type = TT_SPWORD;
				else if (st == ST_SQWORD)
					cw_type = TT_SQWORD;
				else if (st == ST_DQWORD)
					cw_type = TT_DQWORD;

				if (IS_SET_FLG(n_flg, FL_REDIRN))
					redirn = TRUE;

				if (cw_len > 1) {
					cur->next = new word_t;
					cur->next->prev = cur;
					cur = cur->next;
					cur->next = NULL;
					cur->redirn = redirn;

					if((cw_type == TT_WORD || cw_type == TT_SPWORD) && redirn != TRUE)
						cw_len -= 1;
					redirn = FALSE;
					cur->str = p->str.substr(cw_st, cw_len);
					cur->wtype = cw_type;
					cur->cont = cont;
					if (IS_SET_FLG(n_flg, FL_END_OF_TOKEN))
						cont = FALSE;
					else
						cont = TRUE;
				}

				cw_st = -1;
			}

			if (IS_SET_FLG(n_flg, FL_PIPE)) {
				cur->next = new word_t;
				cur->next->prev = cur;
				cur = cur->next;
				cur->next = NULL;
				redirn = FALSE;
				cur->redirn = redirn;

				cur->str = "|";
				cur->wtype = TT_PIPE;
				cur->cont = FALSE;

				cw_st = -1;
			} else if (IS_SET_FLG(n_flg, FL_AMP)) {
				cur->next = new word_t;
				cur->next->prev = cur;
				cur = cur->next;
				cur->next = NULL;
				redirn = FALSE;
				cur->redirn = redirn;

				if ( i > 0 && 
				(unsigned)i < p->str.size()-1 &&
				(p->str[i-1] == '>' || p->str[i-1] == '<' )&& 
				isdigit(p->str[i+1]) ) {
					cur->str = p->str.substr(i, 2);
					cur->wtype = TT_SPWORD;
					i++;
					n_st = ST_START;
					n_flg = FL_NONE;
				} else { 
					cur->str = "&";
					cur->wtype = TT_AMP;
				}
				cur->cont = FALSE;

				cw_st = -1;
			} else if (redirn == TRUE) {
				cur->next = new word_t;
				cur->next->prev = cur;
				cur = cur->next;
				cur->next = NULL;

				cur->str = c;
				cur->wtype = TT_WORD;
				cur->cont = FALSE;
				cur->redirn = TRUE;
				redirn = FALSE;

				cw_st = -1;
			}

			if (IS_SET_FLG(n_flg, FL_SPACE)) {
				if (IS_SET_FLG(flg, FL_END_OF_WORD))
					cont = FALSE;
			}

			if (IS_SET_FLG(n_flg, FL_START_OF_WORD))
				cw_st = i;

			i++;
			st = n_st;
			flg = n_flg;
				
			D(pr_word_t(head));

		}

		if (st == ST_WORD || st == ST_SPWORD)
			p->str += ' ';
		else if (st == ST_ESC_WORD || st == ST_ESC_SPWORD) {
			p->rem = p->str;
			p->rem.pop_back();
			p->str = "";
			p->st = (st == ST_ESC_WORD)? ST_WORD : ST_SPWORD;
			p->flg = flg;
			p->cont = cont;
			p->cw_st = cw_st;
			p->cw_type = cw_type;
			p->redirn = redirn;
			res = INCOMPL;
			*tail = cur;
			break;
		} else if (st == ST_SQWORD || st == ST_DQWORD) {
			p->rem = p->str + "\n";
			p->str = "";
			p->st = st;
			p->flg = flg;
			p->cont = cont;
			p->cw_st = cw_st;
			p->cw_type = cw_type;
			p->redirn = redirn;
			res = INCOMPL;
			*tail = cur;
			break;
		} else if (cur->wtype == TT_PIPE) {
			p->rem = p->str;
			p->str = "";
			p->st = st;
			p->flg = flg;
			p->cont = cont;
			p->cw_st = cw_st;
			p->cw_type = cw_type;
			p->redirn = redirn;
			res = INCOMPL;
			*tail = cur;
			break;
		} else {
			res = COMPL;
			*tail = cur;
			break;
		}
	}
	return res;
}

#ifdef TESTING
int main()
{
	std::string S, acc;
	word_t head, *tail;

	char s[2000];
	parser_t p;
	p.rem = p.str = "";
	p.st = p.flg = p.cont = 0;
	p.cw_st = -1;
	p.cw_type = 0;
	p.redirn = 0;

	mk_jmp_table();	
	head.prev = head.next = NULL;
	tail = &head;
	
	acc = "";
	int res;
	do {
		scanf("%[^\n]", s);
		int c = getchar();
		acc += s;
		acc += c;

		S = s;
		p.str = S;
		res = parse(&p, &head, &tail);
	} while (res == INCOMPL);
	
	acc.pop_back();
	std::cout<<"For history:[" << acc <<"]" << std::endl; 
	
	proc_list_t pl;
	int nprocs = split_proc_list(&head, &pl);
	pr_proc_list(&pl);

	return 0;
}
#endif
