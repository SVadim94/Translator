#include <vector>
#include <iostream>

#ifndef __tables__
#include "tables.h"
#endif

class SWITCH_STACK
{
	vector<int> stack;
public:
	void push (int i) {stack.push_back(i);}
	void pop  ()      {stack.pop_back();}
	int  last ()      {return stack.back();}
};

class Executer
{
	TID     &tid;
	TSTR    &tstr;
	TSTRUCT &tstruct;
	POLIZ   &poliz;

	SWITCH_STACK switch_stack;
	vector<Lex>  stack;

public:
    Executer(TID &tid, TSTR &tstr, TSTRUCT &tstruct, POLIZ &poliz) :
    	tid(tid), tstr(tstr), tstruct(tstruct), poliz(poliz) {}

	void start ();
	void push  (Lex);
	void clear ();
	Lex  pop   ();
};
