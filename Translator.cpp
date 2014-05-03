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

		Parser pars;
		Analyzer syn_analyzer(tid, tstr, lex_list, tstruct);

		pars.start(tid, tstr, lex_list);

		syn_analyzer.start();

		pars.print(tid, tstr, lex_list);
        tstruct.print();
	}
	catch(const char * str) {cout << str << endl;}
	catch(...) {cout << "Unexpected error!";}
//	catch(...) {cout << "I dunno, lol ¯\\(°_o)/¯";}
}
