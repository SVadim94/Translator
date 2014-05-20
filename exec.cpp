#include "exec.h"

#define get_lexeme() cur_lexeme = poliz[pos++]

void Executer :: start()
{
	uint tmp_str_allocated = 0;
	uint pos               = 0;
	int  tmp;
	int  lValue, rValue;

	string field_name;

	Lex  cur_lexeme;
	Lex  lexeme;
	Lex  op1, op2;

	while (pos < poliz.size())
	{
		get_lexeme();

		switch (cur_lexeme.lex_type)
		{
		case LEX_NUM:
		case LEX_STR:
		case POLIZ_ADDRESS:
		case POLIZ_LABEL:
			push(cur_lexeme);
		break;

		case LEX_IDENT:
			switch(tid.get_type(cur_lexeme.value))
			{
			case LEX_INT:
				lexeme.lex_type = LEX_NUM;
				lexeme.value    = tid.get_value(cur_lexeme.value);
				push(lexeme);
			break;

			case LEX_STRING:
				if (!tid.is_init(cur_lexeme.value))
					tid.initialize(cur_lexeme.value, tstr.push(""));

				lexeme.lex_type = LEX_STR;
				lexeme.value    = tid.get_value(cur_lexeme.value);
				push(lexeme);
			break;

			case LEX_BOOL:
				lexeme.lex_type = LEX_BOOL;
				lexeme.value    = tid.get_value(cur_lexeme.value);
				push(lexeme);
			break;

			case LEX_STRUCT:
				lexeme.lex_type = LEX_STRUCT;
				lexeme.value    = cur_lexeme.value;
				push(lexeme);
			break;

			default:
				cout << "WATCH LEX_IDENT" << endl;
			break;
			}
		break;

		case LEX_INT:
			lexeme.lex_type = LEX_NUM;
			lexeme.value    = tid.get_value(cur_lexeme.value);
			push(lexeme);
		break;

		case LEX_BOOL:
			lexeme.lex_type = LEX_BOOL;
			lexeme.value    = tid.get_value(cur_lexeme.value);
			push(lexeme);
		break;

		case LEX_TRUE:
			lexeme.lex_type = LEX_BOOL;
			lexeme.value    = 1;
			push(lexeme);
		break;

		case LEX_FALSE:
			lexeme.lex_type = LEX_BOOL;
			lexeme.value    = 0;
			push(lexeme);
		break;

		case LEX_STRING:
			if (!tid.is_init(cur_lexeme.value))
				tid.initialize(cur_lexeme.value, tstr.push(""));

			lexeme.lex_type = LEX_STR;
			lexeme.value    = tid.get_value(cur_lexeme.value);
			push(lexeme);
		break;

		case LEX_PLUS:
			op2 = pop();
			op1 = pop();

			if (op1.lex_type == LEX_NUM)
			{
				lexeme.lex_type = LEX_NUM;
				lexeme.value    = op1.value + op2.value;
				push(lexeme);
			}
			else
			{
				lexeme.lex_type = LEX_STR;
				lexeme.value    = tstr.push(tstr[op1.value]+tstr[op2.value]);
				push(lexeme);

				++tmp_str_allocated;
			}
		break;

		case LEX_MINUS:
			op2 = pop();
			op1 = pop();

			lexeme.lex_type = LEX_NUM;
			lexeme.value    = op1.value - op2.value;
			push(lexeme);
		break;

		case LEX_UMIN:
			op1       = pop();
			op1.value = - op1.value;
			push(op1);
		break;

		case LEX_MULT:
			op2 = pop();
			op1 = pop();

			lexeme.lex_type = LEX_NUM;
			lexeme.value    = op1.value * op2.value;
			push(lexeme);
		break;

		case LEX_DIV:
			op2 = pop();
			op1 = pop();

			lexeme.lex_type = LEX_NUM;
			lexeme.value    = op1.value / op2.value;
			push(lexeme);
		break;

		case LEX_AND:
			op2 = pop();
			op1 = pop();

			lexeme.lex_type = LEX_BOOL;
			lexeme.value    = op1.value && op2.value;
			push(lexeme);
		break;

		case LEX_OR:
			op2 = pop();
			op1 = pop();

			lexeme.lex_type = LEX_BOOL;
			lexeme.value    = op1.value || op2.value;
			push(lexeme);
		break;

		case LEX_NOT:
			op1 = pop();

			op1.value = !op1.value;
			push(op1);
		break;

		case LEX_READ:
			op1 = pop();

			if (tid.get_type(op1.value) == LEX_STRING)
				cin >> tstr[tid.get_value(op1.value)];
			else
			{
				cin >> tmp;
				tid.set_value(op1.value, tmp);
			}
		break;

		case LEX_WRITE:
			op1 = pop();

			for (vector<Lex> :: iterator elem_to_write = stack.end() - op1.value;
				elem_to_write < stack.end(); ++elem_to_write)
				{
					switch(elem_to_write->lex_type)
					{
					case LEX_NUM:
						cout << elem_to_write->value;
					break;

					case LEX_BOOL:
						cout << ((elem_to_write->value == 0) ? "false" : "true");
					break;

					case LEX_STR:
						cout << tstr[(elem_to_write->value)];
					break;

					default:
						cout << "WATCH WRITE" << endl;
					break;
					}
				}
				cout << endl;
		break;

		case LEX_EQ:
			op2 = pop();
			op1 = pop();

			lexeme.lex_type = LEX_BOOL;

			if (op1.lex_type == LEX_NUM)
				lexeme.value = (op1.value == op2.value);
			else
				lexeme.value = (tstr[op1.value] == tstr[op2.value]);

			push(lexeme);
		break;

		case LEX_NEQ:
			op2 = pop();
			op1 = pop();

			lexeme.lex_type = LEX_BOOL;

			if (op1.lex_type == LEX_NUM)
				lexeme.value = (op1.value != op2.value);
			else
				lexeme.value = (tstr[op1.value] != tstr[op2.value]);

			push(lexeme);
		break;

		case LEX_GTR:
			op2 = pop();
			op1 = pop();

			lexeme.lex_type = LEX_BOOL;

			if (op1.lex_type == LEX_NUM)
				lexeme.value = (op1.value > op2.value);
			else
				lexeme.value = (tstr[op1.value] > tstr[op2.value]);

			push(lexeme);
		break;

		case LEX_LSS:
			op2 = pop();
			op1 = pop();

			lexeme.lex_type = LEX_BOOL;

			if (op1.lex_type == LEX_NUM)
				lexeme.value = (op1.value < op2.value);
			else
				lexeme.value = (tstr[op1.value] < tstr[op2.value]);

			push(lexeme);
		break;

		case LEX_GEQ:
			op2 = pop();
			op1 = pop();

			lexeme.lex_type = LEX_BOOL;
			if (op1.lex_type == LEX_NUM)
				lexeme.value = (op1.value >= op2.value);
			else
				lexeme.value = (tstr[op1.value] >= tstr[op2.value]);

			push(lexeme);
		break;

		case LEX_LEQ:
			op2 = pop();
			op1 = pop();

			lexeme.lex_type = LEX_BOOL;

			if (op1.lex_type == LEX_NUM)
				lexeme.value = (op1.value <= op2.value);
			else
				lexeme.value = (tstr[op1.value] <= tstr[op2.value]);
			push(lexeme);
		break;

		case LEX_SWITCH:
			op1 = pop();

			switch_stack.push(op1.value);
		break;

		case LEX_CASE:
			op2 = pop();
			op1 = pop();

			if (op1.value != switch_stack.last())
				pos = op2.value;
		break;

		case LEX_BREAK:
			pos = cur_lexeme.value;
		break;

		case LEX_DEFAULT:
			switch_stack.pop();
		break;

		case POLIZ_GO:
			op1 = pop();

			pos = op1.value;
		break;

		case POLIZ_FGO:
			op2 = pop();
			op1 = pop();

			if (op1.value == 0)
				pos = op2.value;
		break;

		case LEX_ASSIGN:
			op2 = pop();
			op1 = pop();

			switch(op2.lex_type)
			{
			case LEX_NUM:
			case LEX_BOOL:
				tid.set_value(op1.value, op2.value);
			break;

			case LEX_STR:
				if (!tid.is_init(op1.value))
					tid.initialize(op1.value, tstr.push(""));

				tstr[tid.get_value(op1.value)].assign(tstr[op2.value]);
			break;

			case LEX_STRUCT:
				if (tid.get_value(op1.value) != tid.get_value(op2.value))
					throw "Syntax error: structure type mismatch";

				for (vector<FIELD> :: iterator iter = tstruct[tid.get_value(op2.value)].fields.begin();
					iter < tstruct[tid.get_value(op2.value)].fields.end(); ++iter)
					{
						field_name = tid.get_name(op1.value) + "." + iter->name;

						if ((lValue = tid.find(field_name)) == -1)
						{
							lValue = tid.push(field_name);
							tid.set_type(lValue, iter->type);

							if (iter->type == LEX_BOOL || iter->type == LEX_INT)
								tid.initialize(lValue, 0);
							else
								tid.initialize(lValue, tstr.push(""));
						}

						field_name = tid.get_name(op2.value) + "." + iter->name;

						if ((rValue = tid.find(field_name)) != -1)
						{
							if (tid.get_type(rValue) == LEX_BOOL || tid.get_type(rValue) == LEX_INT)
								tid.set_value(lValue, tid.get_value(rValue));
							else
								tstr[tid.get_value(lValue)].assign(tstr[tid.get_value(rValue)]);
						}
						else
						{
							if (tid.get_type(lValue) == LEX_BOOL || tid.get_type(lValue) == LEX_INT)
								tid.set_value(lValue, 0);
							else
								tstr[tid.get_value(lValue)].assign("");
						}
					}
			break;

			default:
				cout << "WATCH ASSIGN";
			break;
			}
			push(op2);
		break;

		case LEX_SEMICOLON:
			clear();

/*			while(tmp_str_allocated != 0)
			{
				tstr.pop();
				--tmp_str_allocated;
			}*/
		break;

		case LEX_LABEL:
			if (cur_lexeme.value == -1)
				throw "Runtime error: No such label!";

			lexeme.lex_type = POLIZ_LABEL;
			lexeme.value    = cur_lexeme.value;
			push(lexeme);
		break;

		default:
			cout << "UNEXPECTED LEXEME" << endl;
		break;
		}
	}
}

void Executer :: push(Lex lexeme)
{
	stack.push_back(lexeme);
}

Lex Executer :: pop()
{
	Lex tmp = stack.back();
	stack.pop_back();
	return tmp;
}

void Executer :: clear()
{
	stack.clear();
}
