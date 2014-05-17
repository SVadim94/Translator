#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#define __tables__ 1
#define uint unsigned int

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
	LEX_MINUS, LEX_UMIN, LEX_DIV,  LEX_MULT, LEX_DOT,   LEX_ASSIGN,
	//Остальное
	LEX_IDENT, LEX_NUM, LEX_STR,
	//ПОЛИЗ
	POLIZ_GO, POLIZ_FGO, POLIZ_LABEL, POLIZ_ADDRESS,
	//Служебный элемент перечисления для подсчета кол-ва
	LEX_LAST
};

//Структура лексемы
struct Lex
{
	LexType lex_type;
	int     value; //Позиция в таблице идентификаторов или непосредственно значение

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

	string &get_name   (int i)                 {return table.at(i).name;}
	int     push       (string &str)           {ID tmp; tmp.name.assign(str); table.push_back(tmp); return table.size() - 1;}
	int     find       (const string &) const;
	void    print      (ofstream &)     const;
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

//Таблица строк
class TSTR
{
	vector<string> table;
public:
	TSTR() {}
	~TSTR() {}

	int    push        (const string &str) {table.push_back(str); return table.size() - 1;}
	int    find        (const string &)    const;
	void   pop         ()                  {table.pop_back();}
	void   print       (ofstream &)        const;
	string &operator[] (int i)             {return table.at(i);}
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
	void    print      (ofstream &)     const;
	//...
};

class POLIZ
{
	vector<Lex> table;
	vector<uint> labels;
public:
	uint  push        (Lex lex)        {table.push_back(lex); return table.size() - 1;}
	uint  label_pop   ()               {uint tmp = labels.back(); labels.pop_back(); return tmp;}
	uint  current     ()               {return table.size();} // Сюда пока писать нельзя!
	uint  size        ()               {return table.size();}
	void  label_push  (uint lb_index)  {labels.push_back(lb_index);}
	void  print       (ofstream &)     const;
	Lex   &operator[] (int i)          {return table.at(i);}
	Lex   pop         ()               {Lex tmp = table.back(); table.pop_back(); return tmp;}
};

struct FIELD
{
	string  name;
	LexType type;

	FIELD() {}
	FIELD(const string &str, LexType lex) : name(str), type(lex) {}
};

struct STRUCT
{
	string name;
	vector<FIELD> fields;

	explicit STRUCT(const string &str) : name(str) {}
			 STRUCT() {}
};

class TSTRUCT
{
	vector<STRUCT> table;
public:
	void   push_field  (const string &str, LexType lex) {FIELD tmp(str, lex); table.back().fields.push_back(tmp);}
	int    push        (const string &str)              {STRUCT tmp(str); table.push_back(tmp); return table.size() - 1;}
	STRUCT &operator[] (int i)                          {return table.at(i);}

	int  find  (int, const string &) const;
	void print (ofstream &)          const;
	//...
};

string print_lex(LexType);
