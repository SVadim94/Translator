#include "syn.h"
#define get_lex()   lex = lex_list[++pos].lex_type;
#define step_back() lex_list.get_lex(--pos) //Разберись с этим
#define pop_op()    expression_type_stack.back(); expression_type_stack.pop_back();
#define uint        unsigned int

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
	for (uint j = 0; j < str_no_i->fields.size(); ++j)
		if (str.compare(str_no_i->fields[j].name) == 0)
			return j;
	return -1;
}

void TSTRUCT::print() const
{
	for (uint i = 0; i < table.size(); ++i)
	{
		cout << table[i].name << endl;
		for (uint j = 0; j < table[i].fields.size(); ++j)
			cout << '\t' << table[i].fields[j].name << " : " << print_lex(table[i].fields[j].type) << endl;
	}
}

void POLIZ::print() const
{
	for (uint i = 0; i < poliz.size(); ++i)
		cout << '#' << i << ": " << print_lex(poliz[i].lex_type) << " : " << poliz[i].value << endl;
}

void Analyzer::start()
{
	get_lex();

	PROGRAM();
}

void Analyzer::PROGRAM()
{
	if (lex == LEX_PROGRAM)
	{
		get_lex();

		if (lex != LEX_LCRO)
			throw "Syntax error: Expected {";

		get_lex();

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
	get_lex();

	STRUCTURE();

	while(lex == LEX_STRUCT)
	{
		get_lex();

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

	get_lex();

	if (lex != LEX_LCRO)
		throw "Syntax error: Expected {";

	get_lex();

	SDESCRIPTION();

	if (lex != LEX_SEMICOLON)
		throw "Syntax error: Expected ;";

	get_lex();

	while (lex == LEX_INT || lex == LEX_BOOL || lex == LEX_STRING)
	{
		SDESCRIPTION();

		if (lex != LEX_SEMICOLON)
			throw "Syntax error: Expected ;";

		get_lex();
	}

	if (lex != LEX_RCRO)
		throw "Syntax error: Expected }";

	get_lex();

	if (lex != LEX_SEMICOLON)
		throw "Syntax error: Expected ;";

	get_lex();
}

void Analyzer::SDESCRIPTION()
{
	int ident_pos;

	get_lex();

	if (lex != LEX_IDENT)
		throw "Syntax error: Expected identificator";

	ident_pos = lex_list.get_value(pos);

	tid.set_type(ident_pos, LEX_LAST); //Носит флаговый характер

	tstruct.push_field(tid.get_name(ident_pos), lex_list.get_lex(pos - 1));

	get_lex();
}

void Analyzer::DESCRIPTIONS()
{
	while(lex == LEX_INT || lex == LEX_BOOL || lex == LEX_STRING || lex == LEX_STRUCT)
	{
		DESCRIPTION();

		if (lex != LEX_SEMICOLON)
			throw "Syntax error: Expected ;";

		get_lex();
	}
}

void Analyzer::DESCRIPTION()
{
	block_type = lex;

	TYPE();
	VARIABLE();

	while (lex == LEX_COMMA)
	{
		get_lex();

		VARIABLE();
	}
}

void Analyzer::TYPE()
{
	if (lex != LEX_INT && lex != LEX_STRING && lex != LEX_BOOL && lex != LEX_STRUCT)
		throw "Syntax error: Expected type name";

	if (lex == LEX_STRUCT)
	{
		get_lex();

		if (lex != LEX_IDENT)
			throw "Syntax error: Expected identificator";

		struct_block_type = lex_list.get_value(pos);
	}

	get_lex();
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

	get_lex();

	if (lex == LEX_ASSIGN && block_type == LEX_STRUCT)
		throw "Syntax error: initialization of structures is not allowed";

	if (lex == LEX_ASSIGN)
	{
		get_lex();

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
			get_lex();

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

	get_lex();
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

	get_lex();
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

	get_lex();
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

	//label_index_i
	uint lab_i_1 = 0, lab_i_2 = 0;
	uint lab_i_3 = 0, lab_i_4 = 0;

	//target_index_i
	uint tar_i_1 = 0, tar_i_2 = 0;
	uint tar_i_3 = 0, tar_i_4 = 0;

	int ident_pos, write_argc = 0;

	switch(lex)
	{
		case LEX_IF:
			get_lex();

			if (lex != LEX_LPAR)
				throw "Syntax error: Expected (";

			get_lex();

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

			get_lex();

			OPERATOR();

			if (lex != LEX_ELSE)
				throw "Syntax error: Expected 'else'";

			lexeme.lex_type = POLIZ_LABEL;
			lexeme.value    = -1;
			lab_i_2         = poliz.push(lexeme);

			lexeme.lex_type = POLIZ_GO;
			lexeme.value    = -1;

			poliz.push(lexeme);

			get_lex();

			OPERATOR();

			tar_i_1                    = poliz.current();
			poliz[lab_i_1].value       = tar_i_1; // Теперь ссылки указывают на ';'
			poliz[lab_i_2].value       = tar_i_1; // Обе ссылки...

			lexeme.lex_type = LEX_SEMICOLON;
			lexeme.value    = -1;
			poliz.push(lexeme);
		break;

		case LEX_WHILE:
			break_stack.forbid();

			get_lex();

			if (lex != LEX_LPAR)
				throw "Syntax error: Expected (";

			get_lex();

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

			get_lex();

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
			get_lex();

			if (lex != LEX_LPAR)
				throw "Syntax error: Expected (";

			get_lex();

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

			get_lex();

			if (lex != LEX_RPAR)
				throw "Syntax error: Expected )";

			lexeme.lex_type = LEX_READ;
			lexeme.value    = -1;

			poliz.push(lexeme);

			get_lex();

			if (lex != LEX_SEMICOLON)
				throw "Syntax error: Expected ;";

			lexeme.lex_type = LEX_SEMICOLON;
			lexeme.value    = -1;

			poliz.push(lexeme);

			get_lex();
		break;

		case LEX_WRITE:
			get_lex();

			if (lex != LEX_LPAR)
				throw "Syntax error: Expected (";

			get_lex();

			EXPRESSION();

			pop_op();

			++write_argc;

			while(lex == LEX_COMMA)
			{
				get_lex();

				EXPRESSION();

				pop_op();

				++write_argc;
			}

			if (lex != LEX_RPAR)
				throw "Syntax error: Expected )";

			get_lex();

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

			get_lex();
		break;

		case LEX_SWITCH:
/*Используется стек меток, наверное...
Короче, время покажет...
*/
			get_lex();

			if (lex != LEX_LPAR)
				throw "Syntax error: Expected (";

			get_lex();

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

			get_lex();

			if (lex != LEX_LCRO)
				throw "Syntax error: Expected {";

			get_lex();

			if (lex != LEX_CASE)
				throw "Syntax error: Expected 'case'";

			get_lex();

			if (lex == LEX_MINUS)
			{
				get_lex();

				if (lex != LEX_NUM)
					throw "Syntax error: Expected numeric constant in switch";

				lexeme.lex_type = LEX_NUM;
				lexeme.value    = - lex_list[pos].value;
				poliz.push(lexeme);
			}
			else
			if (lex == LEX_PLUS)
			{
				get_lex();

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

			get_lex();

			if (lex != LEX_COLON)
				throw "Syntax error: Expected :";

			get_lex();

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
				get_lex();

				lexeme.lex_type = POLIZ_LABEL;
				lexeme.value    = poliz.current() + 5;
				poliz.push(lexeme);

				lexeme.lex_type = POLIZ_GO;
				lexeme.value    = -1;
				poliz.push(lexeme);

				if (lex == LEX_MINUS)
				{
					get_lex();

					if (lex != LEX_NUM)
						throw "Syntax error: Expected numeric constant in switch";

					lexeme.lex_type = LEX_NUM;
					lexeme.value    = - lex_list[pos].value;
					poliz.push(lexeme);
				}
				else
				if (lex == LEX_PLUS)
				{
					get_lex();

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

				get_lex();

				if (lex != LEX_COLON)
					throw "Syntax error: Expected :";

				get_lex();

				poliz[lab_i_1].value = poliz.current() + 1;

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
				get_lex();

				if (lex != LEX_COLON)
					throw "Syntax error: Expected :";

				get_lex();

				while(oper_begin(lex))
				{
					OPERATOR();
				}
			}

			if (lex != LEX_RCRO)
				throw "Syntax error: Expected }";

			lexeme.lex_type = LEX_DEFAULT;
			lexeme.value    = -1;
			tar_i_1         = poliz.push(lexeme);

			while((lab_i_1 = break_stack.pop()) != -1)
				poliz[lab_i_1].value = tar_i_1;
		break;

		case LEX_LCRO:
			get_lex();

			OPERATORS();

			if (lex != LEX_RCRO)
				throw "Syntax error: Expected }";

			get_lex();
		break;

		case LEX_IDENT:
			get_lex();

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

				get_lex();

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

				get_lex();
			}
		break;

		case LEX_FOR:
			break_stack.forbid();

			get_lex();

			if (lex != LEX_LPAR)
				throw "Syntax error: Expected (";

			get_lex();

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

			get_lex();

			if (lex != LEX_SEMICOLON)
			{
				EXPRESSION();

				op = pop_op();

				if (op != LEX_BOOL && op != LEX_TRUE && op != LEX_FALSE)
					throw "Semantic error: Expected boolean as 'for' second argument";

				if (lex != LEX_SEMICOLON)
					throw "Syntax error: Expected ;";
			}

			lexeme.lex_type = LEX_SEMICOLON;
			lexeme.value    = -1;
			poliz.push(lexeme);

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

			get_lex();

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

			get_lex();

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
			tar_i_1	        = poliz.push(lexeme);

			poliz[lab_i_1].value = tar_i_1;
			poliz[lab_i_2].value = tar_i_2;
			poliz[lab_i_3].value = tar_i_3;
			poliz[lab_i_4].value = tar_i_4;

			while((lab_i_1 = break_stack.pop()) != -1)
				poliz[lab_i_1].value = tar_i_1;
		break;

		case LEX_BREAK:
			//Something poliz
			get_lex();

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

			get_lex();
		break;

		case LEX_GOTO:
			get_lex();

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

			lexeme.lex_type = LEX_GOTO;
			lexeme.value    = -1;
			poliz.push(lexeme);

			get_lex();

			if (lex != LEX_SEMICOLON)
				throw "Syntax error: Expected ;";

			lexeme.lex_type = LEX_SEMICOLON;
			lexeme.value    = -1;
			poliz.push(lexeme);

			get_lex();
		break;

		default:
			EXPRESSION();

			pop_op();

			if (lex != LEX_SEMICOLON)
				throw "Syntax error: Expected ;";

			lexeme.lex_type = LEX_SEMICOLON;
			lexeme.value    = -1;
			poliz.push(lexeme);

			get_lex();
		break;
	}
}

void Analyzer::EXPRESSION()
{
	LexType  op1, op2;
	uint assign_count = 0;

	E1();

	op1 = pop_op();

	while(lex == LEX_ASSIGN)
	{
		poliz.pop();

		lexeme.lex_type = POLIZ_ADDRESS;
		lexeme.value    = lex_list.get_value(pos - 1); //Номер в ТИД

		poliz.push(lexeme);

		get_lex();

		++assign_count;

		E1();

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

void Analyzer::E1()
{
	LexType op1, op2;

	E2();

	op1 = pop_op();

	while(lex == LEX_OR)
	{
		get_lex();

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
		get_lex();

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

			get_lex();

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

		get_lex();

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

		get_lex();

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
		get_lex();

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
		get_lex();

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
		get_lex();

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

			get_lex();

			if (lex == LEX_DOT)
			{
				get_lex();

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

				get_lex();
			}
			else
			{
				poliz.push(lex_list[pos - 1]);
				expression_type_stack.push_back(tid.get_type(lex_list.get_value(pos - 1)));
			}
		break;

		case LEX_RPAR:
			//Wow, such handy...
			get_lex();
		break;

		default:
			poliz.push(lex_list[pos]);

			expression_type_stack.push_back(lex);

			get_lex();
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
