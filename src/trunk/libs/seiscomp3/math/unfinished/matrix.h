/***************************************************************************
 *   Copyright (C) by GFZ Potsdam                                          *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/

# ifndef _MATRIX_H_
# define _MATRIX_H_

# define ZZ 0
# define NN 1
# define EE 2
# define ZN 3
# define ZE 4
# define NE 5

class Matrix
{
public:
    Matrix(unsigned rows, unsigned cols);  // Default constructor
    ~Matrix();                             // Destructor
    Matrix(Matrix const &m);               // Copy constructor

    void    operator=  (Matrix const &m);  // Assignment operator
    double& operator() (unsigned row, unsigned col);
    double  operator() (unsigned row, unsigned col) const;

private:
    unsigned nrows, ncols;  // number of rows/columns
    double *data;       // data
};


int comp_cov_mat (int, float*, float*, float*, float*);
int comp_cov_mat_eig_val (float*, float*);

# endif
