//Оно мне надо?!
#ifndef SEP_TABLE_SIZE
#include "lex.h"
#endif

class POLIZ
{
	vector<Lex> poliz;
	vector<unsigned int> labels;
public:
	unsigned int  push       (Lex lex)               {poliz.push_back(lex); return poliz.size() - 1;}
	unsigned int  label_pop  ()                      {unsigned int tmp = labels.back(); labels.pop_back(); return tmp;}
	unsigned int  current    ()                      {return poliz.size();} // Сюда пока писать нельзя!
	void          label_push (unsigned int lb_index) {labels.push_back(lb_index);}
	void          print      () const;
	Lex          &operator[] (int i)                 {return poliz.at(i);}
	Lex           pop        ()                      {Lex tmp = poliz.back(); poliz.pop_back(); return tmp;}
};

class TSTRUCT
{
	struct FIELD
	{
		string name;
		LexType type;
		FIELD() {}
		FIELD(const string &str, LexType lex) : name(str), type(lex) {}
	};

	struct STRUCT
	{
		string name;
		vector<FIELD> fields;
		STRUCT() {}
		explicit STRUCT(const string &str) : name(str) {}
	};

	vector<STRUCT> table;
public:
	void   push_field  (const string &str, LexType lex) {FIELD tmp(str, lex); table.back().fields.push_back(tmp);}
	int    push        (const string &str)              {STRUCT tmp(str); table.push_back(tmp); return table.size() - 1;}
	STRUCT &operator[] (int i)                          {return table.at(i);}

	int  find  (int, const string &) const;
	void print ()                    const;
	//...
};

/*class SWITCH_STACK
{
	vector<int> stack;
public:
	void push (int i) {stack.push_back(i);}
	void pop  ()      {stack.pop_back();}
	int  last ()      {return stack.back();}
};
*/

class BREAK_STACK
{
	vector<unsigned int> stack;
	bool break_allowed;
public:
	allow()
};

class Analyzer
{
	Lex      lexeme;
	LexType  lex;
	LexType  block_type; //Уродливый, Б-гомързкий костыль!
	int      struct_block_type; //Сие, кстати, тоже!
	int      pos;

	TID      &tid;
	TSTR     &tstr;
	LexList  &lex_list;
	TSTRUCT  &tstruct;
	POLIZ    &poliz;

	vector<LexType> expression_type_stack;

	//Метод рекурсивного спуска
	void PROGRAM();
	void STRUCTURES();
	void STRUCTURE();
	void SDESCRIPTION();
	void DESCRIPTIONS();
	void DESCRIPTION();
	void TYPE();
	void VARIABLE();
	void CONSTANT();
	void STRING();
	void BOOL();
	void INTEGER();
	void OPERATORS();
	void OPERATOR();
	void EXPRESSION();
	void E1();
	void E2();
	void E3();
	void E4();
	void E5();
	void E6();
	void E7();
	void E8();
	//...
public:
	Analyzer(TID &tid, TSTR &tstr, LexList &lex_list, TSTRUCT &tstruct, POLIZ &poliz) :
		pos(-1), tid(tid), tstr(tstr), lex_list(lex_list), tstruct(tstruct), poliz(poliz) {}
	void start();
};

inline void check_assign(LexType, LexType);
inline void check_logic(LexType, LexType);
inline void check_comparison(LexType, LexType);
inline void check_arithmetic(LexType, LexType, LexType);
inline void check_not(LexType);
inline void check_minus(LexType);
inline bool is_cmp(LexType);
