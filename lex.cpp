#include "lex.h"

const LexType Parser::tw[]={
	LEX_PROGRAM, LEX_IF, LEX_ELSE, LEX_SWITCH, LEX_CASE, LEX_DEFAULT,
	LEX_FOR, LEX_WHILE, LEX_BREAK, LEX_GOTO, LEX_READ, LEX_WRITE,
	LEX_STRUCT, LEX_INT, LEX_STRING, LEX_NOT, LEX_OR, LEX_AND, LEX_TRUE,
	LEX_FALSE, LEX_BOOL
};

const LexType Parser::td[]={
	LEX_LCRO, LEX_RCRO, LEX_LPAR, LEX_RPAR, LEX_COLON, LEX_SEMICOLON,
	LEX_COMMA, LEX_EQ, LEX_NEQ, LEX_LEQ, LEX_GEQ, LEX_LSS, LEX_GTR,
	LEX_PLUS, LEX_MINUS, LEX_DIV, LEX_MOD, LEX_MULT, LEX_DOT, LEX_ASSIGN
};

const char * Parser::TW[]={
	"program", "if", "else", "switch", "case", "default",
	"for", "while", "break", "goto", "read", "write",
	"struct", "int", "string", "not", "or", "and", "true",
	"false", "bool"
};

const char * Parser::TD[]={
	"{", "}", "(", ")", ":", ";",
	",", "==", "!=", "<=", ">=", "<", ">",
	"+", "-", "/", "%", "*", ".", "="
};

inline bool is_alpha(char c)
{
	return ((c>='a' && c<='z') || (c>='A' && c<='Z'));
}

inline bool is_num(char c)
{
	return (c>='0' && c<='9');
}

bool is_separator(char c)
{
	for (unsigned int i=0; i<SEP_TABLE_SIZE; ++i)
		if (c==Parser::TD[i][0])
			return true;
	return false;
}

inline bool is_space(char c)
{
	return (c==' ' || c=='\n' || c=='\t' || c==EOF);
}

string print_lex(LexType lex)
{
	if (lex < LEX_NULL || lex >= LEX_LAST)
		throw "Bad print_lex parameter! Lexeme out of range!";

	if (lex == LEX_NULL)
		return "NULL";

	if (lex >= LEX_PROGRAM && lex <= LEX_BOOL)
		return Parser :: TW[lex - LEX_PROGRAM];

	if (lex >= LEX_LCRO && lex <= LEX_ASSIGN)
		return Parser :: TD[lex - LEX_LCRO];

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

	default:
		return "I think we need to change head-programmer...";
	break;
	}
}

void readString(string &str)
{
	char c;
	str.clear();

	while((c = getchar()) != '\"')
	{
		if (c == EOF)
			throw "Unexpected EOF! [2]";

		str.push_back(c);
	}
}

void readComment()
{
	char cur, prev;
	prev = getchar();

	while((cur = getchar()) != EOF)
	{
		if (cur == '/' && prev == '*')
			break;
		prev = cur;
	}
	if (cur == EOF)
		throw "Unexpected EOF!";
}

void TID::print() const
{
	for (unsigned int i=0; i < table.size(); ++i)
		cout << '#' << i << ": " << table.at(i).name << endl <<
		'\t' << "Defined: " << table.at(i).defined << endl <<
		'\t' << "Initialized: " << table.at(i).initialized << endl <<
		'\t' << "Type: " << print_lex(table.at(i).type) << endl <<
		'\t' << "Value: " << table.at(i).value << endl;
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

void TSTR::print() const
{
	for (unsigned int i=0; i < table.size(); ++i)
		cout << '#' << i << ": " << table[i] << endl;
}

void LexList::print() const
{
	for (unsigned int i=0; i<list.size(); ++i)
	{
		cout << print_lex(list[i].lex_type) << " -> " << list[i].value << endl;
	}
}

void Parser::print(TID &tid, TSTR &tstr, LexList &lex_list) const
{
	tid.print();
	cout << "---------\n";
	tstr.print();
	cout << "---------\n";
	lex_list.print();
	cout << "---------\n";
	for (unsigned int i = 0; i < pre_proc_list.size(); ++i)
		cout << pre_proc_list[i].from << "->" << pre_proc_list[i].to << endl;
}

int Parser::findTD(const string &str) const
{
	for (unsigned int i = 0; i < SEP_TABLE_SIZE; ++i)
		if (str.compare(TD[i]) == 0)
			return i;
	return -1;
}

int Parser::findTW(const string &str) const
{
	for (unsigned int i = 0; i < WORDS_TABLE_SIZE; ++i)
		if (str.compare(TW[i]) == 0)
			return i;
	return -1;
}

int Parser::findPP(const string &str) const
{
	for (unsigned int i=0; i < pre_proc_list.size(); ++i)
		if (pre_proc_list[i].from.compare(str) == 0)
			return i;
	return -1;
}

void Parser::start(TID &tid, TSTR &tstr, LexList &lex_list)
{
	int tmp;
	char c;
	Lex lex;
	Define def;
	IfElseStack stack;
	string ident;
	bool define_allowed = true;

	c = getchar();
	do
	{
		switch(mode)
		{
			case START:
				if (c=='#')
				{
					if (!define_allowed)
						throw "# must be first non-separator in the line!";

					ident.clear();

					c = getchar();

					if (!is_alpha(c))
						throw "Wrong directive!";

					while (true)
					{
						if (is_alpha(c))
							ident.push_back(c);
						else
						if (c == ' ' || c == '\t' || c == '\n')
							break;
						else
							throw "Wrong directive! [2]";

						c = getchar();
					}

					if (ident.compare("define") == 0)
					{
						def.from.clear();

						if (c == '\n')
							throw "Missing #define's first argument!";

						while((c = getchar()) == ' ' || c == '\t');

						if (!is_alpha(c))
							throw "First #define's argument must be identifier!";

						while (true)
						{
							if (is_alpha(c) || is_num(c))
								def.from.push_back(c);
							else
							if (c == ' ' || c == '\t')
								break;
							else
							if (c == '\n')
								throw "#define's second argument missing!";
							else
								throw "First #define's argument must be identifier [2]!";

							c = getchar();
						}

//                      if (findTW(def.from)!=-1) // Служенбное слово?
//                          throw "#define's first argument can't be a function word!";

						def.to = 0;

						while((c=getchar())==' ' || c == '\t');

						if (!is_num(c))
							throw "Second argument of #define must be numeric constant!";

						while(true)
						{
							if (is_num(c))
								def.to = 10 * def.to + c - '0'; // Переполнение?!
							else
							if (c==' ' || c=='\t')
							{
								while((c = getchar()) != '\n')
									if (c != ' ' || c != '\t')
										throw "Unexpected symbols after #define's second argument!";
								break;
							}
							else
							if (c == '\n')
								break;
							else
								throw "Second argument of #define must be numeric constant! [2]";

							c = getchar();
						}

						tmp = findPP(def.from);

						if (tmp != -1)
							pre_proc_list.erase(pre_proc_list.begin() + tmp);

						pre_proc_list.push_back(def);
					}
					else
					if (ident.compare("undef") == 0)
					{
						ident.clear();

						if (c == '\n')
							throw "Missing #define's first argument!";

						while((c = getchar()) == ' ' || c == '\t');

						if (!is_alpha(c))
							throw "#undef's argument must be identifier!";

						while (true)
						{
							if (is_alpha(c) || is_num(c))
								ident.push_back(c);
							else
							if (c == ' ' || c == '\t')
							{
								while((c = getchar()) != '\n')
									if (c != ' ' || c != '\t')
										throw "Unexpected symbols after #undef's argument!";
								break;
							}
							else
							if (c == '\n')
								break;
							else
								throw "#undef's argument must be identifier [2]!";

							c = getchar();
						}

						tmp = findPP(ident);

						if (tmp != -1)
							pre_proc_list.erase(pre_proc_list.begin() + tmp);
					}
					else
					if (ident.compare("ifdef") == 0)
					{
						ident.clear();

						if (c == '\n' || c == EOF)
							throw "Missing #ifdef's first argument!";

						while((c = getchar()) == ' ' || c == '\t');

						if (!is_alpha(c))
							throw "First #ifdef's argument must be identifier!";

						while (true)
						{
							if (is_alpha(c) || is_num(c))
								ident.push_back(c);
							else
							if (c == ' ' || c == '\t')
							{
								while((c = getchar()) != '\n')
									if (c != ' ' || c != '\t')
										throw "Unexpected symbols after #ifdef's first argument!";
								break;
							}
							else
							if (c == '\n')
								break;
							else
								throw "First #ifdef's argument must be identifier [2]!";

							c = getchar();
						}

						stack.push();

						tmp = findPP(ident);

						ident.clear();

						if (tmp == -1) //Проскакиваем все до endif'a или else'a
						{
							stack.ignore();

							while(true)
							{
								if (c != '\n')
									while((c = getchar()) != '\n')
										if (c == EOF)
											throw "Unexpected EOF! [3]";

								while ((c = getchar()) == ' ' || c == '\t')
									if (c == EOF)
										throw "Unexpected EOF! [4]";

								if (c == '#')
								{
									ident.clear();

									//while((c = getchar()) == ' ' || c=='\t');
									c = getchar();

									if (c == EOF)
										throw "Unexpected EOF! [5]";

									if (is_alpha(c))
									{
										while (true)
										{
											if (is_alpha(c))
												ident.push_back(c);
											else
											if (c == ' ' || c == '\t' || c == '\n')
												break;
											else
											if (c == EOF)
												throw "Unexpected EOF! [6]";

											c = getchar();
										}

										if (ident.compare("ifdef") == 0 || ident.compare("ifndef") == 0)
										{
											stack.push();
										}
										else
										if (ident.compare("endif") == 0)
										{
											if (c != '\n')
												while((c = getchar()) != '\n')
													if (c != ' ' && c != '\t')
														throw "Restricted symbol after endif!";

											if (stack.stopIgnore())
											{
												stack.pop();
												break;
											}

											stack.pop();
										}
										else
										if (ident.compare("else") == 0)
										{
											if (c != '\n')
												while((c = getchar()) != '\n')
													if (c != ' ' && c != '\t')
														throw "Restricted symbol after endif!";

											stack.check();

											if (stack.stopIgnore())
												break;
										}
									}
								}
							}
						}
					}
					else
					if (ident.compare("ifndef") == 0)
					{
						ident.clear();

						while((c = getchar()) == ' ' || c == '\t');

						if (!is_alpha(c))
							throw "First #ifndef's argument must be identifier!";

						while (true)
						{
							if (is_alpha(c) || is_num(c))
								ident.push_back(c);
							else
							if (c == ' ' || c == '\t')
							{
								while((c = getchar())!='\n')
									if (c != ' ' || c != '\t')
										throw "Unexpected symbols after #ifndef's first argument!";
								break;
							}
							else
							if (c == '\n')
								break;
							else
								throw "First #ifndef's argument must be identifier [2]!";

							c = getchar();
						}

						stack.push();

						tmp = findPP(ident);

						ident.clear();

						if (tmp != -1) //Проскакиваем все до endif'a или else'a
						{
							stack.ignore();

							while(true)
							{
								if (c != '\n')
									while((c = getchar()) != '\n')
										if (c == EOF)
											throw "Unexpected EOF! [3]";

								while ((c = getchar()) == ' ' || c == '\t')
									if (c == EOF)
										throw "Unexpected EOF! [4]";

								if (c == '#')
								{
									ident.clear();

									//while((c = getchar()) == ' ' || c=='\t');
									c = getchar();

									if (c == EOF)
										throw "Unexpected EOF! [5]";

									if (is_alpha(c))
									{
										while (true)
										{
											if (is_alpha(c))
												ident.push_back(c);
											else
											if (c == ' ' || c == '\t' || c == '\n')
												break;
											else
											if (c == EOF)
												throw "Unexpected EOF! [6]";

											c = getchar();
										}

										if (ident.compare("ifdef") == 0 || ident.compare("ifndef") == 0)
											stack.push();
										else
										if (ident.compare("endif") == 0)
										{
											if (c != '\n')
												while((c = getchar()) != '\n')
													if (c != ' ' && c != '\t')
														throw "Restricted symbol after endif!";

											if (stack.stopIgnore())
											{
												stack.pop();
												break;
											}

											stack.pop();
										}
										else
										if (ident.compare("else") == 0)
										{
											if (c != '\n')
												while((c = getchar()) != '\n')
													if (c != ' ' && c != '\t')
														throw "Restricted symbol after endif!";

											stack.check();

											if (stack.stopIgnore())
												break;
										}
									}
								}
							}
						}
					}
					else
					if (ident.compare("else") == 0)
					{
						stack.check();

						if ( c != '\n')
							while((c = getchar()) != '\n')
								if (c != ' ' && c != '\t')
									throw "Unexpected symbols after #else";

						stack.ignore();

						while(true)
						{
							if (c != '\n')
								while((c = getchar()) != '\n')
									if (c == EOF)
										throw "Unexpected EOF! [3]";

							while ((c = getchar()) == ' ' || c == '\t')
								if (c == EOF)
									throw "Unexpected EOF! [4]";

							if (c == '#')
							{
								ident.clear();

								//while((c = getchar()) == ' ' || c=='\t');
								c = getchar();

								if (c == EOF)
									throw "Unexpected EOF! [5]";

								if (is_alpha(c))
								{
									while (true)
									{
										if (is_alpha(c))
											ident.push_back(c);
										else
										if (c == ' ' || c == '\t' || c == '\n')
											break;
										else
										if (c == EOF)
											throw "Unexpected EOF! [6]";

										c = getchar();
									}

									if (ident.compare("ifdef") == 0 || ident.compare("ifndef") == 0)
										stack.push();
									else
									if (ident.compare("endif") == 0)
									{
										if (c != '\n')
											while((c = getchar()) != '\n')
												if (c != ' ' && c != '\t')
													throw "Restricted symbol after endif!";

										if (stack.stopIgnore())
										{
											stack.pop();
											break;
										}

										stack.pop();
									}
									else
									if (ident.compare("else") == 0)
									{
										if (c != '\n')
											while((c = getchar()) != '\n')
												if (c != ' ' && c != '\t')
													throw "Restricted symbol after endif!";

										stack.check();
									}
								}
							}
						}
					}
					else
					if (ident.compare("endif") == 0)
					{
						if (c != '\n')
							while((c = getchar()) != '\n')
								if (c != ' ' && c != '\t')
									throw "Restricted symbol after endif!";

						stack.pop();
					}
					else
						throw "Wrong directive [3]!";
				}
				else
				if (is_alpha(c)) //Побочный эффект
				{
					mode = IDENT;
					ident.clear();
					ident.push_back(c);
					define_allowed = false;
				}
				else
				if (is_num(c))
				{
					mode = NUM;
					lex.lex_type = LEX_NUM;
					lex.value = c - '0';
					define_allowed = false;
				}
				else
				if (is_separator(c))
				{
					ident.clear();
					ident.push_back(c);
					if (c == '=' || c == '!' || c == '<' || c == '>' || c == '/')
					{
						mode = SEPARATOR;
					}
					else
					{
						lex.lex_type = td[findTD(ident)];
						lex.value = -1;
						lex_list.push(lex);
					}

					define_allowed = false;
				}
				else
				if (c == '\"')
				{
					readString(ident);

					lex.lex_type = LEX_STR;

					tmp = tstr.find(ident);

					if (tmp==-1)
						lex.value = tstr.push(ident);
					else
						lex.value = tmp;

					lex_list.push(lex);

					define_allowed = false;
				}
				else
				if (!is_space(c))
					throw "Something went wrong / Restricted symbol!";
				else
				if (c == '\n')
					define_allowed = true;

				c = getchar();
			break;
			case IDENT:
				if (is_alpha(c) || is_num(c))
				{
					ident.push_back(c);
					c = getchar();
				}
				else
				if (is_separator(c) || is_space(c))
				{
					mode = START;

					tmp = findPP(ident);

					if (tmp != -1)
					{
						lex.lex_type = LEX_NUM;
						lex.value = pre_proc_list[tmp].to;
						lex_list.push(lex);
					}
					else
					{
						tmp = findTW(ident);

						if (tmp == -1)
						{
							lex.lex_type = LEX_IDENT;
							tmp = tid.find(ident);
							if (tmp == -1)
								lex.value = tid.push(ident);
							else
								lex.value = tmp;
						}
						else
						{
							lex.lex_type = tw[tmp];
							lex.value = -1;
						}

						lex_list.push(lex);
					}
				}
				else
					throw "Wrong identificator!";
			break;
			case NUM:
				if (is_alpha(c))
					throw "Wrong identificator!";
				else
				if (is_num(c))
				{
					lex.value = 10 * lex.value + c - '0';
					c = getchar();
				}
				else
				if (is_separator(c) || is_space(c))
				{
					lex_list.push(lex);
					mode = START;
				}
				else
					throw "An error occured while reading number!";
			break;
			case SEPARATOR:
				if (c=='*')
				{
					readComment();
					c = getchar();
				}
				else
				{
					if (c == '=')
					{
						ident.push_back(c);
						c = getchar();
					}
					if (ident.size() == 2 || ident[0] != '!')
					{
						tmp = findTD(ident);
						if (tmp == -1)
							throw "Incorrect sepator's combination!";
						lex.lex_type = td[tmp];
						lex.value = -1;
						lex_list.push(lex);
					}
					else
						throw "Expected '=' after '!' but got something else!";
				}

				mode = START;
			break;
		}
	}
	while(c != EOF);

	if (!stack.isEmpty())
		throw "Unexpected EOF! [8]";
}
