#include "OneCounterMatrix.hpp"


OneCounterLargeMatrix::OneCounterLargeMatrix()
{
	rows = (const Vector ***)malloc(4 * sizeof(const Vector **));
	cols = (const Vector ***)malloc(4 * sizeof(const Vector **));
	for (char act = 0; act<4; act++){
		rows[act] = (const Vector **)malloc(Vector::GetStateNb() * sizeof(const Vector *));
		cols[act] = (const Vector **)malloc(Vector::GetStateNb() * sizeof(const Vector *));
	}
	update_hash();
}

OneCounterSmallMatrix::OneCounterSmallMatrix()
{
	rows = (uint **)malloc(4 * sizeof(void *));
	cols = (uint **)malloc(4 * sizeof(void *));
	for (char act = 0; act<4; act++){
		rows[act] = (uint *)malloc(Vector::GetStateNb() * sizeof(uint));
		cols[act] = (uint *)malloc(Vector::GetStateNb() * sizeof(uint));
		
		memset(rows[act], 0, Vector::GetStateNb() * sizeof(uint));
		memset(cols[act], 0, Vector::GetStateNb() * sizeof(uint));
	}
	update_hash();
}

//Constructor from Explicit Matrix
OneCounterLargeMatrix::OneCounterLargeMatrix(const ExplicitMatrix & explMatrix)
{
	for (char act = 0; act<4; act++){
		for (uint i = 0; i < Vector::GetStateNb(); i++)
		{
			vector<bool> row(Vector::GetStateNb());
			vector<bool> col(Vector::GetStateNb());

			for (uint j = 0; j < Vector::GetStateNb(); j++)
			{
				char c1 = explMatrix.coefficients[i * Vector::GetStateNb() + j];
				char c2 = explMatrix.coefficients[j * Vector::GetStateNb() + i];
				row[j] = (c1 <= act);
				col[j] = (c2 <= act);
			}
			unordered_set<Vector>::iterator it;
			//vector<bool> a la place de Vector ?
			it = vectors.insert(row).first;
			rows[act][i] = &(*it);
			it = vectors.insert(col).first;
			cols[act][i] = &(*it);
		}
	}
	update_hash();
}

OneCounterSmallMatrix::OneCounterSmallMatrix(const ExplicitMatrix & explMatrix) : OneCounterSmallMatrix()
{
	for (char act = 0; act<4; act++){
		for (uint i = 0; i < Vector::GetStateNb(); i++)
		{
			for (int j = Vector::GetStateNb() - 1; j >= 0; j--)
			{
				char c1 = explMatrix.coefficients[i * Vector::GetStateNb() + j];
				char c2 = explMatrix.coefficients[j * Vector::GetStateNb() + i];
				rows[act][i] = (rows[act][i] << 1) | ( (c1 <= act) ? 1 : 0);
				cols[act][i] = (cols[act][i] << 1) | ((c2 <= act) ? 1 : 0);
			}
		}
	}
	update_hash();
}

//Print OneCounterMatrix
void OneCounterMatrix::print(std::ostream & os) const
{
	// CAUTION: sparse matrix not implemented
	string actions = "REIO_";
	/*
	for (uint i = 0; i < Vector::GetStateNb(); i++){
		for (char act = 0; act < 4; act++){
				{
					const Vector & row = *rows[act][i];
					os << row.bits << endl;
				}
		}
	}

	for (uint i = 0; i < Vector::GetStateNb(); i++){
		for (char act = 0; act < 4; act++){
				{
					const Vector & col = *cols[act][i];
					os << col.bits << endl;
				}
		}
	}
	*/

		//cout << "Row description " << endl;
	for (uint i = 0; i < Vector::GetStateNb(); i++){
		os << i << ":" << " ";
		for (uint j = 0; j < Vector::GetStateNb(); j++)
			os << actions[get(i, j)];
		os << endl;
	}
}

bool OneCounterLargeMatrix::operator==(const OneCounterLargeMatrix & mat) const
{
	//only on rows
	if (mat._hash != _hash) return false;

	for (char act = 0; act<4; act++){
		const Vector ** rows1 = rows[act];
		const Vector ** rows2 = mat.rows[act];
		for (; rows1 != rows[act] + Vector::GetStateNb(); rows1++, rows2++)
		{
			if (*rows1 != *rows2) return false;
		}
	}
	return true;
};

bool OneCounterSmallMatrix::operator==(const OneCounterSmallMatrix & mat) const
{
	//only on rows
	if (mat._hash != _hash) return false;

	for (char act = 0; act<4; act++)
		for (uint i = 0; i < Vector::GetStateNb(); i++)
			if (rows[act][i] != mat.rows[act][i])
				return false;
	return true;
};


Matrix * OneCounterLargeMatrix::prod(const Matrix * pmat1) const
{
	const OneCounterLargeMatrix & mat1 = *this;
	const OneCounterLargeMatrix & mat2 = *(OneCounterLargeMatrix *)pmat1;

	uint n = Vector::GetStateNb();
	OneCounterLargeMatrix * result = new OneCounterLargeMatrix();


	for (uint i = 0; i < n; i++)//special case for the reset: reset on one side and increment on the other is enough
	{
	result->rows[RESET][i] = sub_prod2(mat1.rows[RESET][i], mat2.cols[INC],mat1.rows[INC][i], mat2.cols[RESET]);
	result->cols[RESET][i] = sub_prod2(mat2.cols[RESET][i], mat1.rows[INC],mat2.cols[INC][i], mat1.rows[RESET]);
	}

	for (uint i = 0; i < n; i++)
	{
		result->rows[EPS][i] = sub_prodor(mat1.rows[EPS][i], mat2.cols[EPS], result->rows[RESET][i]);
		result->cols[EPS][i] = sub_prodor(mat2.cols[EPS][i], mat1.rows[EPS], result->cols[RESET][i]);
	}

	for (char act = INC; act<4; act++){
		for (uint i = 0; i < n; i++)
		{
			result->rows[act][i] = sub_prod(mat1.rows[act][i], mat2.cols[act]);
			result->cols[act][i] = sub_prod(mat2.cols[act][i], mat1.rows[act]);
		}
	}
	result->update_hash();
	return result;
}

Matrix * OneCounterSmallMatrix::prod(const Matrix * pmat1) const
{
	const OneCounterSmallMatrix & mat1 = *this;
	const OneCounterSmallMatrix & mat2 = *(OneCounterSmallMatrix *)pmat1;

	uint n = Vector::GetStateNb();
	OneCounterSmallMatrix * result = new OneCounterSmallMatrix();


	for (uint i = 0; i < n; i++)//special case for the reset: reset on one side and increment on the other is enough
	{
		uint & row_res = result->rows[RESET][i];
		uint & col_res = result->cols[RESET][i];
		row_res = 0;
		for (int j = n - 1; j >= 0; j--)
		{
			row_res = (row_res << 1) | ((mat1.rows[RESET][i] & mat2.cols[INC][j] | mat1.rows[INC][i] & mat2.cols[RESET][j]) ? 1 : 0);
			col_res = (col_res << 1) | ((mat2.cols[RESET][i] & mat1.rows[INC][j] | mat2.cols[INC][i] & mat1.rows[RESET][j]) ? 1 : 0);
		}
	}

	for (uint i = 0; i < n; i++)
	{
		uint & row_res = result->rows[EPS][i];
		uint & col_res = result->cols[EPS][i];
		for (int j = n - 1; j >= 0; j--)
		{
			row_res = (row_res << 1) | ((mat1.rows[EPS][i] & mat2.cols[EPS][j]) ? 1 : 0);
			col_res = (col_res << 1) | ((mat2.cols[EPS][i] & mat1.rows[EPS][j]) ? 1 : 0);
		}
		row_res |= result->rows[RESET][i];
		col_res |= result->cols[RESET][i];
	}

	for (char act = INC; act < 4; act++){
		for (uint i = 0; i < n; i++)
		{
			uint & row_res = result->rows[act][i];
			uint & col_res = result->cols[act][i];
			for (int j = n - 1; j >= 0; j--)
			{
				row_res = (row_res << 1) | ((mat1.rows[act][i] & mat2.cols[act][j]) ? 1 : 0);
				col_res = (col_res << 1) | ((mat2.cols[act][i] & mat1.rows[act][j]) ? 1 : 0);
			}
		}
	}
	result->update_hash();
	return result;
}

//works only on idempotents
Matrix * OneCounterLargeMatrix::stab() const
{
	uint n = Vector::GetStateNb();
	OneCounterLargeMatrix * result = new OneCounterLargeMatrix();

	uint * diags[4]; //sharp of the diagonal, for now on one uint

	//a peaufiner, pour l'instant 1 a priori, on ne prend que celui du premier vecteur.
	uint bitsN = Vector::GetBitSize();
	uint *new_row = (uint *)malloc(bitsN * sizeof(uint));
	uint *new_col = (uint *)malloc(bitsN * sizeof(uint));

	for (char act = 0; act<4; act++){
		if(act==INC) continue;
		diags[act]=(uint *)malloc(bitsN * sizeof(uint));
		for (uint b = 0; b<bitsN; b++){ //initialisation de la diagonale
			diags[act][b] = 0;
		}
		//compute the diagonal 
		//cout << " act:" << (int)act << "\n";
		for (uint i = 0; i <n; i++)
			if (rows[act][i]->contains(i))
				diags[act][i / (8 * sizeof(uint))] |= (1 << (i % (sizeof(uint) * 8)));
	}
	//system("pause");
	diags[INC] = diags[EPS]; //IC impossible, restriction to E


	for (char act = 0; act<4; act++){
		for (uint i = 0; i <n; i++){
			memset(new_row, 0, bitsN*sizeof(uint));
			memset(new_col, 0, bitsN*sizeof(uint));
			for (uint j = 0; j<n; j++){
				bool t = false;//temporary result for coef i,j

				//look for a possible path
				for (uint b = 0; b<bitsN; b++){ t = t || ((rows[act][i]->bits[b] & diags[act][b] & cols[act][j]->bits[b]) != 0); }
				new_row[j / (8 * sizeof(uint))] |= (t ? 1 : 0) << (j % (sizeof(uint) * 8));

				t = false;
				for (uint b = 0; b<bitsN; b++){ t = t || ((rows[act][j]->bits[b] & diags[act][b] & cols[act][i]->bits[b]) != 0); }
				new_col[j / (8 * sizeof(uint))] |= (t ? 1 : 0) << (j % (sizeof(uint) * 8));

			}

			auto it = vectors.emplace(new_row).first;
			result->rows[act][i] = &(*it);
			it = vectors.emplace(new_col).first;
			result->cols[act][i] = &(*it);

		}
	}

	free(new_row);
	free(new_col);

	free(diags[RESET]);
	free(diags[EPS]);
	free(diags[OM]);

	result->update_hash();
	return result;
}

//works only on idempotents
Matrix * OneCounterSmallMatrix::stab() const
{
	uint n = Vector::GetStateNb();
	OneCounterSmallMatrix * result = new OneCounterSmallMatrix();

	uint diags[4]; //sharp of the diagonal, for now on one uint
	
	for (char act = 0; act<4; act++)
	{
		diags[act] = 0;
	//compute the diagonal 
		for (uint i = 0; i <n; i++)
			diags[act] |= rows[act][i] & (1 << i);
	}
	diags[INC] = diags[EPS]; //IC impossible, restriction to E

	for (char act = 0; act<4; act++){
		for (uint i = 0; i <n; i++){
			uint & new_row = result->rows[act][i];
			uint & new_col = result->cols[act][i];
			new_row = 0; new_col = 0;
			for (int j = n-1; j >= 0; j--)
			{
				/*
				t = false;
				for (uint b = 0; b<bitsN; b++){ t = t || ((rows[act][i]->bits[b] & diags[act][b] & cols[act][j]->bits[b]) != 0); }
				new_row[j / (8 * sizeof(uint))] |= (t ? 1 : 0) << (j % (sizeof(uint) * 8));

				t = false;
				for (uint b = 0; b<bitsN; b++){ t = t || ((rows[act][j]->bits[b] & diags[act][b] & cols[act][i]->bits[b]) != 0); }
				new_col[j / (8 * sizeof(uint))] |= (t ? 1 : 0) << (j % (sizeof(uint) * 8));
				*/

				new_row = (new_row << 1) |  ( (rows[act][i] & diags[act] & cols[act][j]) ? 1 : 0);
				new_col = (new_col << 1) | ((rows[act][j] & diags[act] & cols[act][i]) ? 1 : 0);
			}
		}
	}

	result->update_hash();
	return result;
}

/* coefficients getters */
char OneCounterSmallMatrix::get(int i, int j) const
{
	for (char c = 0; c < 4; c++)
		if (rows[c][i] & (1 << j))
			return c;
	return 4;
}

char OneCounterLargeMatrix::get(int i, int j) const
{
	for (char c = 0; c < 4; c++)
		if (rows[c][i]->contains(j))
			return c;
	return 4;
}


bool OneCounterSmallMatrix::isIdempotent() const
{
	return (*this == *(OneCounterSmallMatrix *)(this->OneCounterSmallMatrix::prod(this)));
};

bool OneCounterLargeMatrix::isIdempotent() const
{
	return (*this == *(OneCounterLargeMatrix *)(this->OneCounterLargeMatrix::prod(this)));
};

const Vector * OneCounterLargeMatrix::sub_prodor(const Vector * vec, const Vector ** mat, const Vector * vecor)
{
	uint * new_vec = (uint *)malloc(Vector::GetBitSize() * sizeof(uint));
	memset(new_vec, 0, (size_t)(Vector::GetBitSize()  * sizeof(uint)));

	for (int j = Vector::GetStateNb() - 1; j >= 0; j--)
	{
		bool ok = false;
		if (mat[j] != Matrix::zero_vector)
			for (uint i = 0; i < Vector::GetBitSize(); i++)
			{
			ok = (vec->bits[i] & mat[j]->bits[i]) != 0;
			if (ok) break;
			}
		new_vec[j / (8 * sizeof(uint))] = (new_vec[j / (8 * sizeof(uint))] << 1) | (ok ? 1 : 0);
	}

	for (uint j = 0; j < Vector::GetBitSize(); j++)
		new_vec[j] |= vecor->bits[j];

	auto & it = vectors.emplace(new_vec).first;
	free(new_vec);
	//cout << "Final result "; (*it).print(); cout << endl;
	return &(*it);
}

// Construct a vector obtained by multiplying the line vec by all columns of mat, twice, and then disjunction of the two.
const Vector * OneCounterLargeMatrix::sub_prod2(const Vector * vec1, const Vector ** mat1, const Vector * vec2, const Vector ** mat2){
	if (vec1 == Matrix::zero_vector && vec2 == Matrix::zero_vector)	return Matrix::zero_vector;
	//no Sparse_Matrix

	uint * new_vec1 = (uint *)malloc(Vector::GetBitSize() * sizeof(uint));
	memset(new_vec1, 0, (size_t)(Vector::GetBitSize()  * sizeof(uint)));


	for (int j = Vector::GetStateNb() - 1; j >= 0; j--)
	{
		//cout << "Vector "; vec->print(); cout << endl;
		//cout << "times "; mat[j]->print(); cout << endl;

		bool ok = false;
		if (mat1[j] != Matrix::zero_vector)
			for (uint i = 0; i < Vector::GetBitSize(); i++)
			{
			ok = (vec1->bits[i] & mat1[j]->bits[i]) != 0;
			if (ok) break;
			}
		//cout << "Equal " << (ok ? 1 : 0) << endl;
		new_vec1[j / (8 * sizeof(uint))] = (new_vec1[j / (8 * sizeof(uint))] << 1) | (ok ? 1 : 0);
	}

	uint * new_vec2 = (uint *)malloc(Vector::GetBitSize() * sizeof(uint));
	memset(new_vec2, 0, (size_t)(Vector::GetBitSize()  * sizeof(uint)));


	for (int j = Vector::GetStateNb() - 1; j >= 0; j--)
	{
		//cout << "Vector "; vec->print(); cout << endl;
		//cout << "times "; mat[j]->print(); cout << endl;

		bool ok = false;
		if (mat2[j] != Matrix::zero_vector)
			for (uint i = 0; i < Vector::GetBitSize(); i++)
			{
			ok = (vec2->bits[i] & mat2[j]->bits[i]) != 0;
			if (ok) break;
			}
		//cout << "Equal " << (ok ? 1 : 0) << endl;
		new_vec2[j / (8 * sizeof(uint))] = (new_vec2[j / (8 * sizeof(uint))] << 1) | (ok ? 1 : 0);
	}

	uint * new_vec = (uint *)malloc(Vector::GetBitSize() * sizeof(uint));
	for (uint i = 0; i < Vector::GetBitSize(); i++) new_vec[i] = new_vec1[i] | new_vec2[i];

	auto it = vectors.emplace(new_vec);

	free(new_vec);

	//cout << "Final result "; (*it).print(); cout << endl;
	return &(*it.first);
}