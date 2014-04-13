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
	LEX_PROGRAM, LEX_IF, LEX_ELSE, LEX_SWITCH, LEX_CASE, LEX_DEFAULT,
	LEX_FOR, LEX_WHILE, LEX_BREAK, LEX_GOTO, LEX_READ, LEX_WRITE,
	LEX_STRUCT,	LEX_INT, LEX_STRING, LEX_BOOL,
	//Разделители
	LEX_LCRO, LEX_RCRO, LEX_LPAR, LEX_RPAR, LEX_COLON, LEX_SEMICOLON,
	LEX_COMMA, LEX_EQ, LEX_NEQ, LEX_LEQ, LEX_GEQ, LEX_LSS, LEX_GTR,
	LEX_PLUS, LEX_MINUS, LEX_DIV, LEX_MOD, LEX_MULT, LEX_ASSIGN,
	//Остальное
	LEX_IDENT, LEX_NUM, LEX_STR, LEX_LOG
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

//Таблица идентификаторов
class TID
{
	vector<string> table;
	int top;
public:
	TID() : top(0) {}
	~TID() {}
	int push(string &str) {table.push_back(str); return top++;}
	//...
};

//Последовательность лексем
class LexList
{
	vector<Lex> list;
public:
	LexList() {};
	~LexList() {};
	void push(const Lex &lex) {list.push_back(lex);}
	//...
};

//Класс , ответственный за выделение лексем и составление таблиц
//Содержит также таблицы служебных слов и разделителей
class Parser
{
	static const char * TW[];
	static const char * TD[];
	static const LexType tw[];
	static const LexType td[];
	LexList lex_list;
	TID tid;
	Lex lex;
	string ident;
	char c;
	enum MODE{
		START,
		IDENT,
		NUM,
		SEPARATOR,
		STOP
		//...
	} mode;
public:
	Parser(): mode(START) {}
	~Parser() {}
	int start();
	int findTD(const string &str) const;
	int findTW(const string &str) const;
	//...
friend bool is_separator(char c);
};
