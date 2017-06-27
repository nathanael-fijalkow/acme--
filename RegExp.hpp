
#ifndef REGEXP_HPP
#define REGEXP_HPP

#include "Automata.hpp"
#include "Expressions.hpp"
#include <map>
#include <utility>
#include <list>

//#define VERBOSE_AUTOMATA_COMPUTATION 0 (commented, already in Automata.hpp)
#define LOG_COMPUTATION_TO_FILE 1


using namespace std;


struct RegExp
{
	int starheight; //store the starheight
	virtual ~RegExp() {}
	virtual void print() const=0;
	virtual RegExp* clone() const=0;
	virtual bool operator ==(const RegExp*) const;
	string flat;
};

struct LetterRegExp :RegExp
{
	LetterRegExp(char a);
	char letter;
	virtual void print() const;
	virtual RegExp* clone() const;
};

struct ConcatRegExp : RegExp
{
	ConcatRegExp(const RegExp *e1, const RegExp *e2);
	~ConcatRegExp();
	const RegExp *left;
	const RegExp *right;
	virtual void print() const;
	virtual RegExp* clone() const;
};

struct UnionRegExp : RegExp
{
	UnionRegExp(RegExp *e1, RegExp *e2);
	~UnionRegExp();
	RegExp *left;
	RegExp *right;
	virtual void print() const;
	virtual RegExp* clone() const;
};

struct StarRegExp : RegExp
{
	StarRegExp(const RegExp *e1);
	~StarRegExp();
	const RegExp *base;
	virtual void print() const;
	virtual RegExp* clone() const;
};

// Dynamic casts to test the type of a regular expression
const LetterRegExp * isLetter(const RegExp * expr);
const ConcatRegExp * isConcat(const RegExp * expr);
const UnionRegExp * isUnion(const RegExp * expr);
const StarRegExp * isStar(const RegExp *expr);


RegExp* Aut2RegExp(ClassicAut *Aut, list<uint> order);

list<ExtendedExpression*> Reg2Sharps(const RegExp *reg);

#endif
