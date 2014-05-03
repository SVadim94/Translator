#include "syn.h"
#define gl() lex = lex_list[++pos].lex_type;
#define step_back() lex_list.get_lex(--pos) //Разберись с этим

//С этой лексемы может начинаться оператор?
inline bool oper_begin(const LexType &lex)
{
	return (lex == LEX_PLUS || lex == LEX_MINUS || lex == LEX_IF ||
		lex == LEX_WHILE || lex == LEX_FOR || lex == LEX_READ ||
		lex == LEX_WRITE || lex == LEX_NUM || lex == LEX_IDENT ||
		lex == LEX_SWITCH || lex == LEX_BREAK || lex == LEX_GOTO ||
		lex == LEX_LCRO || lex == LEX_LPAR);
}

void TSTRUCT::print() const
{
	for (unsigned int i = 0; i < table.size(); ++i)
	{
		cout << table[i].name << endl;
		for (unsigned int j = 0; j < table[i].fields.size(); ++j)
			cout << '\t' << table[i].fields[j].name << " : " << print_lex(table[i].fields[j].type) << endl;
	}
}

void Analyzer::start()
{
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

void Analyzer::STRUCTURES()
{
	gl();

	STRUCTURE();

	while(lex == LEX_STRUCT)
	{
		gl();

		STRUCTURE();
	}
}

void Analyzer::STRUCTURE()
{
	int ident_pos;

	if (lex != LEX_IDENT)
		throw "Syntax error: Expected identificator";

	ident_pos = lex_list.get_value(pos);

	tid.define(ident_pos);

	tstruct.push(tid.get_name(ident_pos));

	gl();

	if (lex != LEX_LCRO)
		throw "Syntax error: Expected {";

	gl();

	SDESCRIPTION();

	if (lex != LEX_SEMICOLON)
		throw "Syntax error: Expected ;";

	gl();

	while (lex == LEX_INT || lex == LEX_BOOL || lex == LEX_STRING)
	{
		SDESCRIPTION();

		if (lex != LEX_SEMICOLON)
			throw "Syntax error: Expected ;";

		gl();
	}

	if (lex != LEX_RCRO)
		throw "Syntax error: Expected }";

	gl();

	if (lex != LEX_SEMICOLON)
		throw "Syntax error: Expected ;";

	gl();
}

void Analyzer::SDESCRIPTION()
{
	int ident_pos;

	gl();

	if (lex != LEX_IDENT)
		throw "Syntax error: Expected identificator";

	ident_pos = lex_list.get_value(pos);

	tstruct.push_field(tid.get_name(ident_pos), lex_list.get_lex(pos - 1));

	gl();
}

void Analyzer::DESCRIPTIONS()
{
	while(lex == LEX_INT || lex == LEX_BOOL || lex == LEX_STRING || lex == LEX_STRUCT)
	{
		DESCRIPTION();

		if (lex != LEX_SEMICOLON)
			throw "Syntax error: Expected ;";

		gl();
	}
}

void Analyzer::DESCRIPTION()
{
	block_type = lex;

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

	if (lex == LEX_STRUCT)
	{
		gl();

		if (lex != LEX_IDENT)
			throw "Syntax error: Expected identificator";
	}

	gl();
}

void Analyzer::VARIABLE()
{
	int ident_pos;

	if (lex != LEX_IDENT)
		throw "Syntax error: Expected identificator";

	ident_pos = lex_list.get_value(pos);
	tid.define(ident_pos);
	tid.set_type(ident_pos, block_type);

	gl();

	if (lex == LEX_ASSIGN && block_type == LEX_STRUCT)
		throw "Syntax error: initialization of structures is not allowed";

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

	tid.initialize(lex_list.get_value(pos - 2), lex_list.get_value(pos));

	gl();
}

void Analyzer::BOOL()
{
	if (lex != LEX_TRUE && lex != LEX_FALSE)
		throw "Syntax error: Expected boolean";

	if (lex == LEX_TRUE)
		tid.initialize(lex_list.get_value(pos - 2), 1);
	else
		tid.initialize(lex_list.get_value(pos - 2), 0);

	gl();
}

void Analyzer::INTEGER()
{
	if (lex != LEX_NUM)
		throw "Syntax error: Expected numeric constant";

	tid.initialize(lex_list.get_value(pos - 2), lex_list.get_value(pos));

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
