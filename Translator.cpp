#include <fstream>
#include <iostream>

#ifndef __tables__
#include "tables.h"
#endif

#include "lex.h"
#include "syn.h"
#include "exec.h"

using namespace std;

int main(int argc, char *argv[])
{
	try
	{
		if (argc < 3)
			throw "Error! No filename!";

		ifstream fin(argv[1]);
		ofstream fout(argv[2]);

		TID     tid;
		TSTR    tstr;
		LexList lex_list;
		TSTRUCT tstruct;
		POLIZ   poliz;

		Parser   parser(fin, tid, tstr, lex_list);
		Analyzer syn_analyzer(tid, tstr, lex_list, tstruct, poliz);
		Executer exec(tid, tstr, tstruct, poliz);

		parser.start();

		syn_analyzer.start();

		exec.start();

		parser.print(fout, tid, tstr, lex_list);
		tstruct.print(fout);
		poliz.print(fout);
	}
	catch(const char * str) {cout << str << endl;}
	catch(...) {cout << "Unexpected error!";}
}
