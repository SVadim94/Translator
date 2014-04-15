#include "lex.h"

const LexType Parser::tw[]={
	LEX_PROGRAM, LEX_IF, LEX_ELSE, LEX_SWITCH, LEX_CASE, LEX_DEFAULT,
	LEX_FOR, LEX_WHILE, LEX_BREAK, LEX_GOTO, LEX_READ, LEX_WRITE,
	LEX_STRUCT,	LEX_INT, LEX_STRING, LEX_BOOL
};

const LexType Parser::td[]={
	LEX_LCRO, LEX_RCRO, LEX_LPAR, LEX_RPAR, LEX_COLON, LEX_SEMICOLON,
	LEX_COMMA, LEX_EQ, LEX_NEQ, LEX_LEQ, LEX_GEQ, LEX_LSS, LEX_GTR,
	LEX_PLUS, LEX_MINUS, LEX_DIV, LEX_MOD, LEX_MULT, LEX_ASSIGN
};

const char * Parser::TW[]={
	"program", "if", "else", "switch", "case", "default",
	"for", "while", "break", "goto", "read", "write",
	"struct", "int", "string", "bool"
};

const char * Parser::TD[]={
	"{", "}", "(", ")", ":", ";",
	",", "==", "!=", "<=", ">=", "<", ">",
	"+", "-", "/", "%", "*", "="
};

void TID::print() const
{
	for (int i=0; i<top; ++i)
		cout << '#' << i << ": " << table[i] << endl;
}

void LexList::print() const
{
	LexType tmp_lex;
	for (unsigned int i=0; i<list.size(); ++i)
	{
		tmp_lex=list[i].lex_type;
		if (tmp_lex <= LEX_NULL || tmp_lex >= LEX_LAST)
			throw "Error in list of lexeme! Lexeme out of range!";
		else
		if (tmp_lex >= LEX_PROGRAM && tmp_lex <= LEX_BOOL)
			cout << Parser :: TW[tmp_lex - LEX_PROGRAM] << " -> " << list[i].value << endl; //Убрать после дебага!
		else
		if (tmp_lex >= LEX_LCRO && tmp_lex <= LEX_ASSIGN)
			cout << Parser :: TD[tmp_lex - LEX_LCRO] << " -> " << list[i].value << endl;
		else
			switch (tmp_lex)
			{
			case LEX_IDENT:
				cout << "Identificator" << " -> " << list[i].value << endl;
			break;
			case LEX_NUM:
				cout << "Numeric constant" << " -> " << list[i].value << endl;
			break;
			case LEX_STR:
				cout << "String constant" << " -> " << list[i].value << endl;
			break;
			}
	}
}

void Parser::print() const
{
	tid.print();
    lex_list.print();
}

bool is_alpha(char c)
{
	return ((c>='a' && c<='z') || (c>='A' && c<='Z'));
}

bool is_num(char c)
{
	return (c>='0' && c<='9');
}

bool is_separator(char c)
{
	for (int i=0; i<SEP_TABLE_SIZE; ++i)
		if (c==Parser::TD[i][0])
			return true;
	return false;
}

void readString(string &str)
{
	char c;
	str.clear();
	while((c = getchar()) != '\"')
		str.push_back(c);
}

void readComment()
{
	char cur,prev;
	prev=getchar();
	if (prev!='*')
		throw "Expected '*' after '/' but got something else!";
	while((cur=getchar())!=EOF)
	{
		if (cur=='/' && prev=='*')
			break;
		prev=cur;
	}
	if (cur==EOF)
		throw "Unexpected end of file!";
}

int Parser::findTD(const string &str) const
{
	for (int i=0; i < SEP_TABLE_SIZE; ++i)
		if (str.compare(TD[i])==0)
			return i;
	return -1;
}

int Parser::findTW(const string &str) const
{
	for (int i=0; i < WORDS_TABLE_SIZE; ++i)
		if (str.compare(TW[i])==0)
			return i;
	return -1;
}

void Parser::start()
{
	int tmp;
	c = getchar();
	do
	{
		switch(mode)
		{
			case START:
				if (is_alpha(c))
				{
					mode = IDENT;
					ident.clear();
					ident.push_back(c);
				}
				else
				if (is_num(c))
				{
					mode=NUM;
					lex.lex_type = LEX_NUM;
					lex.value = c-'0';
				}
				else
				if (is_separator(c))
				{
					ident.clear();
					ident.push_back(c);
					if (c=='=' || c=='!' || c=='<' || c=='>')
					{
						mode = SEPARATOR;
					}
					else
					{
						lex.lex_type = td[findTD(ident)];
						lex.value = -1;
						lex_list.push(lex);
					}
				}
				else
				if (c=='/')
				{
					readComment();
				}
				else
				if (c=='\"')
				{
					readString(ident);
					lex.lex_type = LEX_STR;
					lex.value = tid.push(ident);
					lex_list.push(lex);
				}
				else
				if (c!=' ' && c!='\n' && c!='\t' && c!=EOF)
				{
					throw "Something went wrong!";
				}
				c = getchar();
			break;
			case IDENT:
				if (is_alpha(c) || is_num(c))
				{
					ident.push_back(c);
					c = getchar();
				}
				else
				{
					mode=START;
					tmp=findTW(ident);
					if (tmp==-1)
					{
						lex.lex_type = LEX_IDENT;
						lex.value = tid.push(ident);
					}
					else
					{
						lex.lex_type = tw[tmp];
						lex.value = -1;
					}
					lex_list.push(lex);
				}
			break;
			case NUM:
				if (is_alpha(c))
					throw "Wrong identificator!";
				else
				if (is_num(c))
				{
					lex.value = 10*lex.value + c - '0';
					c = getchar();
				}
				else
				{
					lex_list.push(lex);
					mode=START;
				}
			break;
			case SEPARATOR:
				if (c=='=')
				{
					ident.push_back(c);
					c = getchar();
				}
				if (ident.size()!=2 || ident[0]!='!')
				{
					tmp=findTD(ident);
					if (tmp==-1)
						throw "Incorrect sepator's combination!";
					lex.lex_type = td[tmp];
					lex.value = -1;
					lex_list.push(lex);
				}
				else
					throw "Expected '=' after '!' but got something else!";
				mode = START;
			break;
		}
	}
	while(mode!=STOP && c!=EOF);
}
