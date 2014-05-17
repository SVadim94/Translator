#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <vector>

#ifndef __tables__
#include "tables.h"
#endif

using namespace std;

//Контроль баланса #if-#else-#endif
class IfElseStack
{
	struct Stack
	{
		bool   was_else;
		bool   ignore_point;
		Stack *next;

		Stack() : was_else(false), ignore_point(false) {}
	};

	Stack *first;
public:
	IfElseStack() : first(NULL) {}

	~IfElseStack()
	{
		Stack *tmp = first;
		while (tmp != NULL)
		{
			tmp = first->next;
			delete first;
			first = tmp;
		}
	}

	bool isEmpty() {return first == NULL;}

	void push()
	{
		Stack *tmp = new Stack;
		tmp->next  = first;
		first      = tmp;
	}

	void pop()
	{
		Stack *tmp;
		if (isEmpty())
			throw "#if-#endif disbalance!";

		tmp = first->next;
		delete first;
		first = tmp;
	}

	void check()
	{
		if (isEmpty())
			throw "Unexpected error[7]";

		if (first->was_else)
			throw "#if-#else disbalance!";

		first->was_else = true;
	}

	bool stopIgnore()
	{
		if (isEmpty())
			throw "Undefined error!";

		return first->ignore_point;
	}

	void ignore()
	{
		if (isEmpty())
			throw "Undefined error! [2]";

		first->ignore_point = true;
	}
};

//Класс , ответственный за выделение лексем и составление таблиц
//Содержит также таблицы служебных слов и разделителей
class Parser
{
	static const char *  TW[];
	static const char *  TD[];
	static const LexType tw[];
	static const LexType td[];

	ifstream &fin;
	TID      &tid;
	TSTR     &tstr;
	LexList  &lex_list;

	enum MODE{
		START,
		IDENT,
		NUM,
		SEPARATOR,
	} mode;

	struct Define
	{
		string from;
		int to;
	};

	vector<Define> pre_proc_list;

public:
	Parser(ifstream &fin, TID &tid, TSTR &tstr, LexList &lex_list) :
		fin(fin), tid(tid), tstr(tstr), lex_list(lex_list), mode(START) {}
	~Parser() {}

	void start();

	int findTD(const string &) const;
	int findTW(const string &) const;
	int findPP(const string &) const;

	void check_ident(TID &tid, int i) {if (tid.is_defined(i)) throw "Semanthic error: Redefinition"; else tid.define(i);}
	void print(ofstream &, TID &, TSTR &, LexList &) const;

	friend bool is_separator(char);
	friend string print_lex(LexType);
};

inline bool is_alpha(char c);
inline bool is_space(char c);

bool is_separator(char c);
void readString(ifstream &fin, string &str);
void readComment(ifstream &fin);
