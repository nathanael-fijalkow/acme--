/* INCLUDES */
#ifndef MATRIX_HPP
#define MATRIX_HPP


#include <string.h>
#include <iostream>
#include <unordered_set>

#include "Expressions.hpp"
#include "Vector.hpp"

typedef size_t HashMat;
typedef size_t HashRowCol;


using namespace std;

/* if 1 recurrent states and idempotent matrices are stored in a static array*/
#define CACHE_RECURRENT_STATES 1


/* CLASS DEFINITIONS */
// Class of explicit matrices, represented as arrays
class ExplicitMatrix
{
public:
	// Number of states of the matrix
	const uint stateNb;

	// coefficients is a C-style array
	// the coefficient i,j is stored at position  i * n + j
	// 0 means 0, 1 means +, 2 means 1
	char * coefficients;

	// Constructor 
	ExplicitMatrix(uint stateNb);

	//Random matrix
	static ExplicitMatrix  * random(uint stateNb);

	// Free a useless explicit matrix
	~ExplicitMatrix();
};


class Matrix
{
public:
	// Number of states of the matrix
	const uint stateNb;

	// Constructor from state nb
	Matrix(uint stateNb);

	// Print
	virtual void print(std::ostream& os = std::cout) const = 0;

	// Function computing the product and stabilization
	// They update the matrices, rows and columns
	//The caller is in charge of deleting the returned object
	virtual Matrix * prod(const Matrix  *) const = 0;

	// compute stabilisation
	//The caller is in charge of deleting the returned object
	virtual Matrix * stab() const = 0;

	// Function returning the hash
	HashMat hash() const { return _hash; };

	// Function checking whether a matrix is idempotent
	virtual bool isIdempotent() const = 0;

	// Two STATIC elements
	// This is the set of known vectors
	static std::unordered_set <const Vector> vectors;

	// This is the constant vector with only zero entries
	static const Vector * zero_vector;

protected:

	// The hash expression
	HashMat _hash;

	// Function computing the hash
	virtual void update_hash() = 0;
	
	// Function allocating memory, used by the constructor
	virtual void allocate() = 0;

	// Function used in the product
	static const Vector * sub_prod(const Vector *, const Vector **, size_t stateNb);

	// Create a new vector, keep only coordinates of v that are true in tab
#if USE_SPARSE_MATRIX
	static const Vector * purge(const Vector *varg, bool * tab);
#else
	static const Vector * purge(const Vector *varg, const Vector * tab);
#endif

};




/* for printing to a file */
std::ostream& operator<<(std::ostream& os, const Matrix & mat);


// Defines default hash for the matrix class
namespace std
{
	template <> struct hash<Matrix>
	{
		size_t operator()(const Matrix & mat) const
		{
			return mat.hash();
		}
	};

}

#endif
