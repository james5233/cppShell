.DEFAULT: release

release: shell.cpp shell.h proc_list.cpp lexer_parser.cpp dispatcher.cpp builtins.cpp history.cpp utilities.cpp Makefile
	g++ -std=c++0x -o shell shell.cpp proc_list.cpp lexer_parser.cpp dispatcher.cpp builtins.cpp history.cpp utilities.cpp

debug: shell.cpp shell.h proc_list.cpp lexer_parser.cpp dispatcher.cpp builtins.cpp history.cpp utilities.cpp Makefile
	g++ -Wall -Wextra -std=c++0x -DDEBUG -g3 -o shell shell.cpp proc_list.cpp lexer_parser.cpp dispatcher.cpp builtins.cpp history.cpp utilities.cpp

clean: shell
	rm -f shell shell
