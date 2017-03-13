
#include <ProbMatrix.hpp>

using namespace std;

// Class Matrix
void ProbMatrix::allocate()
{
	myvectors = (const Vector **)malloc(4 * Vector::GetStateNb() * sizeof(void *));
}
ProbMatrix::~ProbMatrix()
{
    delete myvectors;
    myvectors = NULL;
}

//copy constructor and assignment operator
ProbMatrix::ProbMatrix( const ProbMatrix& other )
{
    allocate();
    *this = other;
    _hash = other.hash();
}

ProbMatrix& ProbMatrix::operator=( const ProbMatrix& other )
{
    memcpy(myvectors, other.myvectors, 4 * Vector::GetStateNb()  * sizeof(void *));
    _hash = other.hash();
    return *this;
}


const Vector *const *const ProbMatrix::getRowOnes() const
{
  return row_ones();
}
const Vector *const *const ProbMatrix::getRowPluses() const
{
  return row_pluses();
}

ProbMatrix::ProbMatrix()
{
    allocate();
}

// Convert an explicit matrix into a matrix
ProbMatrix::ProbMatrix(const ExplicitMatrix & explMatrix)
{
	allocate();

	for (uint i = 0; i < Vector::GetStateNb(); i++)
	{
		vector<bool> r_pluses(Vector::GetStateNb());
		vector<bool> r_ones(Vector::GetStateNb());
		vector<bool> c_pluses(Vector::GetStateNb());
		vector<bool> c_ones(Vector::GetStateNb());

        bool ok = false;
        
		for (uint j = 0; j < Vector::GetStateNb(); j++)
		{
			char c1 = explMatrix.coefficients[i][j];
			r_pluses[j] = (c1 >= 1);
			r_ones[j] = (c1 >= 2);
            ok |= r_ones[j];

			char c2 = explMatrix.coefficients[j][i];
			c_pluses[j] = (c2 >= 1);
			c_ones[j] = (c2 == 2);
		}
        
        if(!ok)
            throw runtime_error("Cannot create ProbMatrix: every row should have at least one '1' entry");

		unordered_set<Vector>::iterator it = vectors.emplace(r_ones).first;
		row_ones()[i] = &(*it);

		it = vectors.emplace(r_pluses).first;
		row_pluses()[i] = &(*it);

		it = vectors.emplace(c_ones).first;
		col_ones()[i] = &(*it);

		it = vectors.emplace(c_pluses).first;
		col_pluses()[i] = &(*it);
	}
	update_hash();
}

// Print
void ProbMatrix::print(std::ostream & os, vector<string> state_names) const
{
	//cout << "Row description " << endl;
	for (uint i = 0; i < Vector::GetStateNb(); i++)
	{
		os << (state_names.size() > i ? state_names[i] : to_string(i))  << ":" << " ";
		const Vector & ones = *row_ones()[i];
		const Vector & pluses = *row_pluses()[i];

		for (uint j = 0; j < Vector::GetStateNb(); j++)
			os << (ones.contains(j) ? "1" : pluses.contains(j) ? "+" : "_");
		os << endl;
	}
	/*
	Uncomment to get deep description of the matrix, including columns and hashes and adresses
	os << "Col description " << endl;
	for (uint i = 0; i < Vector::GetStateNb(); i++)
	{
	os << i << ":" << " ";
	const Vector & ones = *col_ones[i];
	const Vector & pluses = *col_pluses[i];

	uint jones = 0, jpluses = 0;
	for (uint j = 0; j < Vector::GetStateNb(); j++)
	{
	if (ones.entriesNb > jones && ones.entries[jones] == j)
	{
	os << ((pluses.entriesNb > jpluses && pluses.entries[jpluses] == j) ? "2 " : "1 ");
	jones++;
	jpluses++;
	}
	else if (pluses.entriesNb > jpluses && pluses.entries[jpluses] == j)
	{
	os << "+ ";
	jpluses++;
	}
	else
	os << "_ ";
	}
	os << endl;
	}
	for (uint i = 0; i < Vector::GetStateNb(); i++)
	{
	os << "row " << i << ":" << " ";
	os << "h" << row_ones[i]->Hash() << " #" << row_ones[i] << " h" << row_pluses[i]->Hash() << " #" << row_pluses[i] << " ";
	os << endl;
	}
	for (uint i = 0; i < Vector::GetStateNb(); i++)
	{
	os << "col " << i << ":" << " ";
	os << "h" << col_ones[i]->Hash() << " #" << col_ones[i] << " h" << col_pluses[i]->Hash() << " #" << col_pluses[i] << " ";
	os << endl;
	}
	*/
}

ExplicitMatrix* ProbMatrix::toExplicitMatrix() const 
{
        ExplicitMatrix* ret = new ExplicitMatrix(Vector::GetStateNb());
	for (uint i = 0; i < Vector::GetStateNb(); i++)
	{
	        const Vector & ones = *row_ones()[i];
		const Vector & pluses = *row_pluses()[i];

		for (uint j = 0; j < Vector::GetStateNb(); j++)
		  (ones.contains(j) ? 
		   ret->coefficients[i][j]='1' : 
		   pluses.contains(j) ? 
		   ret->coefficients[i][j]='+' : 
		   ret->coefficients[i][j]='_');
	}
	return ret;
}

bool ProbMatrix::operator==(const ProbMatrix & mat) const
{
    if (mat._hash != _hash) return false;
    auto N =Vector::GetStateNb();
    const Vector ** row = myvectors;
    const Vector ** row1 = mat.myvectors;
    for( ; row  != myvectors + 2*N; row++,row1++)
        if(*row != * row1)
            return false;
	return true;
};

// computes the list of recurrent states.
const Vector * ProbMatrix::recurrent_states() const
{
	size_t s = (Vector::GetStateNb() + 8 * sizeof(uint) - 1) / (8 * sizeof(uint));

	size_t * new_vec = (size_t *)malloc(s * sizeof(size_t));
	memset((void *)new_vec, (int)0, (size_t)(s * sizeof(size_t)));

	for (int i = Vector::GetStateNb() - 1; i >= 0; i--)
		new_vec[i / (8 * sizeof(uint))] = (new_vec[i / (8 * sizeof(uint))] << 1) | (recurrent(i) ? 1 : 0);

	auto it = vectors.emplace(new_vec, false).first;
	return &(*it);

}

/* computes the list of recurrence classesgiven the list of recurrent states.
The matrix is assumed to be idempotent. */
const Vector * ProbMatrix::recurrence_classes(const Vector * recs) const
{
	uint s = (Vector::GetStateNb() + 8 * sizeof(uint) - 1) / (8 * sizeof(uint));
	size_t * new_vec = (size_t *)malloc(s * sizeof(size_t));
	memcpy(new_vec, recs->bits, Vector::GetBitSize() * sizeof(size_t));

	//for each recurrent state we remove all its successors from the list of recurrent states
	uint b = 1;
	for (uint i = 0; i < Vector::GetStateNb(); i++)
	{
		auto r = row_ones()[i];
		if ((new_vec[i / (8 * sizeof(uint))] & b) != 0)
		{
			for (uint j = 0; j < Vector::GetStateNb(); j++)
			{
				if (i != j && r->contains(j))
				{
					//cout << "State " << j << " is in the same rec class than state " << i << ", deleting." << endl;
					new_vec[j / (8 * sizeof(uint))] &= ~b;
				}
			}
		}
		b *= 2;
	}

	auto it = vectors.emplace(new_vec, false).first;
	return &(*it);
}


uint ProbMatrix::countLeaks(const Vector * classes) const
{
	uint answer = 0;
	for (uint i = 0; i < Vector::GetStateNb(); i++)
	{
		if (classes->contains(i))
		{
			//			cout << "Checking leaks from state " << i << endl;
			for (uint j = 0; j < Vector::GetStateNb(); j++)
			{
				//				cout << "Checking leaks from state " << i << " to state " << j << endl;
				if (i != j && classes->contains(j) && row_pluses()[i]->contains(j))
				{
					answer++;
#if MONOID_COMPUTATION_VERBOSITY
					cout << "Found leak from class " << i << " to class " << j << endl;
#endif
					//				print();
				}
			}
		}
	}
	return answer;
}

bool ProbMatrix::check() const
{
	//at least one 1 per line
	for (uint i = 0; i < Vector::GetStateNb(); i++)
		if (row_ones()[i] == Matrix::zero_vector)
			return false;
	return true;
}

// Function computing the product of two matrices
const Matrix * ProbMatrix::prod(const Matrix * pmat1) const
{
	const ProbMatrix & mat1 = *this;
	const ProbMatrix & mat2 = *(ProbMatrix *)pmat1;
	uint n = Vector::GetStateNb();
	ProbMatrix * result = new ProbMatrix();

	for (uint i = 0; i < n; i++)
	{
		result->row_ones()[i] = sub_prod(mat1.row_ones()[i], mat2.col_ones());
		result->row_pluses()[i] = sub_prod(mat1.row_pluses()[i], mat2.col_pluses());
		result->col_ones()[i] = sub_prod(mat2.col_ones()[i], mat1.row_ones());
		result->col_pluses()[i] = sub_prod(mat2.col_pluses()[i], mat1.row_pluses());
	}

	result->update_hash();
	return result;
}

// Function checking whether j is recurrent. Only works if this is idempotent
bool ProbMatrix::recurrent(int j) const{
	auto r = row_ones()[j];
	for (uint i = 0; i < Vector::GetStateNb(); i++)
		if (r->contains(i) && !row_ones()[i]->contains(j))
			return false;
	return true;
}

// Function computing the stabilization. mat is assumed to be idempotent
const Matrix * ProbMatrix::stab(bool isIdempotentForSure) const
{
    if(!isIdempotentForSure)
        throw runtime_error("Stab of non idempotent matrice sunimplemented yet");
    
	const Vector * recs = recurrent_states();
	uint n = Vector::GetStateNb();
	ProbMatrix * result = new ProbMatrix();

	for (uint i = 0; i < n; i++)
	{
		result->row_ones()[i] = purge(row_ones()[i], recs);
		result->row_pluses()[i] = row_pluses()[i];
	}

	for (uint j = 0; j < n; j++)
	{
		if (recs->contains(j))
			result->col_ones()[j] = col_ones()[j];
		else
			result->col_ones()[j] = Matrix::zero_vector;
		result->col_pluses()[j] = col_pluses()[j];
	}
	result->update_hash();
	return result;
}

bool ProbMatrix::isIdempotent() const
{
  	return (*this == *(ProbMatrix *)(this->ProbMatrix::prod(this)));
}
