#include "syn.h"

#define get_lexeme() lex = lex_list[++pos].lex_type;
#define step_back()  lex_list.get_lex(--pos)
#define pop_op()     expression_type_stack.back(); expression_type_stack.pop_back();


//С этой лексемы может начинаться оператор?
inline bool oper_begin(const LexType &lex)
{
	return (
		lex == LEX_PLUS   || lex == LEX_MINUS || lex == LEX_IF    ||
		lex == LEX_WHILE  || lex == LEX_FOR   || lex == LEX_READ  ||
		lex == LEX_WRITE  || lex == LEX_NUM   || lex == LEX_IDENT ||
		lex == LEX_SWITCH || lex == LEX_BREAK || lex == LEX_GOTO  ||
		lex == LEX_LCRO   || lex == LEX_LPAR  || lex == LEX_UMIN
		);
}

void Analyzer::start()
{
	get_lexeme();

	PROGRAM();
}

void Analyzer::PROGRAM()
{
	if (lex == LEX_PROGRAM)
	{
		get_lexeme();

		if (lex != LEX_LCRO)
			throw "Syntax error: Expected {";

		get_lexeme();

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
	get_lexeme();

	STRUCTURE();

	while(lex == LEX_STRUCT)
	{
		get_lexeme();

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

	get_lexeme();

	if (lex != LEX_LCRO)
		throw "Syntax error: Expected {";

	get_lexeme();

	SDESCRIPTION();

	if (lex != LEX_SEMICOLON)
		throw "Syntax error: Expected ;";

	get_lexeme();

	while (lex == LEX_INT || lex == LEX_BOOL || lex == LEX_STRING)
	{
		SDESCRIPTION();

		if (lex != LEX_SEMICOLON)
			throw "Syntax error: Expected ;";

		get_lexeme();
	}

	if (lex != LEX_RCRO)
		throw "Syntax error: Expected }";

	get_lexeme();

	if (lex != LEX_SEMICOLON)
		throw "Syntax error: Expected ';' after struct description";

	get_lexeme();
}

void Analyzer::SDESCRIPTION()
{
	int ident_pos;

	get_lexeme();

	if (lex != LEX_IDENT)
		throw "Syntax error: Expected identificator";

	ident_pos = lex_list.get_value(pos);

	tid.set_type(ident_pos, LEX_LAST); //Носит флаговый характер

	tstruct.push_field(tid.get_name(ident_pos), lex_list.get_lex(pos - 1));

	get_lexeme();
}

void Analyzer::DESCRIPTIONS()
{
	while(lex == LEX_INT || lex == LEX_BOOL || lex == LEX_STRING || lex == LEX_STRUCT)
	{
		DESCRIPTION();

		if (lex != LEX_SEMICOLON)
			throw "Syntax error: Expected ;";

		get_lexeme();
	}
}

void Analyzer::DESCRIPTION()
{
	block_type = lex;

	TYPE();
	VARIABLE();

	while (lex == LEX_COMMA)
	{
		get_lexeme();

		VARIABLE();
	}
}

void Analyzer::TYPE()
{
	if (lex != LEX_INT && lex != LEX_STRING && lex != LEX_BOOL && lex != LEX_STRUCT)
		throw "Syntax error: Expected type name";

	if (lex == LEX_STRUCT)
	{
		get_lexeme();

		if (lex != LEX_IDENT)
			throw "Syntax error: Expected identificator";

		struct_block_type = tid.get_value(lex_list.get_value(pos));
	}

	get_lexeme();
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

	get_lexeme();

	if (lex == LEX_ASSIGN && block_type == LEX_STRUCT)
		throw "Syntax error: initialization of structures is not allowed";

	if (lex == LEX_ASSIGN)
	{
		get_lexeme();

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
			get_lexeme();

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

	get_lexeme();
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

	get_lexeme();
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

	get_lexeme();
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
	LexType op, type;

	//label_index_i
	uint lab_i_1 = 0, lab_i_2 = 0;
	uint lab_i_3 = 0, lab_i_4 = 0;

	//target_index_i
	uint tar_i_1 = 0, tar_i_2 = 0;
	uint tar_i_3 = 0, tar_i_4 = 0;

	int ident_pos, write_argc = 0;
	int type_pos,  struct_pos;
	int field_pos;

	string field_name;


	switch(lex)
	{
		case LEX_IF:
			get_lexeme();

			if (lex != LEX_LPAR)
				throw "Syntax error: Expected (";

			get_lexeme();

			EXPRESSION();

			if (lex != LEX_RPAR)
				throw "Syntax error: Expected )";

			op = pop_op();

			if (op != LEX_BOOL && op != LEX_TRUE && op != LEX_FALSE)
				throw "Semantic error: Expected boolean as 'if' argument";

			lexeme.lex_type = POLIZ_LABEL;
			lexeme.value    = -1; // Заполнить потом!!!
			lab_i_1         = poliz.push(lexeme);

			lexeme.lex_type = POLIZ_FGO;
			lexeme.value    = -1; //Ни оптимизации ради, а токмо в силу связавших меня обязательств по соблюдению логики программы...
			poliz.push(lexeme);

			get_lexeme();

			OPERATOR();

			if (lex != LEX_ELSE)
				throw "Syntax error: Expected 'else'";

			lexeme.lex_type = POLIZ_LABEL;
			lexeme.value    = -1;
			lab_i_2         = poliz.push(lexeme);

			lexeme.lex_type = POLIZ_GO;
			lexeme.value    = -1;
			poliz.push(lexeme);

			get_lexeme();

			OPERATOR();

			tar_i_1              = poliz.current();
			poliz[lab_i_1].value = tar_i_1; // Теперь ссылки указывают на ';'
			poliz[lab_i_2].value = tar_i_1; // Обе ссылки...

			lexeme.lex_type = LEX_SEMICOLON;
			lexeme.value    = -1;
			poliz.push(lexeme);
		break;

		case LEX_WHILE:
			break_stack.forbid();

			get_lexeme();

			if (lex != LEX_LPAR)
				throw "Syntax error: Expected (";

			get_lexeme();

			tar_i_2 = poliz.current();

			EXPRESSION();

			if (lex != LEX_RPAR)
				throw "Syntax error: Expected )";

			op = pop_op();

			if (op != LEX_BOOL && op != LEX_TRUE && op != LEX_FALSE)
				throw "Semantic error: Expected boolean as 'while' argument";

			lexeme.lex_type = POLIZ_LABEL;
			lexeme.value    = -1; // На конец!
			lab_i_1         = poliz.push(lexeme);

			lexeme.lex_type = POLIZ_FGO;
			lexeme.value    = -1;
			poliz.push(lexeme);

			break_stack.allow();

			get_lexeme();

			OPERATOR();

			lexeme.lex_type = POLIZ_LABEL;
			lexeme.value    = tar_i_2;
			poliz.push(lexeme);

			lexeme.lex_type = POLIZ_GO;
			lexeme.value    = -1;
			poliz.push(lexeme);

			tar_i_1                  = poliz.current();
			poliz[lab_i_1].value     = tar_i_1;

			lexeme.lex_type = LEX_SEMICOLON; //Подумай о целесообразности этой штуковины
			lexeme.value    = -1;
			poliz.push(lexeme);

			while((lab_i_1 = break_stack.pop()) != -1)
				poliz[lab_i_1].value = tar_i_1;
		break;

		case LEX_READ:
			get_lexeme();

			if (lex != LEX_LPAR)
				throw "Syntax error: Expected (";

			get_lexeme();

			ident_pos = lex_list.get_value(pos);

			if (lex != LEX_IDENT)
				throw "Syntax error: Expected identificator";
			else
			if (!tid.is_defined(ident_pos))
				throw "Syntax error: Undefined identificator";

			get_lexeme();

			if (lex == LEX_DOT)
			{
				get_lexeme();

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

				if ((ident_pos = tid.find(field_name)) == -1)
				{
					ident_pos = tid.push(field_name);
					tid.set_type(ident_pos, type);
				}

				lexeme.lex_type = POLIZ_ADDRESS;
				lexeme.value    = ident_pos;
				poliz.push(lexeme);
			}
			else
			{
				step_back();

				op = tid.get_type(ident_pos);

				if (op != LEX_INT && op != LEX_STRING)
					throw "Semantic error: Expected integer/string variable";

				lexeme.lex_type = POLIZ_ADDRESS;
				lexeme.value    = lex_list[pos].value;
				poliz.push(lexeme);
			}

			get_lexeme();

			if (lex != LEX_RPAR)
				throw "Syntax error: Expected )";

			lexeme.lex_type = LEX_READ;
			lexeme.value    = -1;
			poliz.push(lexeme);

			get_lexeme();

			if (lex != LEX_SEMICOLON)
				throw "Syntax error: Expected ;";

			lexeme.lex_type = LEX_SEMICOLON;
			lexeme.value    = -1;
			poliz.push(lexeme);

			get_lexeme();
		break;

		case LEX_WRITE:
			get_lexeme();

			if (lex != LEX_LPAR)
				throw "Syntax error: Expected (";

			get_lexeme();

			EXPRESSION();

			pop_op();

			++write_argc;

			while(lex == LEX_COMMA)
			{
				get_lexeme();

				EXPRESSION();

				pop_op();

				++write_argc;
			}

			if (lex != LEX_RPAR)
				throw "Syntax error: Expected )";

			get_lexeme();

			lexeme.lex_type = LEX_NUM;
			lexeme.value    = write_argc;
			poliz.push(lexeme);

			lexeme.lex_type = LEX_WRITE;
			lexeme.value    = -1;
			poliz.push(lexeme);

			if (lex != LEX_SEMICOLON)
				throw "Syntax error: Expected ;";

			lexeme.lex_type = LEX_SEMICOLON;
			lexeme.value    = -1;
			poliz.push(lexeme);

			get_lexeme();
		break;

		case LEX_SWITCH:
/*Используется стек меток, наверное...
Короче, время покажет...
*/
			get_lexeme();

			if (lex != LEX_LPAR)
				throw "Syntax error: Expected (";

			get_lexeme();

			EXPRESSION();

			op = pop_op();

			if (op != LEX_INT && op != LEX_NUM)
				throw "Semantic error: Expected integer as switch argument";

			if (lex != LEX_RPAR)
				throw "Syntax error: Expected )";

			lexeme.lex_type = LEX_SWITCH;
			lexeme.value    = -1;
			poliz.push(lexeme);

			break_stack.allow();

			get_lexeme();

			if (lex != LEX_LCRO)
				throw "Syntax error: Expected {";

			get_lexeme();

			if (lex != LEX_CASE)
				throw "Syntax error: Expected 'case'";

			get_lexeme();

			if (lex == LEX_MINUS)
			{
				get_lexeme();

				if (lex != LEX_NUM)
					throw "Syntax error: Expected numeric constant in switch";

				lexeme.lex_type = LEX_NUM;
				lexeme.value    = - lex_list[pos].value;
				poliz.push(lexeme);
			}
			else
			if (lex == LEX_PLUS)
			{
				get_lexeme();

				if (lex != LEX_NUM)
					throw "Syntax error: Expected numeric constant in switch";

				lexeme.lex_type = LEX_NUM;
				lexeme.value    = lex_list[pos].value;
				poliz.push(lexeme);
			}
			else
			if (lex == LEX_NUM)
			{
				lexeme.lex_type = LEX_NUM;
				lexeme.value    = lex_list[pos].value;
				poliz.push(lexeme);
			}
			else
				throw "Syntax error: Expected numeric constant";

			get_lexeme();

			if (lex != LEX_COLON)
				throw "Syntax error: Expected :";

			get_lexeme();

			lexeme.lex_type = POLIZ_LABEL;
			lexeme.value    = -1;
			lab_i_1         = poliz.push(lexeme);

			lexeme.lex_type = LEX_CASE;
			lexeme.value    = -1;
			poliz.push(lexeme);

			while(oper_begin(lex))
			{
				OPERATOR();
			}

			while (lex == LEX_CASE)
			{
				get_lexeme();

				lexeme.lex_type = POLIZ_LABEL;
				lexeme.value    = poliz.current() + 5;
				poliz.push(lexeme);

				lexeme.lex_type = POLIZ_GO;
				lexeme.value    = -1;
				poliz.push(lexeme);

				if (lex == LEX_MINUS)
				{
					get_lexeme();

					if (lex != LEX_NUM)
						throw "Syntax error: Expected numeric constant in switch";

					lexeme.lex_type = LEX_NUM;
					lexeme.value    = - lex_list[pos].value;
					poliz.push(lexeme);
				}
				else
				if (lex == LEX_PLUS)
				{
					get_lexeme();

					if (lex != LEX_NUM)
						throw "Syntax error: Expected numeric constant in switch";

					lexeme.lex_type = LEX_NUM;
					lexeme.value    = lex_list[pos].value;
					poliz.push(lexeme);
				}
				else
				if (lex == LEX_NUM)
				{
					lexeme.lex_type = LEX_NUM;
					lexeme.value    = lex_list[pos].value;
					poliz.push(lexeme);
				}
				else
					throw "Syntax error: Expected numeric constant";

				get_lexeme();

				if (lex != LEX_COLON)
					throw "Syntax error: Expected :";

				get_lexeme();

				poliz[lab_i_1].value = poliz.current() - 1;

				lexeme.lex_type = POLIZ_LABEL;
				lexeme.value    = -1;
				lab_i_1         = poliz.push(lexeme);

				lexeme.lex_type = LEX_CASE;
				lexeme.value    = -1;
				poliz.push(lexeme);

				while(oper_begin(lex))
				{
					OPERATOR();
				}
			}

			poliz[lab_i_1].value = poliz.current();

			if (lex == LEX_DEFAULT)
			{
				get_lexeme();

				if (lex != LEX_COLON)
					throw "Syntax error: Expected :";

				get_lexeme();

				while(oper_begin(lex))
				{
					OPERATOR();
				}
			}

			if (lex != LEX_RCRO)
				throw "Syntax error: Expected }";

			get_lexeme();

			lexeme.lex_type = LEX_DEFAULT;
			lexeme.value    = -1;
			tar_i_1         = poliz.push(lexeme);

			while((lab_i_1 = break_stack.pop()) != -1)
				poliz[lab_i_1].value = tar_i_1;
		break;

		case LEX_LCRO:
			get_lexeme();

			OPERATORS();

			if (lex != LEX_RCRO)
				throw "Syntax error: Expected }";

			get_lexeme();
		break;

		case LEX_IDENT:
			get_lexeme();

			if (lex == LEX_COLON)
			{
				ident_pos = lex_list.get_value(pos - 1);

				if (tid.is_defined(ident_pos))
					throw "Semantic error: duplicate label!";

				tid.set_type(ident_pos, LEX_LABEL);
				tid.define(ident_pos);

				if (!tid.is_init(ident_pos))
					tid.initialize(ident_pos, poliz.current());
				else
					poliz[tid.get_value(ident_pos)].value = poliz.current();

				get_lexeme();

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

				lexeme.lex_type = LEX_SEMICOLON;
				lexeme.value    = -1;
				poliz.push(lexeme);

				get_lexeme();
			}
		break;

		case LEX_FOR:
			break_stack.forbid();

			get_lexeme();

			if (lex != LEX_LPAR)
				throw "Syntax error: Expected (";

			get_lexeme();

			if (lex != LEX_SEMICOLON)
			{
				EXPRESSION();

				pop_op();

				if (lex != LEX_SEMICOLON)
					throw "Syntax error: Expected ;";
			}

			lexeme.lex_type = LEX_SEMICOLON;
			lexeme.value    = -1;
			poliz.push(lexeme);

			tar_i_3 = poliz.current();

			get_lexeme();

			if (lex != LEX_SEMICOLON)
			{
				EXPRESSION();

				op = pop_op();

				if (op != LEX_BOOL && op != LEX_TRUE && op != LEX_FALSE)
					throw "Semantic error: Expected boolean as 'for' second argument";

				if (lex != LEX_SEMICOLON)
					throw "Syntax error: Expected ;";
			}

			lexeme.lex_type = POLIZ_LABEL;
			lexeme.value    = -1;
			lab_i_1         = poliz.push(lexeme);

			lexeme.lex_type = POLIZ_FGO;
			lexeme.value    = -1;
			poliz.push(lexeme);

			lexeme.lex_type = POLIZ_LABEL;
			lexeme.value    = -1;
			lab_i_2         = poliz.push(lexeme);

			lexeme.lex_type = POLIZ_GO;
			lexeme.value    = -1;
			poliz.push(lexeme);

			lexeme.lex_type = LEX_SEMICOLON;
			lexeme.value    = -1;
			poliz.push(lexeme);

			get_lexeme();

			tar_i_4 = poliz.current();

			if (lex != LEX_RPAR)
			{
				EXPRESSION();

				pop_op();

				if (lex != LEX_RPAR)
					throw "Syntax error: Expected )";
			}

			lexeme.lex_type = POLIZ_LABEL;
			lexeme.value    = -1;
			lab_i_3         = poliz.push(lexeme);

			lexeme.lex_type = POLIZ_GO;
			lexeme.value    = -1;
			poliz.push(lexeme);

			break_stack.allow();

			get_lexeme();

			tar_i_2 = poliz.current();

			OPERATOR();

			lexeme.lex_type = POLIZ_LABEL;
			lexeme.value    = -1;
			lab_i_4         = poliz.push(lexeme);

			lexeme.lex_type = POLIZ_GO;
			lexeme.value    = -1;
			poliz.push(lexeme);

			lexeme.lex_type = LEX_SEMICOLON;
			lexeme.value    = -1;
			tar_i_1         = poliz.push(lexeme);

			poliz[lab_i_1].value = tar_i_1;
			poliz[lab_i_2].value = tar_i_2;
			poliz[lab_i_3].value = tar_i_3;
			poliz[lab_i_4].value = tar_i_4;

			while((lab_i_1 = break_stack.pop()) != -1)
				poliz[lab_i_1].value = tar_i_1;
		break;

		case LEX_BREAK:
			//Something poliz
			get_lexeme();

			if (!break_stack.is_allowed())
				throw "Semantic error: 'break' must be in loop / switch";

			break_stack.push(poliz.current());

			lexeme.lex_type = LEX_BREAK;
			lexeme.value    = -1;
			poliz.push(lexeme);

			if (lex != LEX_SEMICOLON)
				throw "Syntax error: Expected ';' after break";

			lexeme.lex_type = LEX_SEMICOLON;
			lexeme.value    = -1;
			poliz.push(lexeme);

			get_lexeme();
		break;

		case LEX_GOTO:
			get_lexeme();

			if (lex != LEX_IDENT)
				throw "Syntax error: Expected identificator";

			ident_pos = lex_list.get_value(pos);

			if (tid.get_type(ident_pos) != LEX_NULL && // Ссылка вперед?
				tid.get_type(ident_pos) != LEX_LABEL)
					throw "Syntax error: no such label";

			lexeme.lex_type = LEX_LABEL;

			if (tid.is_defined(ident_pos))
			{
				lexeme.value = tid.get_value(ident_pos);
				poliz.push(lexeme);
			}
			else
			{
				lexeme.value = -1;
				poliz.push(lexeme);
				tid.initialize(ident_pos, poliz.current() - 1);
			}

			lexeme.lex_type = POLIZ_GO;
			lexeme.value    = -1;
			poliz.push(lexeme);

			get_lexeme();

			if (lex != LEX_SEMICOLON)
				throw "Syntax error: Expected ;";

			lexeme.lex_type = LEX_SEMICOLON;
			lexeme.value    = -1;
			poliz.push(lexeme);

			get_lexeme();
		break;

		default:
			EXPRESSION();

			pop_op();

			if (lex != LEX_SEMICOLON)
				throw "Syntax error: Expected ;";

			lexeme.lex_type = LEX_SEMICOLON;
			lexeme.value    = -1;
			poliz.push(lexeme);

			get_lexeme();
		break;
	}
}

void Analyzer::EXPRESSION()
{
	LexType  op1, op2;
	uint assign_count = 0;

	EXP_OR();

	op1 = pop_op();

	while(lex == LEX_ASSIGN)
	{
		lexeme = poliz.pop();

		lexeme.lex_type = POLIZ_ADDRESS;
		poliz.push(lexeme);

		get_lexeme();

		++assign_count;

		EXP_OR();

		op2 = pop_op();

		check_assign(op1, op2);
	}

	lexeme.lex_type = LEX_ASSIGN;
	lexeme.value    = -1;

	for (uint i = 0; i < assign_count; ++i)
	{
		poliz.push(lexeme);
	}

	expression_type_stack.push_back(op1);
}

void Analyzer::EXP_OR()
{
	LexType op1, op2;

	EXP_AND();

	op1 = pop_op();

	while(lex == LEX_OR)
	{
		get_lexeme();

		EXP_AND();

		op2 = pop_op();

		check_logic(op1, op2);

		lexeme.lex_type = LEX_OR;
		lexeme.value    = -1;
		poliz.push(lexeme);
	}

	expression_type_stack.push_back(op1);
}

void Analyzer::EXP_AND()
{
	LexType op1, op2;

	EXP_CMP();

	op1 = pop_op();

	while(lex == LEX_AND)
	{
		get_lexeme();

		EXP_CMP();

		op2 = pop_op();

		check_logic(op1, op2);

		lexeme.lex_type = LEX_AND;
		lexeme.value    = -1;
		poliz.push(lexeme);
	}

	expression_type_stack.push_back(op1);
}

void Analyzer::EXP_CMP()
{
	LexType op1, op2;

	EXP_PLUS_MINUS();

	op1 = pop_op();

	if (is_cmp(lex))
	{
		do
		{
			lexeme.lex_type = lex;
			lexeme.value    = -1;

			get_lexeme();

			EXP_PLUS_MINUS();

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

void Analyzer::EXP_PLUS_MINUS()
{
	LexType op1, op2, op_type;

	EXP_MULT_DIV();

	op1 = pop_op();

	while(lex == LEX_PLUS || lex == LEX_MINUS)
	{
		op_type = lex;

		get_lexeme();

		EXP_MULT_DIV();

		op2 = pop_op();

		check_arithmetic(op1, op2, op_type);

		lexeme.lex_type = op_type;
		lexeme.value    = -1;
		poliz.push(lexeme);
	}

	expression_type_stack.push_back(op1);
}

void Analyzer::EXP_MULT_DIV()
{
	LexType op1, op2, op_type;

	EXP_NOT();

	op1 = pop_op();

	while(lex == LEX_MULT || lex == LEX_DIV)
	{
		op_type = lex;

		get_lexeme();

		EXP_NOT();

		op2 = pop_op();

		check_arithmetic(op1, op2, op_type);

		lexeme.lex_type = op_type;
		lexeme.value    = -1;
		poliz.push(lexeme);
	}

	expression_type_stack.push_back(op1);
}

void Analyzer::EXP_NOT()
{
	LexType op1;

	if (lex == LEX_NOT)
	{
		get_lexeme();

		EXP_UN_MINUS();

		op1 = pop_op();

		check_not(op1);

		lexeme.lex_type = LEX_NOT;
		lexeme.value    = -1;
		poliz.push(lexeme);

		expression_type_stack.push_back(op1);
	}
	else
		EXP_UN_MINUS();
}

void Analyzer::EXP_UN_MINUS()
{
	LexType op1;

	if (lex == LEX_MINUS)
	{
		get_lexeme();

		EXP_PAR();

		op1 = pop_op();

		check_minus(op1);

		lexeme.lex_type = LEX_UMIN;
		lexeme.value    = -1;
		poliz.push(lexeme);

		expression_type_stack.push_back(op1);
	}
	else
		EXP_PAR();
}

void Analyzer::EXP_PAR()
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
		get_lexeme();

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

			get_lexeme();

			if (lex == LEX_DOT)
			{
				get_lexeme();

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

				lexeme.lex_type = LEX_IDENT;
				lexeme.value    = tid_pos;
				poliz.push(lexeme);

				expression_type_stack.push_back(type);

				get_lexeme();
			}
			else
			{
				poliz.push(lex_list[pos - 1]);
				expression_type_stack.push_back(tid.get_type(lex_list.get_value(pos - 1)));
			}
		break;

		case LEX_RPAR:
			//Wow, such handy...
			get_lexeme();
		break;

		default:
			poliz.push(lex_list[pos]);

			expression_type_stack.push_back(lex);

			get_lexeme();
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
	return  lex == LEX_EQ   || lex == LEX_NEQ || lex == LEX_GTR ||
			lex == LEX_LSS  || lex == LEX_GEQ || lex == LEX_LEQ;
}
