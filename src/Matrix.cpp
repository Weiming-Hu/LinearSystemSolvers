/* Copyright (c) 2018 Weiming Hu
 *
 * File:   matrix.cpp
 * Author: Weiming Hu
 * 
 * Created on September 10, 2018, 1:08 PM
 */

#include "Matrix.h"

#include <fstream>
#include <cmath>
#include <sstream>
#include <vector>
#include <numeric>
#include <iomanip>

#ifdef _PROFILE_TIME
#include <ctime>
#endif

#ifdef _USE_MPI
#include "mpi.h"
#endif

using namespace std;
static const double _ZERO_LIMIT = 1.0e-9;

Matrix::Matrix() {
}

Matrix::Matrix(size_t nsize) {
    resize(nsize, nsize);
}

Matrix::Matrix(size_t nrows, size_t ncols) {
    resize(nrows, ncols);
}

Matrix::Matrix(const Matrix& orig) {
    *this = orig;
}

Matrix::~Matrix() {
    resize(0, 0);
}

void
Matrix::resize(size_t nrows, size_t ncols) {
    vector< vector<double> >::resize(nrows);
    for (auto & row : * this) {
        row.resize(ncols, 0);
    }

    nrows_ = nrows;
    ncols_ = ncols;
}

size_t
Matrix::nrows() const {
    return (nrows_);
}

size_t
Matrix::ncols() const {
    return (ncols_);
}

bool
Matrix::checkDominant() const {
    double sum;
    for (size_t i = 0; i < nrows_; i++) {
        sum = accumulate((*this)[i].begin(), (*this)[i].end(), 0.0, [](
                double lhs, double rhs) {
            return (abs(lhs) + abs(rhs));
        });
        if ((*this)[i][i] < sum - (*this)[i][i]) {
            return false;
        }
    }
    return (true);
}

bool
Matrix::readMatrix(const std::string & csv_file) {
    ifstream file;
    file.open(csv_file);

    if (!file.is_open()) {
        throw runtime_error("File can't be opened.");
    }

    char comma;
    string buffer;
    double tmp_val;
    size_t nrows = 0;
    vector<double> tmp_vec;

    while (!file.eof()) {
        getline(file, buffer);

        if (!buffer.empty()) {
            istringstream iss(buffer);
            while (!iss.eof()) {
                iss >> tmp_val;
                tmp_vec.push_back(tmp_val);
                iss >> comma;
            }
            nrows++;
        }
    }

    size_t index = 0;
    size_t ncols = tmp_vec.size() / nrows;
    this->resize(nrows, ncols);

    for (size_t i = 0; i < nrows; i++) {
        for (size_t j = 0; j < ncols; j++, index++) {
            (*this)[i][j] = tmp_vec[index];
        }
    }

    return (true);
}


Matrix
Matrix::inverse() {
    
    if (nrows_ != ncols_)
        throw runtime_error("Matrix should be square!");
    
    size_t nsize = nrows_;
    Matrix mat_inv(nsize), mat(*this);
    
    // Initialize the inverse matrix to an identity matrix
    for (size_t i = 0; i < nsize; i++) {
        mat_inv[i][i] = 1;
    }

#ifdef _PROFILE_TIME
    clock_t time_start = clock();
#endif
    
    // Use Gaussian Elimination to solve the system
    //
    // Forward elimination
    for (size_t k = 0; k < nsize - 1; k++) {
        for (size_t i = k + 1; i < nsize; i++) {
            
            if (abs(mat[k][k]) < _ZERO_LIMIT) {
                ostringstream message;
                message << "0 occurs (" << mat[k][k]
                        << "). Please use row permutation.";
                throw runtime_error(message.str());
            }

            double coef = mat[i][k] / mat[k][k];

            for (size_t j = k; j < nsize; j++) {
                mat[i][j] -= mat[k][j] * coef;
            }

            for (size_t j = 0; j < nsize; j++) {
                mat_inv[i][j] -= mat_inv[k][j] * coef;
            }
        }
    }

#ifdef _PROFILE_TIME
    clock_t time_forward = clock();
#endif
    
    // Change the proceeding of each line to 1
    for (size_t i = 0; i < nsize; i++) {

        if (abs(mat[i][i]) < _ZERO_LIMIT) {
            ostringstream message;
            message << mat[i][i] << " occurs during inverse.";
            throw runtime_error(message.str());
        }
        double coef = mat[i][i];

        for (size_t j = i; j < nsize; j++) {
            mat[i][j] /= coef;
        }

        for (size_t j = 0; j < nsize; j++) {
            mat_inv[i][j] /= coef;
        }
    }

    // Backward elimination
    for (int i = nsize - 2; i >= 0; i--) {
        for (int j = nsize - 1; j > i; j--) {

            for (int m = 0; m < nsize; m++) {
                mat_inv[i][m] -= mat_inv[j][m] * mat[i][j];
            }

            mat[i][j] -= mat[j][j] * mat[i][j];
        }
    }

#ifdef _PROFILE_TIME
    clock_t time_backward = clock();
    
    double duration_forward = (time_forward - time_start) /(double) CLOCKS_PER_SEC;
    double duration_backward = (time_backward - time_forward) /(double) CLOCKS_PER_SEC;
    double duration_total = (time_backward - time_start) /(double) CLOCKS_PER_SEC;

    cout << setprecision(4) << "-----------------------------------" << endl
         << "Time profiling for the matrix inverse function:" << endl
         << "Forward elimination: " << duration_forward << "s (" << duration_forward / duration_total * 100 << "%)" << endl
         << "Backward elimination: " << duration_backward << "s (" << duration_backward / duration_total * 100 << "%)" << endl
         << "Inverse function total: " << duration_total << "s (" << duration_total / duration_total << ")" << endl
         << "-----------------------------------" << endl;
#endif
    
    return (mat_inv);
}

Matrix
Matrix::transpose() {
    
    Matrix mat_t(ncols_, nrows_);
    
    // Assign values
    for (size_t i = 0; i < nrows_; i++) {
        for (size_t j = 0; j < ncols_; j++) {
            mat_t[j][i] = (*this)[i][j];
        }
    }
    
    return mat_t;
}

void
Matrix::print(ostream & os) const {
    os << "Matrix [" << nrows_ << "][" << ncols_ << "]:" << endl;

    // Print the first line indicating the column numbers
    os << "\t";
    for (size_t i_col = 0; i_col < ncols_; i_col++) {
        os << "[ ," << i_col << "]\t";
    }
    os << endl;

    // Print values from each row
    for (size_t i_row = 0; i_row < nrows_; i_row++) {
        os << "[" << i_row << ", ]\t";
        for (size_t i_col = 0; i_col < ncols_; i_col++) {
            os << (*this)[i_row][i_col] << " \t";
        }
        os << endl;
    }
    os << endl;
}

Matrix &
        Matrix::operator=(const Matrix & rhs) {
    if (this != &rhs) {
        nrows_ = rhs.nrows_;
        ncols_ = rhs.ncols_;

        vector< vector<double> >::resize(nrows_);
        for (size_t i = 0; i < nrows_; i++) {
            (*this)[i].resize(ncols_);
            for (size_t j = 0; j < ncols_; j++) {
                (*this)[i][j] = rhs[i][j];
            }
        }
    }
    return (*this);
}

Matrix
operator+(const Matrix & lhs, const Matrix & rhs) {

    Matrix mat_add(0, 0);


        if (lhs.ncols_ != rhs.ncols_ || lhs.nrows_ != rhs.nrows_) {
            throw runtime_error("Matrices do not have the correct shape.");
        }

        mat_add.resize(lhs.nrows_, lhs.ncols_);

        for (size_t i = 0; i < lhs.nrows_; i++) {
            for (size_t j = 0; j < lhs.ncols_; j++) {
                mat_add[i][j] = lhs[i][j] + rhs[i][j];
            }
        }

    return (mat_add);
}

Matrix
operator-(const Matrix & lhs, const Matrix & rhs) {
    
    Matrix mat_minus(0, 0);

        if (lhs.ncols_ != rhs.ncols_ || lhs.nrows_ != rhs.nrows_) {
            throw runtime_error("Matrices do not have the correct shape.");
        }
        
        mat_minus.resize(lhs.nrows_, lhs.ncols_);

        for (size_t i = 0; i < lhs.nrows_; i++) {
            for (size_t j = 0; j < lhs.ncols_; j++) {
                mat_minus[i][j] = lhs[i][j] - rhs[i][j];
            }
        }

    return (mat_minus);
}

#ifdef _USE_MPI
Matrix
operator*(const Matrix & lhs, const Matrix & rhs) {
    // MPI version of matrix multiplication

    Matrix mat_mul(0, 0);

    if (lhs.ncols_ != rhs.nrows_) {
        throw runtime_error("Matrices do not have the correct shape.");
    }

    size_t mid = lhs.ncols_;
    size_t nrows = lhs.nrows_;
    size_t ncols = rhs.ncols_;

    mat_mul.resize(nrows, ncols);

    double sum = 0.0;
    for (size_t i = 0; i < nrows; i++) {
        for (size_t j = 0; j < ncols; j++) {
            sum = 0.0;
            for (size_t k = 0; k < mid; k++) {
                sum += lhs[i][k] * rhs[k][j];
            }
            mat_mul[i][j] = sum;
        }
    }

    return (mat_mul);
}
#else
Matrix
operator*(const Matrix & lhs, const Matrix & rhs) {
    // Serial version of matrix multiplication

    Matrix mat_mul(0, 0);

    if (lhs.ncols_ != rhs.nrows_) {
        throw runtime_error("Matrices do not have the correct shape.");
    }

    size_t mid = lhs.ncols_;
    size_t nrows = lhs.nrows_;
    size_t ncols = rhs.ncols_;

    mat_mul.resize(nrows, ncols);

    double sum = 0.0;
    for (size_t i = 0; i < nrows; i++) {
        for (size_t j = 0; j < ncols; j++) {
            sum = 0.0;
            for (size_t k = 0; k < mid; k++) {
                sum += lhs[i][k] * rhs[k][j];
            }
            mat_mul[i][j] = sum;
        }
    }

    return (mat_mul);
}
#endif

ostream &
operator<<(ostream & os, const Matrix & mat) {
    mat.print(os);
    return (os);
}

#ifdef _USE_MPI
int Matrix::_MPI_rank = -1;
int Matrix::_MPI_size = -1;

void
Matrix::_MPI_start() {
    MPI_init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &_MPI_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &_MPI_rank);
    return;
}

void
Matrix::_MPI_end() {
    MPI_Finalize();    
    return;
}
#endif
