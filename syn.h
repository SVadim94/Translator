#ifndef SEP_TABLE_SIZE
#include "lex.h"
#endif

class Poliz
{
	vector<LexType> poliz;
public:
	LexType operator[](int i) {return poliz.at(i);}
	//...
};

class Analyzer
{
	Poliz poliz;
	LexType lex;
	Parser *pars;

	void PROGRAM();
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
	Analyzer(): pars(NULL) {}
	void start(Parser &);
};
