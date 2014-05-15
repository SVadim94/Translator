#include "exec.h"

#define get_lexeme() lexeme = poliz[pos++]

void Executer :: start()
{
    uint pos = 0;
    Lex  lexeme;
    Lex  lex_tmp;

    while (pos < poliz.size())
	{
		get_lexeme();

        switch (lexeme.lex_type)
        {
		case LEX_NUM:
		case LEX_STR:
		case LEX_BOOL:
			push(lexeme);
		break;

		case LEX_INT:
			lex_tmp.lex_type = LEX_NUM;
			lex_tmp.value    = tid.get_value(lexeme.value);
			push(lexeme);
		break;
        }
	}
}

void Executer :: push(Lex lexeme)
{
    stack.push_back(lexeme);
}
