#ifndef __tables__
#include "tables.h"
#endif

#include "lex.h"
#include "syn.h"

int main()
{
	try
	{
		TID tid;
		TSTR tstr;
		LexList lex_list;
		TSTRUCT tstruct;
		POLIZ poliz;

		Parser pars(tid, tstr, lex_list);
		Analyzer syn_analyzer(tid, tstr, lex_list, tstruct, poliz);

		pars.start();

		syn_analyzer.start();

		pars.print(tid, tstr, lex_list);
		tstruct.print();
		poliz.print();
	}
	catch(const char * str) {cout << str << endl;}
//	catch(...) {cout << "Unexpected error!";}
	catch(...) {cout << "I dunno, lol ¯\\(°_o)/¯";}
}
