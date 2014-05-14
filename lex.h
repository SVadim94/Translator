#include <iostream>
#include <cstdio>
#include <string>
#include <vector>
#define SEP_TABLE_SIZE sizeof(Parser::td) / sizeof(int)
#define WORDS_TABLE_SIZE sizeof(Parser::tw) / sizeof(int)

using namespace std;

/*Типы лексем.
LEX_LCRO и LEX_RCRO - левая и правая фигурные скобки
LEX_RPAR и LEX_LPAR - левая и правая круглые скобки
*/
enum LexType
{
	LEX_NULL,
	//Служебные слова
	LEX_PROGRAM, LEX_IF,    LEX_ELSE,   LEX_SWITCH, LEX_CASE, LEX_DEFAULT,
	LEX_FOR,     LEX_WHILE, LEX_BREAK,  LEX_GOTO,   LEX_READ, LEX_WRITE,
	LEX_STRUCT,	 LEX_INT,   LEX_STRING, LEX_NOT,    LEX_OR,   LEX_AND,
	LEX_LABEL,   LEX_TRUE,	LEX_FALSE,  LEX_BOOL,
	//Разделители
	LEX_LCRO,  LEX_RCRO, LEX_LPAR, LEX_RPAR, LEX_COLON, LEX_SEMICOLON, LEX_COMMA,
	LEX_EQ,    LEX_NEQ,  LEX_LEQ,  LEX_GEQ,  LEX_LSS,   LEX_GTR,       LEX_PLUS,
	LEX_MINUS, LEX_DIV,  LEX_MULT, LEX_DOT,   LEX_ASSIGN,
	//Остальное
	LEX_IDENT, LEX_NUM, LEX_STR,
	//ПОЛИЗ
	POLIZ_GO, POLIZ_FGO, POLIZ_TGO, POLIZ_LABEL, POLIZ_ADDRESS,
	//Служебный элемент перечисления для подсчета кол-ва
	LEX_LAST
};

//Структура лексемы
struct Lex
{
	LexType lex_type;
	int value; //Позиция в таблице идентификаторов или непосредственно значение
	Lex(LexType lT=LEX_NULL, int val=-1) : lex_type(lT), value(val) {}
	~Lex() {}
	//...
};

//Структура идентификатора
struct ID
{
	string  name;
	bool    defined;
	bool    initialized;
	int     value; /*Для целых чисел - значение, для строк - номер
	в таблице строк, а для логического типа 0 или 1*/
	LexType type;
	ID() : defined(false), initialized(false), value(0), type(LEX_NULL) {}
};

//Таблица идентификаторов
class TID
{
	vector<ID> table;
public:
	TID() {}
	~TID() {}
	const string &get_name   (int i)          const  {return table.at(i).name;}
	      int     push       (string &str)           {ID tmp; tmp.name.assign(str); table.push_back(tmp); return table.size() - 1;}
	      int     find       (const string &) const;
	      void    print      ()               const;
	      bool    is_defined (int i)          const  {return table.at(i).defined;}
	      bool    is_init    (int i)          const  {return table.at(i).initialized;}
	      void    define     (int i)                 {if (table.at(i).defined) throw "Syntax error: redifinition"; else table.at(i).defined = true;}
	      void    initialize (int i, int value)      {table.at(i).initialized = true; table.at(i).value = value;}
	      int     get_value  (int i)                 {return table.at(i).value;}
	      void    set_value  (int i, int value)      {table.at(i).value = value;}
	      void    set_type   (int i, LexType lex)    {table.at(i).type = lex;}
	      LexType get_type   (int i)          const  {return table.at(i).type;} // Все еще нужна?
	//...
};

class TSTR
{
	vector<string> table;
public:
	TSTR() {}
	~TSTR() {}
	int  push  (const string &str) {table.push_back(str); return table.size() - 1;}
	int  find  (const string &)    const;
	void print ()                  const;
};

//Последовательность лексем
class LexList
{
	vector<Lex> list;
public:
	LexList() {};
	~LexList() {};
	Lex    &operator[] (int i)          {return list.at(i);}
	void    push       (const Lex &lex) {list.push_back(lex);}
	int     get_value  (int i)          {return list[i].value;}
	LexType get_lex    (int i)          {return list.at(i).lex_type;}
	void    print      ()               const;
	//...
};


//Контроль баланса #if-#else-#endif
class IfElseStack
{
	struct Stack
	{
		bool   was_else;
		bool   ignore_point;
		Stack *next;
		Stack(): was_else(false), ignore_point(false) {}
	};
	Stack *first;
public:
	IfElseStack(): first(NULL) {}

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
	static const char * TW[];
	static const char * TD[];
	static const LexType tw[];
	static const LexType td[];

	TID     &tid;
	TSTR    &tstr;
	LexList &lex_list;

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
	Parser(TID &tid, TSTR &tstr, LexList &lex_list) :
		tid(tid), tstr(tstr), lex_list(lex_list), mode(START) {}
	~Parser() {}

	void start();

	int findTD(const string &) const;
	int findTW(const string &) const;
	int findPP(const string &) const;

	void check_ident(TID &tid, int i) {if (tid.is_defined(i)) throw "Semanthic error: Redefinition"; else tid.define(i);}
	void print(TID &, TSTR &, LexList &) const;
friend bool is_separator(char);
friend string print_lex(LexType);
};

string print_lex(LexType);
