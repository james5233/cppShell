#include "shell.h"

/* Print the Process List i.e. all processes in the pipeline */
void pr_proc_list(proc_list_t *pl)
{
	proc_list_t *cur = pl->next;

	while(cur != NULL)
	{
		printf("{ %d }:", cur->index);
		pr_word_t(&(cur->head));
		pr_redir_t(&(cur->r));
		pr_argv(cur->argv);
		cur = cur->next;
	}
}

/* Print the IO redirection table */
void pr_redir_t(redir_t *r)
{
	for(int i = 0; i < 10; i++)
	{
		if(r->fd_red[i] < 0)
		{
			if(r->s_red[i] != "")
				printf("\t[%d]: %s : %s\n", i, r->s_red[i].c_str(), r->rw[i] ? "WRITE" : "READ");
		}
		else if(r->fd_red[i] != i)
		{
			if(r->s_red[i] != "")
				printf("\t[%d]: Error\n", i);
			else
				printf("\t[%d]: %d : %s\n", i, r->fd_red[i], r->rw[i] ? "WRITE" : "READ");
		}
	}
}

/* Print the argument list */
void pr_argv(char *argv[])
{
	int i = 0;
	while(argv[i] != 0)
	{
		printf("<arg: %d><%s>\n", i, argv[i]);
		i++;
	}
}

/* split the words list into a list of processes */
int split_proc_list(word_t *head, proc_list_t *pl)
{
	pl->index = 0;
	pl->prev = pl->next = NULL;

	word_t *cur = head->next;
	word_t *last = head->next;

	while (cur != NULL)
	{
		if(cur->wtype != TT_PIPE)
			cur = cur->next;
		else
		{
			word_t *sav = cur;
			cur->prev->next = NULL;
			pl->next = new proc_list_t;
			pl->next->prev = pl;
			pl = pl->next;
			pl->next = NULL;
			pl->index = pl->prev->index + 1;
			pl->head.next = last;
			last->prev = &(pl->head);
			pl->head.prev = NULL;
			last = cur->next;
			cur = cur->next;
			delete sav;
			fix_redirn(&(pl->head));
			process_sqwords(&(pl->head));
			process_dqwords(&(pl->head));
			process_spwords(&(pl->head));
			build_redir(&(pl->head), &(pl->r));
			consolidate_tokens(&(pl->head));
			build_argv(&(pl->head), pl->argv);
		}
	}
	
	pl->next = new proc_list_t;
	pl->next->prev = pl;
	pl = pl->next;
	pl->next = NULL;
	pl->index = pl->prev->index + 1;
	pl->head.next = last;
	if (last)
		last->prev = &(pl->head);
	pl->head.prev = NULL;

	fix_redirn(&(pl->head));
	process_sqwords(&(pl->head));
	process_dqwords(&(pl->head));
	process_spwords(&(pl->head));
	build_redir(&(pl->head), &(pl->r));
	consolidate_tokens(&(pl->head));
	build_argv(&(pl->head), pl->argv);

	return pl->index;
}

/* Second stage processing of single quoted words */
void process_sqwords(word_t *head)
{
	word_t *cur = head->next;

	while(cur != NULL)
	{
		if(cur->wtype == TT_SQWORD)
		{
			cur->str.erase(0, 1);
			cur->str.pop_back();
		}

		cur = cur->next;
	}
}

/* Second stage processing of double quoted words */
void process_dqwords(word_t *head)
{
	word_t *cur = head->next;
	char c;

	while(cur != NULL)
	{
		if(cur->wtype == TT_DQWORD)
		{
			cur->str.erase(0, 1);
			cur->str.pop_back();

			for(unsigned int i = 0; i < cur->str.size();)
			{
				if(cur->str[i] == '$')
				{
					unsigned int psav = i;
					std::string varname = "";
					i++;

					if (i < cur->str.size() && cur->str[i] == '$')
					{
						int pid = getpid();
						char str_pid[MAXSTR];
						sprintf(str_pid, "%d", pid);
						varname = str_pid;
						cur->str.erase(psav, 2);
						cur->str.insert(psav, varname);
					}
					else
					{
						while(i < cur->str.size() && ( isalpha((c = cur->str[i]))  || isdigit(c) || c == '_') )
						{
							varname += c;
							i++;
						}

						if(varname.size() > 0)
						{
							char *str = getenv(varname.c_str());
							std::string varval = (str == NULL) ? "" : str;
							cur->str.erase(psav, 1 + varname.size());
							cur->str.insert(psav, varval);
						}
					}
				}
				else 
					i++;
			}
		}

		cur = cur->next;
	}
}

/* Second stage processing of unquoted words with punctuation */
void process_spwords(word_t *head)
{
	word_t *cur = head->next;
	char c;

	while(cur != NULL)
	{
		if(cur->wtype == TT_WORD || cur->wtype == TT_SPWORD)
		{

			for(unsigned int i = 0; i < cur->str.size();)
			{
				if(cur->str[i] == '\\')
				{
					std::string c = cur->str.substr(i+1, 1);
					cur->str.erase(i, 2);
					cur->str.insert(i, c);
					i++;
				}
				else if(cur->str[i] == '$')
				{
					unsigned int psav = i;
					std::string varname = "";
					i++;
					
					if (i < cur->str.size() && cur->str[i] == '$')
					{
						int pid = getpid();
						char str_pid[MAXSTR];
						sprintf(str_pid, "%d", pid);
						varname = str_pid;
						cur->str.erase(psav, 2);
						cur->str.insert(psav, varname);
					}
					else
					{
						while(i < cur->str.size() && ( isalpha((c = cur->str[i]))  || isdigit(c) || c == '_') )
						{
							varname += c;
							i++;
						}

						if(varname.size() > 0)
						{
							char *str = getenv(varname.c_str());
							std::string varval = (str == NULL) ? "" : str;
							cur->str.erase(psav, 1 + varname.size());
							cur->str.insert(psav, varval);
						}
					}
				}
				else 
					i++;
			}
		}

		cur = cur->next;
	}
}

/* Fix the redirection word : identify and separate */
void fix_redirn(word_t *head)
{
	word_t *cur = head->next;

	while(cur != NULL)
	{
		if(cur->redirn == 1)
		{
			if(cur->str.size() == 1)
				;
			else if(cur->str.size() > 2 || !isdigit(cur->str[0]))
			{
				word_t *nw = new word_t;
				nw->next = cur;
				nw->prev = cur->prev;
				nw->wtype = cur->wtype;
				nw->redirn = 0;
				nw->cont = cur->cont;
				nw->str = cur->str.substr(0, cur->str.size() - 1);
				cur->str.erase(0, cur->str.size()-1);
				cur->prev = nw;
				cur->cont = 0;
				nw->prev->next = nw;
			}
		}

		cur = cur->next;
	}
}

/* Build the IO redirection table */
void build_redir(word_t *head, redir_t *r)
{
	
	word_t *cur = head->next;
	int target;
	
	for (int i = 0; i < 10; i++)
	{
		r->fd_red[i] = i;
		r->s_red[i] = "";
	}
	
	while(cur != NULL)
	{
		if(cur->redirn == 1)
		{
			if(cur->str.size() == 1)
			{
				if(cur->str[0] == '>')
					target = 1;
				else
					target = 0;
			}
			else
				target = cur->str[0] - '0';


			if(cur->next->str.size() == 2 && cur->next->str[0] == '&' && isdigit(cur->next->str[1]))
			{
				int fd = cur->next->str[1] - '0';
				if (r->fd_red[fd] >= 0)
				{
					r->fd_red[target] = fd;
					r->s_red[target] = "";
					if (cur->str[0] == '<')
						r->rw[target] = RW_READ;
					else
						r->rw[target] = RW_WRITE;
				}
				else
				{
					r->fd_red[target] = -1;
					r->s_red[target] = r->s_red[fd];
					if (cur->str[0] == '<')
						r->rw[target] = RW_READ;
					else
						r->rw[target] = RW_WRITE;

				}
			}
			else
			{
				r->fd_red[target] = -1;
				r->s_red[target] = cur->next->str;
				if (cur->str[0] == '<')
					r->rw[target] = RW_READ;
				else
					r->rw[target] = RW_WRITE;
			}

			cur->prev->next = cur->next->next;
			if( cur->next->next)
			{
				cur->next->next->prev = cur->prev;
				cur->next->next->cont = 0;
			}
			word_t *temp = cur->next->next;
			delete cur->next;
			delete cur;
			cur = temp;
		}
		else
			cur = cur->next;
	}
}

/* consolidate continued words into one word */
void consolidate_tokens(word_t *head)
{
	word_t *cur = head->next, *last = head, *temp;
	
	while(cur != NULL)
	{
		if(cur->cont == 0)
		{
			last = cur;
			cur = cur->next;
		}
		else
		{
			last->str += cur->str;
			last->next = cur->next;
			if(cur->next)
				cur->next->prev = last;
			temp = cur->next;
			delete cur;
			cur = temp;
		}
	}
}

/* Build the argument list */
void build_argv(word_t *head, char *argv[])
{	
	int i = 0;
	word_t *cur = head->next;

	while(i < MAXARGS -1 && cur != NULL)
	{
		int sz = cur->str.size();
		argv[i] = (char *)malloc(sz+1);
		strcpy(argv[i], cur->str.c_str());
		i++;
		cur = cur->next;
	}

	argv[i] = 0;
}

/* Free the space allocated for storing the argument list */
void free_argv(char *argv[])
{
	int i = 0;
	while(argv[i] != 0)
		free(argv[i++]);
}

/* Free the space allocated for storing the process list */
void dealloc_space(proc_list_t *p)
{
	proc_list_t *cur = p->next;

	while(cur != NULL)
	{
		proc_list_t *sav_p = cur->next;

		free_argv(cur->argv);
		word_t *wp = cur->head.next;

		while(wp != NULL)
		{
			word_t *sav_w = wp->next;
			delete wp;
			wp = sav_w;
		}

		delete cur;
		cur = sav_p;
	}
}
