#include "syn.h"
#define gl() lex = lex_list[++pos].lex_type;
#define step_back() lex_list.get_lex(--pos) //Разберись с этим
#define pop_op() expression_type_stack.back(); expression_type_stack.pop_back();

//С этой лексемы может начинаться оператор?
inline bool oper_begin(const LexType &lex)
{
	return (
		lex == LEX_PLUS   || lex == LEX_MINUS || lex == LEX_IF    ||
		lex == LEX_WHILE  || lex == LEX_FOR   || lex == LEX_READ  ||
		lex == LEX_WRITE  || lex == LEX_NUM   || lex == LEX_IDENT ||
		lex == LEX_SWITCH || lex == LEX_BREAK || lex == LEX_GOTO  ||
		lex == LEX_LCRO   || lex == LEX_LPAR
		);
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

void POLIZ::print() const
{
	for (unsigned int i = 0; i < poliz.size(); ++i)
		cout << '#' << i << ": " << print_lex(poliz[i].lex_type) << " : " << poliz[i].value << endl;
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

		struct_block_type = lex_list.get_value(pos);
	}

	gl();
}

void Analyzer::VARIABLE()
{
	int ident_pos;

	if (lex != LEX_IDENT)
		throw "Syntax error: Expected identificator";

	ident_pos = lex_list.get_value(pos);

	if (block_type == LEX_STRUCT)
		tid.initialize(ident_pos, struct_block_type);

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
	LexType op;
	unsigned int label_index;
	unsigned int target_index;
	int ident_pos;

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

			lexeme.lex_type = POLIZ_LABEL;
			lexeme.value    = -1; // Заполнить потом!!!
			label_index     = poliz.push(lexeme);

			poliz.label_push(label_index);

			lexeme.lex_type = POLIZ_FGO;
			lexeme.value    = -1; //Ни оптимизации ради, а токмо в силу связавших меня обязательств по соблюдению логики программы...

			poliz.push(lexeme);

			gl();

			OPERATOR();

			if (lex != LEX_ELSE)
				throw "Syntax error: Expected 'else'";

			lexeme.lex_type = POLIZ_LABEL;
			lexeme.value    = -1;
			label_index     = poliz.push(lexeme);

			poliz.label_push(label_index);

			lexeme.lex_type = POLIZ_GO;
			lexeme.value    = -1;

			poliz.push(lexeme);

			gl();

			OPERATOR();

			target_index               = poliz.current();

			label_index                = poliz.label_pop();
			poliz[label_index].value   = target_index; // Теперь ссылки указывают на ';'

			label_index                = poliz.label_pop();
			poliz[label_index].value   = target_index; // Обе ссылки...

			lexeme.lex_type = LEX_SEMICOLON;
			lexeme.value    = -1;
			
			poliz.push(lexeme);
		break;

		case LEX_WHILE:
			gl();

			if (lex != LEX_LPAR)
				throw "Syntax error: Expected (";

			gl();

			target_index = poliz.current();

			EXPRESSION();

			if (lex != LEX_RPAR)
				throw "Syntax error: Expected )";

			op = pop_op();

			if (op != LEX_BOOL && op != LEX_TRUE && op != LEX_FALSE)
				throw "Semantic error: Expected boolean as 'while' argument";

			lexeme.lex_type = POLIZ_LABEL;
			lexeme.value    = -1; // На конец!
			label_index     = poliz.push(lexeme);

			poliz.label_push(label_index);

			lexeme.lex_type = POLIZ_FGO;
			lexeme.value    = -1;

			poliz.push(lexeme);

			gl();

			OPERATOR();

			lexeme.lex_type = POLIZ_LABEL;
			lexeme.value    = target_index;

			poliz.push(lexeme);

			lexeme.lex_type = POLIZ_GO;
			lexeme.value    = -1;

			poliz.push(lexeme);

			target_index             = poliz.current();
			label_index              = poliz.label_pop();
			poliz[label_index].value = target_index;

			lexeme.lex_type = LEX_SEMICOLON; //Подумай о целесообразности этой штуковины
			lexeme.value    = -1;
			poliz.push(lexeme);
		break;

		case LEX_READ:
			gl();

			if (lex != LEX_LPAR)
				throw "Syntax error: Expected (";

			gl();

			ident_pos = lex_list.get_value(pos);

			if (lex != LEX_IDENT)
				throw "Syntax error: Expected identificator";
			else
			if (!tid.is_defined(ident_pos))
				throw "Syntax error: Undefined identificator";

			op = tid.get_type(ident_pos);

			if (op != LEX_INT && op != LEX_STRING)
				throw "Semantic error: Expected integer/string variable";

			poliz.push(lex_list[pos]);

			gl();

			if (lex != LEX_RPAR)
				throw "Syntax error: Expected )";

			lexeme.lex_type = LEX_READ;
			lexeme.value    = -1;

			poliz.push(lexeme);

			gl();

			if (lex != LEX_SEMICOLON)
				throw "Syntax error: Expected ;";

			lexeme.lex_type = LEX_SEMICOLON;
			lexeme.value    = -1;

			poliz.push(lexeme);

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
	LexType  op1, op2;
	unsigned int assign_count = 0;

	E1();

	op1 = pop_op();

	while(lex == LEX_ASSIGN)
	{
		poliz.pop();

		lexeme.lex_type = POLIZ_ADDRESS;
		lexeme.value    = lex_list.get_value(pos - 1); //Номер в ТИД

		poliz.push(lexeme);

		gl();

		++assign_count;

		E1();

		op2 = pop_op();

		check_assign(op1, op2);
	}

	lexeme.lex_type = LEX_ASSIGN;
	lexeme.value    = -1;

	for (unsigned int i = 0; i < assign_count; ++i)
	{
		poliz.push(lexeme);
	}

	expression_type_stack.push_back(op1);
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

		lexeme.lex_type = LEX_OR;
		lexeme.value    = -1;

		poliz.push(lexeme);
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

		lexeme.lex_type = LEX_AND;
		lexeme.value    = -1;

		poliz.push(lexeme);
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
			lexeme.lex_type = lex;
			lexeme.value    = -1;

			gl();

			E4();

			op2 = pop_op();

			check_comparison(op1, op2);

			poliz.push(lexeme);

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

		lexeme.lex_type = op_type;
		lexeme.value    = -1;

		poliz.push(lexeme);
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

		lexeme.lex_type = op_type;
		lexeme.value    = -1;

		poliz.push(lexeme);
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

		lexeme.lex_type = LEX_NOT;
		lexeme.value    = -1;

		poliz.push(lexeme);

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

		lexeme.lex_type = LEX_MINUS;
		lexeme.value    = -1;

		poliz.push(lexeme);

		expression_type_stack.push_back(op1);
	}
	else
		E8();
}

void Analyzer::E8()
{
	int ident_pos  = 0; // Позиция лексемы в ТИД
	int type_pos   = 0; // Позиция переменной-структуры в ТИД
	int struct_pos = 0; // Позиция переменной структуры в таблице структур
	int field_pos  = 0; // Позиция поля в таблице структур
	int tid_pos    = 0; // Позиция поля в ТИД

	LexType type;
	string field_name;

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
				type_pos   = lex_list.get_value(pos - 2);
				ident_pos  = lex_list.get_value(pos);
				struct_pos = tid.get_value(type_pos);
				field_pos  = tstruct.find(struct_pos, tid.get_name(ident_pos));

				if (field_pos == -1)
					throw "Syntax error: No such field in this structure";

				type       = tstruct[struct_pos].fields[field_pos].type;
				field_name = tid.get_name(type_pos) + "." + tstruct[struct_pos].fields[field_pos].name;

				if ((tid_pos = tid.find(field_name)) == -1)
				{
					tid_pos = tid.push(field_name);
					tid.set_type(tid_pos, type);
				}

				lexeme.lex_type = type;
				lexeme.value    = tid_pos;

				poliz.push(lexeme);

				expression_type_stack.push_back(type);

				gl();
			}
			else
			{
				poliz.push(lex_list[pos - 1]);
				expression_type_stack.push_back(tid.get_type(lex_list.get_value(pos - 1)));
			}
		break;

		case LEX_RPAR:
			//Wow, such handy...
			gl();
		break;

		default:
			poliz.push(lex_list[pos]);

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
