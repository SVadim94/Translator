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
		ifstream fin;
		ofstream fout;

		if (argc == 1)
			throw "No file name!";
		else
		if (argc == 2)
			cout << "Warning : Debug info won't be printed!" << endl;
		else
			fout.open(argv[2]);

		fin.open(argv[1]);

		if (fin.fail())
			throw "Error while opening file!";

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

        if (argc == 2)
		{
			parser.print(fout, tid, tstr, lex_list);
			tstruct.print(fout);
			poliz.print(fout);
		}
	}
	catch(const char * str) {cout << str << endl;}
	catch(...) {cout << "Unexpected error!";}
}
