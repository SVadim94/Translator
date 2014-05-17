#include "tables.h"

#define SEP_TABLE_SIZE   sizeof(t_td) / sizeof(int)
#define WORDS_TABLE_SIZE sizeof(t_tw) / sizeof(int)


const LexType t_tw[] = {
	LEX_PROGRAM, LEX_IF,    LEX_ELSE,   LEX_SWITCH, LEX_CASE, LEX_DEFAULT,
	LEX_FOR,     LEX_WHILE, LEX_BREAK,  LEX_GOTO,   LEX_READ, LEX_WRITE,
	LEX_STRUCT,	 LEX_INT,   LEX_STRING, LEX_NOT,    LEX_OR,   LEX_AND,
	LEX_LABEL,   LEX_TRUE,	LEX_FALSE,  LEX_BOOL
};

const LexType t_td[] = {
	LEX_LCRO,  LEX_RCRO, LEX_LPAR, LEX_RPAR, LEX_COLON, LEX_SEMICOLON, LEX_COMMA,
	LEX_EQ,    LEX_NEQ,  LEX_LEQ,  LEX_GEQ,  LEX_LSS,   LEX_GTR,       LEX_PLUS,
	LEX_MINUS, LEX_UMIN, LEX_DIV,  LEX_MULT, LEX_DOT,  LEX_ASSIGN
};

const char *t_TW[] = {
	"program", "if", "else", "switch", "case", "default",
	"for", "while", "break", "goto", "read", "write",
	"struct", "int", "string", "not", "or", "and", "label",
	"true",	"false", "bool"
};

const char *t_TD[] = {
	"{", "}", "(", ")", ":", ";",
	",", "==", "!=", "<=", ">=", "<", ">",
	"+", "-", "-", "/", "*", ".", "="
};

void TID::print(ofstream &fout) const
{
	for (unsigned int i=0; i < table.size(); ++i)
		fout << '#' << i << ": " << table.at(i).name            << endl <<
		'\t' << "Defined: "      << table.at(i).defined         << endl <<
		'\t' << "Initialized: "  << table.at(i).initialized     << endl <<
		'\t' << "Type: "         << print_lex(table.at(i).type) << endl <<
		'\t' << "Value: "        << table.at(i).value           << endl;
}

int TID::find(const string &str) const
{
	for (unsigned int i = 0; i < table.size(); ++i)
	{
		if (str.compare(table.at(i).name) == 0)
			return i;
	}
	return -1;
}

int TSTR::find(const string &str) const
{
	for (unsigned int i = 0; i < table.size(); ++i)
	{
		if (str.compare(table[i]) == 0)
			return i;
	}
	return -1;
}

void TSTR::print(ofstream &fout) const
{
	for (unsigned int i=0; i < table.size(); ++i)
		fout << '#' << i << ": " << table[i] << endl;
}

void LexList::print(ofstream &fout) const
{
	for (unsigned int i=0; i<list.size(); ++i)
	{
		fout << print_lex(list[i].lex_type) << " -> " << list[i].value << endl;
	}
}

string print_lex(LexType lex)
{
	if (lex < LEX_NULL || lex >= LEX_LAST)
		throw "Bad print_lex parameter! Lexeme out of range!";

	if (lex == LEX_NULL)
		return "NULL";

	if (lex >= LEX_PROGRAM && lex <= LEX_BOOL)
		return t_TW[lex - LEX_PROGRAM];

	if (lex >= LEX_LCRO && lex <= LEX_ASSIGN)
		return t_TD[lex - LEX_LCRO];

	switch (lex)
	{
	case LEX_IDENT:
		return "Identificator";
	break;

	case LEX_NUM:
		return "Numeric constant";
	break;

	case LEX_STR:
		return "String constant";
	break;

	case POLIZ_ADDRESS:
		return "POLIZ ADDRESS";
	break;

	case POLIZ_FGO:
		return "POLIZ FGO";
	break;

	case POLIZ_GO:
		return "POLIZ GO";
	break;

	case POLIZ_LABEL:
		return "POLIZ LABEL";
	break;

	default:
		return "I think we need to change head-programmer...";
	break;
	}
}

int TSTRUCT::find(int i, const string &str) const
{
	const STRUCT *str_no_i = &(table.at(i));
	for (uint j = 0; j < str_no_i->fields.size(); ++j)
		if (str.compare(str_no_i->fields[j].name) == 0)
			return j;
	return -1;
}

void TSTRUCT::print(ofstream &fout) const
{
	for (uint i = 0; i < table.size(); ++i)
	{
		fout << table[i].name << endl;
		for (uint j = 0; j < table[i].fields.size(); ++j)
			fout << '\t' << table[i].fields[j].name << " : " << print_lex(table[i].fields[j].type) << endl;
	}
}

void POLIZ::print(ofstream &fout) const
{
	for (uint i = 0; i < table.size(); ++i)
		fout << '#' << i << ": " << print_lex(table[i].lex_type) << " : " << table[i].value << endl;
}
