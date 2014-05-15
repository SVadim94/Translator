#include <vector>
#include <string>

#ifndef __tables__
#include "tables.h"
#endif

class BREAK_STACK
{
	vector<uint> stack;
	bool break_allowed;
public:
    BREAK_STACK () : break_allowed(false) {}
	// Кладет -1 ~ 0xFFFF... как разделитель кадра стека текущего цикла/switch'a
	void allow  ()        {break_allowed = true; stack.push_back(-1);}
	// Позиция незаполненного break'a в ПОЛИЗЕ
	void push   (uint i)  {stack.push_back(i);}
	// Запрет break'a в условиях while / for / switch
	void forbid ()        {break_allowed = false;}

	int pop()
	{
		uint tmp = stack.back();
		stack.pop_back();
		if (stack.size() == 0)
			break_allowed = false;
		return tmp;
	}

	bool is_allowed() {return break_allowed;}
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

	BREAK_STACK break_stack;

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
