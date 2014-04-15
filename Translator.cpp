#include "lex.h"

int main()
{
	try
	{
		Parser pars;
		pars.start();
        pars.print();
	}
	catch(const char * str) {cout << str << endl;}
	catch(...) {cout << "I dunno, lol ¯\\(°_o)/¯";}
}
