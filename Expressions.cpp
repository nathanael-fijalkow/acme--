
#include <string.h>
#include <iostream>
#include <typeinfo>

#include "Expressions.hpp"

// Constructors
LetterExpr::LetterExpr(char letter) : letter(letter)
{
	_hash = std::hash_value(letter);
	sharp_height=0;
};

// Second constructor: a copy constructor which performs a memcopy of the field sons
ConcatExpr::ConcatExpr(const ConcatExpr & other) : sonsNb(other.sonsNb)
{
	_hash = other._hash;
	sons = (const ExtendedExpression **)malloc(sonsNb * sizeof(void *));
	memcpy(sons,other.sons,sonsNb * sizeof(void *));
	char m=0;
	for(int i=0;i<sonsNb;i++){
		if (sons[i]->sharp_height>m) m=sons[i]->sharp_height;
	}
	sharp_height=m;
}

// Concatenation constructor: creates the concatenation of two expressions
ConcatExpr::ConcatExpr(const ExtendedExpression * expr_left, const ExtendedExpression * expr_right)
{
	// Casts
	const ConcatExpr * ConcatExprLeft = isConcatExpr(expr_left);
	const ConcatExpr * ConcatExprRight = isConcatExpr(expr_right);

	int subtrees_nb_left = (ConcatExprLeft != NULL) ? ConcatExprLeft->sonsNb : 1;
	sonsNb = subtrees_nb_left + ((ConcatExprRight != NULL) ? ConcatExprRight->sonsNb : 1);

	/* temporary array used to create all infixes */
	sons = (const ExtendedExpression **)malloc(sonsNb * sizeof(void *));

	/* copy the expressions in the array*/
	if (ConcatExprLeft != NULL)
		memcpy(sons, ConcatExprLeft->sons, ConcatExprLeft->sonsNb * sizeof(void*));
	else
		sons[0] = expr_left;

	if (ConcatExprRight != NULL)
		memcpy(sons + subtrees_nb_left, ConcatExprRight->sons, ConcatExprRight->sonsNb * sizeof(void*));
	else
		sons[subtrees_nb_left] = expr_right;
	
	if (expr_left->sharp_height > expr_right->sharp_height)
		sharp_height=expr_left->sharp_height;
	else
		sharp_height=expr_right->sharp_height;
	update_hash();
}

// This is an assignment operator which performs a memcopy of the field sons
ConcatExpr & ConcatExpr::operator=(const ConcatExpr & other)
{
	if (this != &other)
	{
		sonsNb = other.sonsNb;
		_hash = other._hash;
		sons = (const ExtendedExpression **)malloc(sonsNb * sizeof(void *));
		memcpy(sons, other.sons, sonsNb * sizeof(void *));
		sharp_height=other.sharp_height;
	}	
	return *this;
}

// Adds a son to the left
// Assumes that enough memory was allocated when creating the son's pointer
/*
void ConcatExpr::addLeftSon(const ExtendedExpression * new_son)
{
	*(sons + sonsNb) = new_son;
	sonsNb++;
	//_hash ^= hash_value(new_son->Hash()) + 0x9e3779b9 + (_hash << 6) + (_hash >> 2);
	update_hash();
}
*/

SharpedExpr::SharpedExpr(const ExtendedExpression * son) : son(son)
{
	_hash = std::hash_value(son->Hash());
	sharp_height=1+son->sharp_height;
}




// Equality operator

bool ExtendedExpression::operator == (const ExtendedExpression & exp) const
{
	const ExtendedExpression * pexp = &exp;
	if (typeid(this) != typeid(pexp))
		return false;
	const SharpedExpr * sexpr = isSharpedExpr(this);
	if (sexpr != NULL)
		return (*sexpr == *(SharpedExpr *)pexp);
	else
	{
		const ConcatExpr * cexpr = isConcatExpr(this);
		if (cexpr != NULL)
			return (*cexpr == *(ConcatExpr *)pexp);
		else
			return *(LetterExpr *)this == *(LetterExpr *)pexp;
	}
}

std::ostream& operator<<(std::ostream& os, const ExtendedExpression & expr){ expr.print(os); return os; };


// Dynamic casts
const SharpedExpr * isSharpedExpr(const ExtendedExpression * expr) { return dynamic_cast<const SharpedExpr *>(expr); }
const ConcatExpr * isConcatExpr(const ExtendedExpression * expr) { return dynamic_cast<const ConcatExpr *>(expr); }
const LetterExpr * isLetterExpr(const ExtendedExpression * expr){ return dynamic_cast<const LetterExpr *>(expr); }


//Sharp-height
/*
char LetterExpr::sharp_height() const
{
	return 0;
}

char ConcatExpr::sharp_height() const
{
	char maxs=0, temp;
	for (uint i = sonsNb ; i > 0; i--){
		temp=sons[i -1]->sharp_height();
		if (temp>maxs) maxs=temp;
	}
	return maxs;
}

char SharpedExpr::sharp_height() const
{
	return 1+son->sharp_height();
}
*/
// Printing functions
using namespace std;

void LetterExpr::print(std::ostream& os) const
{
  if(letter < 'a')
	os << (char)('a' + letter);
  else
	os << (char) letter;
    
}

void ConcatExpr::print(std::ostream& os) const
{
	//was reversed, put the normal order again
	for (uint i = 0 ; i <sonsNb; i++)
		sons[i]->print(os);
}

void SharpedExpr::print(std::ostream& os) const
{
	if (isLetterExpr(son))
	{
		son->print(os);
		os << "#";
	}
	else
	{
		os << "(";
		son->print(os);
		os << ")^#";
	}
}

// Free the memory
ConcatExpr::~ConcatExpr()
{
	free(sons);
	sons = NULL;
}
