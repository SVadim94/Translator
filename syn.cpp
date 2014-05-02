#include "syn.h"
#define gl() pars->get_lex(lex)
#define step_back() pars->back()

//С этой лексемы может начинаться оператор?
bool oper_begin(const LexType &lex)
{
	return (lex == LEX_PLUS || lex == LEX_MINUS || lex == LEX_IF ||
		lex == LEX_WHILE || lex == LEX_FOR || lex == LEX_READ ||
		lex == LEX_WRITE || lex == LEX_NUM || lex == LEX_IDENT ||
		lex == LEX_SWITCH || lex == LEX_BREAK || lex == LEX_GOTO ||
		lex == LEX_LCRO || lex == LEX_LPAR);
}

void Analyzer::start(Parser & parser)
{
	pars = &parser;

	gl();

	PROGRAM();
}

void Analyzer::PROGRAM()
{
	if (lex == LEX_PROGRAM)
	{
		gl();

		if (lex != LEX_LCRO)
			throw "Syntax error: Expected {";

		gl();

		if (lex == LEX_STRUCT)
			STRUCTURES();
		
		DESCRIPTIONS();
		OPERATORS();

		if (lex != LEX_RCRO)
			throw "Syntax error: Expected }";
	}
	else
		throw "Syntax error: Expected 'program'";
}

void Analyzer::STRUCTURES
{
	STRUCTURE();

	while(lex == LEX_STRUCT)
	{
		gl();

		STRUCTURE();
	}
}

void Analyzer::DESCRIPTIONS()
{
	while(lex == LEX_INT || lex == LEX_BOOL || lex == LEX_STRING)
	{
		DESCRIPTION();

		if (lex != LEX_SEMICOLON)
			throw "Syntax error: Expected ;";

		gl();
	}
}

void Analyzer::DESCRIPTION()
{
	TYPE();
	VARIABLE();

	while (lex == LEX_COMMA)
	{
		gl();

		VARIABLE();
	}
}

void Analyzer::TYPE()
{
	if (lex != LEX_INT && lex != LEX_STRING && lex != LEX_BOOL && lex != LEX_STRUCT)
		throw "Syntax error: Expected type name";

	gl();
}

void Analyzer::VARIABLE()
{
	if (lex == LEX_IDENT)
		gl();
	else
		throw "Syntax error: Expected identificator";

	if (lex == LEX_ASSIGN)
	{
		gl();

		CONSTANT();
	}
}

void Analyzer::CONSTANT()
{
	if (lex == LEX_STR)
	{
		STRING();
	}
	else
	if (lex == LEX_TRUE || lex == LEX_FALSE)
	{
		BOOL();
	}
	else
	{
		if (lex == LEX_MINUS || lex == LEX_PLUS)
		{
			gl();

			INTEGER();
		}

		INTEGER();
	}
}

void Analyzer::STRING()
{
	if (lex != LEX_STR)
		throw "Syntax error: Expected string constant";

	gl();
}

void Analyzer::BOOL()
{
	if (lex != LEX_TRUE && lex != LEX_FALSE)
		throw "Syntax error: Expected boolean";

	gl();
}

void Analyzer::INTEGER()
{
	if (lex != LEX_NUM)
		throw "Syntax error: Expected numeric constant";

	gl();
}

void Analyzer::OPERATORS()
{
	while(oper_begin(lex))
	{
		OPERATOR();
	}
}

void Analyzer::OPERATOR()
{
	switch(lex)
	{
		case LEX_IF:
			gl();

			if (lex != LEX_LPAR)
				throw "Syntax error: Expected (";

			gl();

			EXPRESSION();

			if (lex != LEX_RPAR)
				throw "Syntax error: Expected )";

			gl();

			OPERATOR();

			if (lex != LEX_ELSE)
				throw "Syntax error: Expected 'else'";

			gl();

			OPERATOR();
		break;

		case LEX_WHILE:
			gl();

			if (lex != LEX_LPAR)
				throw "Syntax error: Expected (";

			gl();

			EXPRESSION();

			if (lex != LEX_RPAR)
				throw "Syntax error: Expected )";

			gl();

			OPERATOR();
		break;

		case LEX_READ:
			gl();

			if (lex != LEX_LPAR)
				throw "Syntax error: Expected (";

			gl();

			if (lex != LEX_IDENT)
				throw "Syntax error: Expected identificator";

			gl();

			if (lex != LEX_RPAR)
				throw "Syntax error: Expected )";

			gl();

			if (lex != LEX_SEMICOLON)
				throw "Syntax error: Expected ;";

			gl();
		break;

		case LEX_WRITE:
			gl();

			if (lex != LEX_LPAR)
				throw "Syntax error: Expected (";

			gl();

			EXPRESSION();

			while(lex == LEX_COMMA)
				EXPRESSION();

			if (lex != LEX_RPAR)
				throw "Syntax error: Expected )";

			gl();

			if (lex != LEX_SEMICOLON)
				throw "Syntax error: Expected ;";

			gl();
		break;

		case LEX_SWITCH:
			gl();

			if (lex != LEX_LPAR)
				throw "Syntax error: Expected (";

			gl();

			EXPRESSION();

			if (lex != LEX_RPAR)
				throw "Syntax error: Expected )";

			gl();

			if (lex != LEX_LCRO)
				throw "Syntax error: Expected {";

			gl();

			if (lex != LEX_CASE)
				throw "Syntax error: Expected 'case'";

			gl();

			if (lex == LEX_MINUS || lex == LEX_PLUS || lex == LEX_NUM)
				CONSTANT();
			else
				throw "Syntax error: Expected numeric constant";

			if (lex != LEX_COLON)
				throw "Syntax error: Expected :";

			while(oper_begin(lex))
			{
				OPERATOR();
			}

			while (lex == LEX_CASE)
			{
				if (lex == LEX_MINUS || lex == LEX_PLUS || lex == LEX_NUM)
					CONSTANT();
				else
					throw "Syntax error: Expected numeric constant";

				if (lex != LEX_COLON)
					throw "Syntax error: Expected :";

				while(oper_begin(lex))
				{
					OPERATOR();
				}
			}

			if (lex == LEX_DEFAULT)
			{
				gl();

				if (lex == LEX_MINUS || lex == LEX_PLUS || lex == LEX_NUM)
					CONSTANT();
				else
					throw "Syntax error: Expected numeric constant";

				if (lex != LEX_COLON)
					throw "Syntax error: Expected :";

				while(oper_begin(lex))
				{
					OPERATOR();
				}
			}

			if (lex != LEX_RCRO)
				throw "Syntax error: Expected }";
		break;

		case LEX_LCRO:
			gl();

			OPERATORS();

			gl();

			if (lex != LEX_RCRO)
				throw "Syntax error: Expected }";

			gl();
		break;

		case LEX_IDENT:
			gl();

			if (lex == LEX_COLON)
			{
				gl();

				OPERATOR();
			}
			else
			{
				lex = step_back();

				EXPRESSION();

				if (lex != LEX_SEMICOLON)
					throw "Syntax error: Expected ;";

				gl();
			}
		break;

		case LEX_FOR:
			gl();

			if (lex != LEX_LPAR)
				throw "Syntax error: Expected (";

			gl();

			if (lex != LEX_SEMICOLON)
				EXPRESSION();

			if (lex != LEX_SEMICOLON)
				throw "Syntax error: Expected ;";

			gl();

			if (lex != LEX_SEMICOLON)
				EXPRESSION();

			if (lex != LEX_SEMICOLON)
				throw "Syntax error: Expected ;";

			gl();

			EXPRESSION();

			if (lex != LEX_RPAR)
				throw "Syntax error: Expected (";

			gl();

			OPERATOR();
		break;

		case LEX_BREAK:
			//Something poliz
		break;

		case LEX_GOTO:
			gl();

			if (lex != LEX_IDENT)
				throw "Syntax error: Expected identificator";
		gl();

		if (lex != LEX_SEMICOLON)
			throw "Syntax error: Expected ;";

		gl();
		break;
		default:
            EXPRESSION();

            if (lex != LEX_SEMICOLON)
				throw "Syntax error: Expected ;";

			gl();
		break;
	}
}

void Analyzer::EXPRESSION()
{
	E1();

	while(lex == LEX_ASSIGN)
	{
		gl();

		E1();
	}

//	if (lex != LEX_SEMICOLON)
//		throw "Syntax error: Expected ;";
//
//	gl();
}

void Analyzer::E1()
{
	E2();

	while(lex == LEX_OR)
	{
		gl();

		E2();
	}
}

void Analyzer::E2()
{
	E3();

	while(lex == LEX_AND)
	{
		gl();

		E3();
	}
}

void Analyzer::E3()
{
	E4();

	while(lex >= LEX_EQ && lex <= LEX_GTR)
	{
		gl();

		E4();
	}
}

void Analyzer::E4()
{
	E5();

	while(lex == LEX_PLUS || lex == LEX_MINUS)
	{
		gl();

		E5();
	}
}

void Analyzer::E5()
{
	E6();

	while(lex == LEX_MULT || lex == LEX_DIV)
	{
		gl();

		E6();
	}
}

void Analyzer::E6()
{
	if (lex == LEX_NOT)
	{
		gl();

		E7();
	}
	else
	{
		E7();
	}
}

void Analyzer::E7()
{
	if (lex == LEX_MINUS)
	{
		gl();

		E8();
	}
	else
		E8();
}

void Analyzer::E8()
{
	if (lex == LEX_LPAR)
	{
		gl();

		EXPRESSION();

		if (lex != LEX_RPAR)
			throw "Syntax error: Expected )";
	}
	else
	if (lex != LEX_IDENT && lex != LEX_STR && lex != LEX_NUM && lex != LEX_TRUE && lex != LEX_FALSE)
		throw "Syntax error: Expected identificator/constant/expression in brackets";

	if (lex == LEX_IDENT)
	{
		gl();

		if (lex == LEX_DOT)
		{
			gl();

			if (lex != LEX_IDENT)
				throw "Syntax error: wrong '.' usage";

			gl();
		}
	}
	else
		gl();
}
