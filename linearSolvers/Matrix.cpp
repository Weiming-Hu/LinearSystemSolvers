/* Copyright (c) 2018 Weiming Hu
 *
 * File:   matrix.cpp
 * Author: Weiming Hu
 * 
 * Created on September 10, 2018, 1:08 PM
 */

#include "Matrix.h"

using namespace std;

Matrix::Matrix() {
}

Matrix::Matrix(size_t nsize) {
    resize(nsize, nsize);
}

Matrix::Matrix(size_t nrows, size_t ncols) {
    resize(nrows, ncols);
}

//matrix::matrix(const matrix& orig) {
//}

Matrix::~Matrix() {
    resize(0, 0);
}

void
Matrix::resize(size_t nrows, size_t ncols) {
    vector< vector<double> >::resize(nrows);
    for (auto & row : *this) {
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

ostream &
operator<<(ostream & os, const Matrix & mat) {
    mat.print(os);
}