#include <string>
#include <vector>

/*Типы лексем.
LEX_LCRO и LEX_RCRO - левая и правая фигурные скобки
LEX_RPAR и LEX_LPAR - левая и правая круглые скобки
*/
enum lex_type
{
	//Служебные слова
	LEX_PROGRAM, LEX_IF, LEX_ELSE, LEX_SWITCH, LEX_CASE, LEX_DEFAULT,
	LEX_FOR, LEX_WHILE, LEX_BREAK, LEX_GOTO, LEX_READ, LEX_WRITE,
	LEX_STRUCT,	LEX_INT, LEX_STRING, LEX_BOOL,
	//Разделители
	LEX_LCRO, LEX_RCRO, LEX_LPAR, LEX_RPAR, LEX_COLON, LEX_SEMICOLON,
	LEX_COMMA, LEX_EQ, LEX_NEQ, LEX_LEQ, LEX_GEQ, LEX_LSS, LEX_GTR,
	LEX_PLUS, LEX_MINUS, LEX_DIV, LEX_MOD, LEX_MULT,
	//Остальное
	LEX_IDENT, LEX_NUM, LEX_STR, LEX_LOG
}

//Структура лексемы
class Lex
{
	lex_type lType;
	int pos; //Позиция в таблице идентификаторов или непосредственно значение
public:
	Lex(lex_type lT, int val) : lType(lT), pos(val) {}
	~Lex() {}
	//...
};

//Таблица идентификаторов
class TID
{
	vector<string> table;
	int top;
public:
	TID() : top(-1) {}
	~TID() {}
	void add(char c) {table[top].push_back(c);}
	void push(char c) {table[++top].push_back(c);}
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
	static const lex_type tw[];
	static const lex_type td[];
public:
	Parser() {}
	~Parser() {}
	//...
};

lex_type Parser::tw[]={
	LEX_PROGRAM, LEX_IF, LEX_ELSE, LEX_SWITCH, LEX_CASE, LEX_DEFAULT,
	LEX_FOR, LEX_WHILE, LEX_BREAK, LEX_GOTO, LEX_READ, LEX_WRITE,
	LEX_STRUCT,	LEX_INT, LEX_STRING, LEX_BOOL
}

lex_type Parser::tw[]={
	LEX_LCRO, LEX_RCRO, LEX_LPAR, LEX_RPAR, LEX_COLON, LEX_SEMICOLON,
	LEX_COMMA, LEX_EQ, LEX_NEQ, LEX_LEQ, LEX_GEQ, LEX_LSS, LEX_GTR,
	LEX_PLUS, LEX_MINUS, LEX_DIV, LEX_MOD, LEX_MULT
}

const char * Parser::TW[]={
	"program", "if", "else", "switch", "case", "default",
	"for", "while", "break", "goto", "read", "write",
	"struct", "int", "string", "bool"
}

const char * Parser::TD[]={
	"{", "}", "(", ")", ":", ";",
	",", "=", "!=", "<=", ">=", "<", ">",
	"+", "-", "/", "%", "*"
}