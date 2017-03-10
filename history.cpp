#include "shell.h"

#ifdef TESTING
	#define HISTSZ 5
#endif

static std::deque<history_t> vH;
static int hist_last;

void load_history(const char *filepath)
{
	std::string buf;
	hist_last = 0;
	
	/* Because I don't know how to create file if it doesn't exist using ifstream */
	int fd = open(filepath, O_RDWR | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd > 0)
		close(fd);

	std::ifstream hist_fl(filepath, std::ifstream::in);

	while (std::getline(hist_fl, buf))
		pushline_in_history(buf);
}

int getline_from_history(int i, std::string &s)
{
	if (vH.size() == 0)
		return -1;
	
	if (i < 0)
		i += vH.back().i + 1;

	if (i < vH.front().i || i > vH.back().i)
		return -1;
	
	s = vH[i - vH.front().i].s;
	return 0;
}

void pushline_in_history(std::string &s)
{
	while(vH.size() >= HISTSZ)
		vH.pop_front();
	
	history_t temp;
	temp.i = ++hist_last;
	temp.s = s;

	vH.push_back(temp);
}

int search_in_history(std::string &s, std::string &res)
{
	std::deque<history_t>::reverse_iterator it;

	for (it = vH.rbegin(); it != vH.rend(); it++)
		if (it->s.find(s) == 0)
			break;

	if (it == vH.rend())
		return -1;
	
	res = it->s;
	return 1;
}

void write_history(const char *filepath)
{
	std::ofstream hist_fl(filepath, std::ofstream::out);
	std::string buf;

	while (vH.size() > 0) {
		buf = vH.front().s;
		vH.pop_front();
		hist_fl << buf << std::endl;
	}
}

std::string dump_last_nlines(unsigned int n)
{
	if (n == 0)
		n = HISTSZ;

	if (n > vH.size())
		n = vH.size();

	int end = vH.size();
	int st = end - n;
	std::string res = "";

	for (int i = st; i < end; i++) {
		char s[MAXSTR];
		sprintf(s, "%6d  ", vH[i].i);
		res += s + vH[i].s + "\n";
	}

	return res;
}

int substitute_bang(std::string &s, std::string &err)
{
	int in_sqword = 0;
	err = "";
	std::string str = "";

	for(unsigned int i = 0; i < s.size(); i++) {
		char c = s[i];
		if (in_sqword) {
			if (c == '\'') {
				in_sqword = 0;
				continue;
			} else {
				continue;
			}
		} else {
			if (c == '\'') {
				in_sqword = 1;
				continue;
			} else if (c == '\\') {
				if ( i + 1 >= s.size())
					return 0;
				else {
					i++;
					continue;
				}
			} else if (c == '!') {
				if (i + 1 >= s.size())
					return 0;
				else {
					c = s[++i];
					if (c == '!') {
						int res = getline_from_history(-1, str);
						if (res < 0) {
							err = "history is empty!";
							return -1;
						} else {
							s.erase(i-1, 2);
							s.insert(i-1, str);
							i += str.size() - 1;
							continue;
						}
					} else if (c == '-') {
						if (i+1 >= s.size()) {
							err = "!-: event not found";
							return -1;
						}
						char c1 = s[i+1];
						if (isdigit(c1)) {
							int sav = ++i;
							while (i < s.size() && isdigit(s[i]))
								i++;
							std::string num_str = s.substr(sav, i-sav);
							int num;
							sscanf(num_str.c_str(), "%d", &num);
							num = -num;
							int res = getline_from_history(num, str);
							if (res < 0) {
								err = "!-" + num_str + ": event not found";
								return -1;
							} else {
								s.erase(sav-2, 2 + num_str.size());
								s.insert(sav-2, str);
								i--;
								continue;
							}
						} else { 
							err = "!-: event not found";
							return -1;
						}
					} else if (isdigit(c)) {
						int sav = i;
						while (i < s.size() && isdigit(s[i]))
							i++;
						std::string num_str = s.substr(sav, i-sav);
						int num;
						sscanf(num_str.c_str(), "%d", &num);
						int res = getline_from_history(num, str);
						if (res < 0) {
							err = "!" + num_str + ": event not found";
							return -1;
						} else {
							s.erase(sav-1, 1 + num_str.size());
							s.insert(sav-1, str);
							i--;
							continue;
						}
					} else if (isalpha(c)) {
						int sav = i;
						while (i < s.size() && (isalpha(s[i]) || isdigit(s[i])))
							i++;
						std::string srch_str = s.substr(sav, i-sav);
						int res = search_in_history(srch_str, str);
						if (res < 0) {
							err = "!" + srch_str + ": event not found";
							return -1;
						} else {
							s.erase(sav-1, 1 + srch_str.size());
							s.insert(sav-1, str);
							i--;
							continue;
						}
					}
				}
			}
		}
	}
	return 0;
}

							
#ifdef TESTING
int main()
{
	char filepath[] = "./history_file";
	std::string s1 = "ls -l | wc -l"; 
	std::string s2 = "echo Hello bitches";
	std::string s3 = "ps ax";
	std::string s4 = "ls -l | grep -Eo \'[0-9]*\' | wc -l";
	std::string s5 = "echo \"$HOME\"";
	std::string s6 = "whoami";
	std::string s7 = "echo \'Hello\'\"Hello\"Hello \\$Hello";

	load_history(filepath);
	std::cout << dump_last_nlines(0);
	
	/*pushline_in_history(s1);
	std::cout << dump_last_nlines(0);
	
	pushline_in_history(s2);
	std::cout << dump_last_nlines(0);
	
	pushline_in_history(s3);
	std::cout << dump_last_nlines(0);
	
	pushline_in_history(s4);
	std::cout << dump_last_nlines(0);
	
	pushline_in_history(s5);
	std::cout << dump_last_nlines(0);
	
	pushline_in_history(s6);
	std::cout << dump_last_nlines(0);
	
	pushline_in_history(s7);
	std::cout << dump_last_nlines(0);

	std::cout << dump_last_nlines(2);
	*/

	std::string s, err;
	while (std::getline(std::cin, s)) {
		int retval = substitute_bang(s, err);
		std::cout << s << " [RETURN : " << retval << " | " << err << " ]" << std::endl;
	}

	write_history(filepath);
	return 0;
}
#endif
