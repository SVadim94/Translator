//Оно мне надо?!
#ifndef SEP_TABLE_SIZE
#include "lex.h"
#endif

class TSTRUCT
{
	struct FIELD
	{
		string name;
		LexType type;
		FIELD() {}
		FIELD(const string &str, LexType lex) : name(str), type(lex) {}
	};

	struct STRUCT
	{
		string name;
		vector<FIELD> fields;
		STRUCT() {}
		explicit STRUCT(const string &str) : name(str) {}
	};

	vector<STRUCT> table;
public:
	int push(const string &str) {STRUCT tmp(str); table.push_back(tmp); return table.size() - 1;}
	void push_field(const string &str, LexType lex) {FIELD tmp(str, lex); table.back().fields.push_back(tmp);}
	int find(int, const string &) const;
	void print() const;
	//...
};

class Analyzer
{
	LexType lex;
	LexType block_type; //Уродливый, Б-гомързкий костыль!
	int pos;
	TID &tid;
	TSTR &tstr;
	LexList &lex_list;
	TSTRUCT &tstruct;
	vector<LexType> expression_type_stack;

	//Метод рекурсивного спуска
	void PROGRAM();
	void STRUCTURES();
	void STRUCTURE();
	void SDESCRIPTION();
	void DESCRIPTIONS();
	void DESCRIPTION();
	void TYPE();
	void VARIABLE();
	void CONSTANT();
	void STRING();
	void BOOL();
	void INTEGER();
	void OPERATORS();
	void OPERATOR();
	void EXPRESSION();
	void E1();
	void E2();
	void E3();
	void E4();
	void E5();
	void E6();
	void E7();
	void E8();
	//...
public:
	Analyzer(TID &tid, TSTR &tstr, LexList &lex_list, TSTRUCT &tstruct) :
		pos(-1), tid(tid), tstr(tstr), lex_list(lex_list), tstruct(tstruct) {}
	void start();
};

inline void check_assign(LexType, LexType);
inline void check_logic(LexType, LexType);
inline void check_comparison(LexType, LexType);
inline void check_arithmetic(LexType, LexType, LexType);
inline void check_not(LexType);
inline void check_minus(LexType);
inline bool is_cmp(LexType);