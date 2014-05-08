#include "syn.h"
#define gl() lex = lex_list[++pos].lex_type;
#define step_back() lex_list.get_lex(--pos) //Разберись с этим
#define pop_op() expression_type_stack.back(); expression_type_stack.pop_back();

//С этой лексемы может начинаться оператор?
inline bool oper_begin(const LexType &lex)
{
	return (lex == LEX_PLUS || lex == LEX_MINUS || lex == LEX_IF ||
		lex == LEX_WHILE || lex == LEX_FOR || lex == LEX_READ ||
		lex == LEX_WRITE || lex == LEX_NUM || lex == LEX_IDENT ||
		lex == LEX_SWITCH || lex == LEX_BREAK || lex == LEX_GOTO ||
		lex == LEX_LCRO || lex == LEX_LPAR);
}

int TSTRUCT::find(int i, const string &str) const
{
	const STRUCT *str_no_i = &(table.at(i));
	for (unsigned int j = 0; j < str_no_i->fields.size(); ++j)
		if (str.compare(str_no_i->fields[j].name) == 0)
			return j;
	return -1;
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
	int ident_pos, struct_pos;

	if (lex != LEX_IDENT)
		throw "Syntax error: Expected identificator";

	ident_pos = lex_list.get_value(pos);
	struct_pos = tstruct.push(tid.get_name(ident_pos));

	tid.define(ident_pos);
	tid.initialize(ident_pos, struct_pos);

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
	int ident_pos, type_pos;

	if (lex != LEX_IDENT)
		throw "Syntax error: Expected identificator";

	ident_pos = lex_list.get_value(pos);
	type_pos = lex_list.get_value(pos - 1);

	if (block_type == LEX_STRUCT)
		tid.initialize(ident_pos, tid.get_value(type_pos));

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
	int ident_pos;

	if (lex != LEX_STR)
		throw "Syntax error: Expected string constant";

	if (block_type != LEX_STRING)
		throw "Syntax error: Type mismatch in initialization";

	ident_pos = lex_list.get_value(pos - 2);

	tid.initialize(ident_pos, lex_list.get_value(pos));
	tid.set_type(ident_pos, block_type);

	gl();
}

void Analyzer::BOOL()
{
	int ident_pos;

	if (lex != LEX_TRUE && lex != LEX_FALSE)
		throw "Syntax error: Expected boolean";

	if (block_type != LEX_BOOL)
		throw "Syntax error: Type mismatch in initialization";

	ident_pos = lex_list.get_value(pos - 2);

	if (lex == LEX_TRUE)
		tid.initialize(ident_pos, 1);
	else
		tid.initialize(ident_pos, 0);

	tid.set_type(ident_pos, block_type);

	gl();
}

void Analyzer::INTEGER()
{
	int ident_pos;

	if (lex != LEX_NUM)
		throw "Syntax error: Expected numeric constant";

	if (block_type != LEX_INT)
		throw "Syntax error: Type mismatch in initialization";

	ident_pos = lex_list.get_value(pos - 2);

	tid.initialize(ident_pos, lex_list.get_value(pos));
	tid.set_type(ident_pos, block_type);

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
	int op;

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

			op = pop_op();

			if (op != LEX_BOOL && op != LEX_TRUE && op != LEX_FALSE)
				throw "Semantic error: Expected boolean as 'if' argument";

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

			op = pop_op();

			if (op != LEX_BOOL && op != LEX_TRUE && op != LEX_FALSE)
				throw "Semantic error: Expected boolean as 'while' argument";

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
			else
			if (!tid.is_defined(lex_list.get_value(pos)))
				throw "Syntax error: Undefined identificator";

			op = tid.get_type(lex_list.get_value(pos));

			if (op != LEX_INT && op != LEX_STRING)
				throw "Semantic error: Expected integer/string variable";

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

			pop_op();

			while(lex == LEX_COMMA)
			{
				gl();

				EXPRESSION();

				pop_op();
			}

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

			op = pop_op();

			if (op != LEX_INT && op != LEX_NUM)
				throw "Semantic error: Expected integer as switch argument";

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

				if (!tid.is_defined(lex_list.get_value(pos)))
					throw "Syntax error: Undefined identificator";

				EXPRESSION();

				pop_op();

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
			{
				EXPRESSION();

				pop_op();

				if (lex != LEX_SEMICOLON)
					throw "Syntax error: Expected ;";
			}

			gl();

			if (lex != LEX_SEMICOLON)
			{
				EXPRESSION();

				op = pop_op();

				if (op != LEX_BOOL && op != LEX_TRUE && op != LEX_FALSE)
					throw "Semantic error: Expected boolean as 'for' second argument";

				if (lex != LEX_SEMICOLON)
					throw "Syntax error: Expected ;";
			}

			gl();

			if (lex != LEX_RPAR)
			{
				EXPRESSION();

				pop_op();

				if (lex != LEX_RPAR)
					throw "Syntax error: Expected )";
			}

			gl();

			OPERATOR();
		break;

		case LEX_BREAK:
			//Something poliz
			gl();

			if (lex != LEX_SEMICOLON)
				throw "Syntax error: Expected ';' after break";

			gl();
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

			pop_op();

			if (lex != LEX_SEMICOLON)
				throw "Syntax error: Expected ;";

			gl();
		break;
	}
}

void Analyzer::EXPRESSION()
{
	LexType op1, op2;

	E1();

	op1 = pop_op();

	while(lex == LEX_ASSIGN)
	{
		gl();

		E1();

		op2 = pop_op();

		check_assign(op1, op2);
	}

	expression_type_stack.push_back(op1);

//	if (lex != LEX_SEMICOLON)
//		throw "Syntax error: Expected ;";
//
//	gl();
}

void Analyzer::E1()
{
	LexType op1, op2;

	E2();

	op1 = pop_op();

	while(lex == LEX_OR)
	{
		gl();

		E2();

		op2 = pop_op();

		check_logic(op1, op2);
	}

	expression_type_stack.push_back(op1);
}

void Analyzer::E2()
{
	LexType op1, op2;

	E3();

	op1 = pop_op();

	while(lex == LEX_AND)
	{
		gl();

		E3();

		op2 = pop_op();

		check_logic(op1, op2);
	}

	expression_type_stack.push_back(op1);
}

void Analyzer::E3()
{
	LexType op1, op2;

	E4();

	op1 = pop_op();

	if (is_cmp(lex))
	{
		do
		{
			gl();

			E4();

			op2 = pop_op();

			check_comparison(op1, op2);

			expression_type_stack.push_back(LEX_TRUE); //Не важно true или false. Главное, логическая константа
		}
		while (is_cmp(lex));
	}
	else
		expression_type_stack.push_back(op1);
}

void Analyzer::E4()
{
	LexType op1, op2, op_type;

	E5();

	op1 = pop_op();

	while(lex == LEX_PLUS || lex == LEX_MINUS)
	{
		op_type = lex;

		gl();

		E5();

		op2 = pop_op();

		check_arithmetic(op1, op2, op_type);
	}

	expression_type_stack.push_back(op1);
}

void Analyzer::E5()
{
	LexType op1, op2, op_type;

	E6();

	op1 = pop_op();

	while(lex == LEX_MULT || lex == LEX_DIV)
	{
		op_type = lex;

		gl();

		E6();

		op2 = pop_op();

		check_arithmetic(op1, op2, op_type);
	}

	expression_type_stack.push_back(op1);
}

void Analyzer::E6()
{
	LexType op1;

	if (lex == LEX_NOT)
	{
		gl();

		E7();

		op1 = pop_op();

		check_not(op1);

		expression_type_stack.push_back(op1);
	}
	else
		E7();
}

void Analyzer::E7()
{
	LexType op1;

	if (lex == LEX_MINUS)
	{
		gl();

		E8();

		op1 = pop_op();

		check_minus(op1);

		expression_type_stack.push_back(op1);
	}
	else
		E8();
}

void Analyzer::E8()
{
	int ident_pos, type_pos;

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

	switch(lex)
	{
		case LEX_IDENT:
			if (!tid.is_defined(lex_list.get_value(pos)))
				throw "Syntax error: Undefined identificator";

			gl();

			if (lex == LEX_DOT)
			{
				gl();

				if (lex != LEX_IDENT)
					throw "Syntax error: wrong '.' usage";

// В случае структур в поле value хранится номер записи соответстующего типа в таблице структур
				type_pos = lex_list.get_value(pos - 2);
				ident_pos = lex_list.get_value(pos);

				if (tstruct.find(tid.get_value(type_pos), tid.get_name(ident_pos)) == -1)
					throw "Syntax error: No such field in this structure";

				expression_type_stack.push_back(tid.get_type(ident_pos));

				gl();
			}
			else
				expression_type_stack.push_back(tid.get_type(lex_list.get_value(pos - 1)));
		break;
		case LEX_RPAR:
			//Wow, such handy...
			gl();
		break;
		default:
			expression_type_stack.push_back(lex);

			gl();
		break;
	}
}

inline void check_assign(LexType op1, LexType op2)
{
	if (op1 == LEX_NUM || op1 == LEX_STR || op1 == LEX_TRUE || op1 == LEX_FALSE)
		throw "Semantic error: Trying to assign to constant";

	if (!((op1 == op2) || (op1 == LEX_BOOL && (op2 == LEX_TRUE || op2 == LEX_FALSE)) ||
		(op1 == LEX_STRING && op2 == LEX_STR) || (op1 == LEX_INT && op2 == LEX_NUM)))
			throw "Semantic error: Type mismatch";
}

inline void check_logic(LexType op1, LexType op2)
{
	if (op1 != LEX_BOOL && op1 != LEX_TRUE && op1 != LEX_FALSE)
		throw "Semantic error: first operand must be boolean";

	if (op2 != LEX_BOOL && op2 != LEX_TRUE && op2 != LEX_FALSE)
		throw "Semantic error: second operand must be boolean";
}

inline void check_comparison(LexType op1, LexType op2)
{
	if (op1 == LEX_BOOL || op1 == LEX_TRUE || op1 == LEX_FALSE)
		throw "Semantic error: boolean comparison is not allowed";

	if (op2 == LEX_BOOL || op2 == LEX_TRUE || op2 == LEX_FALSE)
		throw "Semantic error: boolean comparison is not allowed";

	if (op1 == LEX_STRING)
		op1 =  LEX_STR;
	else
	if (op1 == LEX_INT)
		op1 =  LEX_NUM;

	if (op2 == LEX_STRING)
		op2 =  LEX_STR;
	else
	if (op2 == LEX_INT)
		op2 =  LEX_NUM;

	if (op1 != op2)
		throw "Semantic error: cannot compare string with integer";
}

inline void check_arithmetic(LexType op1, LexType op2, LexType lex)
{
	if (op1 == LEX_NUM)
		op1 =  LEX_INT;

	if (op2 == LEX_NUM)
		op2 =  LEX_INT;

	if (lex != LEX_PLUS)
	{
		if (op1 != LEX_INT)
			throw "Semantic error: first operand must be integer";

		if (op2 != LEX_INT)
			throw "Semantic error: second operand must be integer";
	}
	else
	{
		if (op1 == LEX_STR)
			op1 =  LEX_STRING;

		if (op2 == LEX_STR)
			op2 =  LEX_STRING;

		if (op1 != LEX_INT && op1 != LEX_STRING)
			throw "Semantic error: '+' first argument must be integer/string";

		if (op2 != LEX_INT && op2 != LEX_STRING)
			throw "Semantic error: '+' second argument must be integer/string";

		if (op1 != op2)
			throw "Semantic error: Type mismatch";
	}
}

inline void check_not(LexType op)
{
	if (op != LEX_BOOL && op != LEX_TRUE && op != LEX_FALSE)
		throw "Semantic error: 'not' argument must be boolean";
}

inline void check_minus(LexType op)
{
	if (op != LEX_NUM && op != LEX_INT)
		throw "Semantic error: '-' argument must be integer";
}

inline bool is_cmp(LexType lex)
{
	return 	lex == LEX_EQ 	|| lex == LEX_NEQ || lex == LEX_GTR ||
			lex == LEX_LSS	|| lex == LEX_GEQ || lex == LEX_LEQ;
}
