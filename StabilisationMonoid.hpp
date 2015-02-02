/* INCLUDES */
#ifndef STAB_MONOID_HPP
#define STAB_MONOID_HPP

#include "Monoid.hpp"
#include "MultiCounterMatrix.hpp"


class UnstableStabMonoid : public UnstableMonoid
{
public:
	// Creates zero vector
	UnstableStabMonoid(uint dim);

	// The set containing the known small matrices
	unordered_set <OneCounterSmallMatrix> small_matrices;
	unordered_set <OneCounterLargeMatrix> large_matrices;

protected:
	pair <Matrix *, bool> addMatrix(Matrix * mat);

	/* converts an explicit matrix */
	Matrix * convertExplicitMatrix(const ExplicitMatrix & mat) const;

};

class UnstableMultiStabMonoid : public UnstableMonoid
{
public:
	// Creates zero vector
	UnstableMultiStabMonoid(uint dim);

	// The set containing the known small matrices
	unordered_set <MultiCounterMatrix> matrices;

protected:
	pair <Matrix *, bool> addMatrix(Matrix * mat);

	/* converts an explicit matrix */
	Matrix * convertExplicitMatrix(const ExplicitMatrix & mat) const;

};


#endif
