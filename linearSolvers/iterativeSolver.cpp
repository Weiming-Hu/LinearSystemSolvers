/* Copyright (c) 2018 Weiming Hu
 * 
 * File:   iterativeSolvers.cpp
 * Author: Weiming Hu
 *
 * Created on September 15, 2018, 4:51 PM
 */

#include "Matrix.h"

#include <numeric>
#include <iomanip>
#include <cmath>
#include <ctime>

using namespace std;

void runGauss(const Matrix & A, const Matrix & b, Matrix & solution, int verbose) {
    // Gauss-Seidel Method
    //
    // For the linear system Ax = b,
    // let L and U be the strict lower and upper triangular portions of A and
    // D be the diagonal matrix, so that A = L + U + D.
    // We then define M = D + L and N = U so that A = M + N.
    // The iteration scheme is x_k+1 = M^-1 * (b - N * x_k);
    //

    // D is the diagonal matrix
    Matrix D(A.nrows(), A.ncols());
    for (size_t i = 0; i < D.nrows(); i++) D[i][i] = A[i][i];

    // L and U is the strict lower and upper part of the matrix A
    Matrix L(A.nrows(), A.ncols()), U(A.nrows());
    for (size_t i = 0; i < A.nrows(); i++) {
        for (size_t j = 0; j < i; j++) {
            L[i][j] = A[i][j];
        }

        for (size_t j = i + 1; j < A.nrows(); j++) {
            U[i][j] = A[i][j];
        }
    }

    // Define M_inv
    Matrix M_inv = (D + L).inverse();

    // Initialize the residual vector
    Matrix resids;

    // Initialize the residual metric
    double resid_metric = 999;

    // Initialize the residual threshold
    double small_resid = 1.0e-3;

    // Define the maximum iteration number
    size_t max_it = 1000;

    // Initialize the solution
    Matrix solution_new(b.nrows(), 1);
    std::srand(std::time(nullptr));
    for (size_t i = 0; i < solution_new.nrows(); i++) {
        solution_new[i][0] = rand();
    }

    if (verbose >= 3) {
        cout << "A is " << A << "b is " << b;
    }

    if (verbose >= 4) {
        cout << "D is " << D << "L is " << L << "U is " << U
            << "Initialized solution: " << solution_new << endl;
    }

    for (size_t i_it = 0; i_it < max_it && resid_metric > small_resid; i_it++) {

        solution = solution_new;
        solution_new = M_inv * (b - U * solution);

        resids = A * solution_new - b;
        resid_metric = accumulate(resids.begin(), resids.end(), 0.0, [](
                const double lhs, const vector<double> rhs) {
            return (lhs + abs(rhs[0]));
        });

        if (verbose >= 2) {
            cout << "Iteration " << i_it + 1 << " residual: " << resid_metric << endl;
        }
    }

    if (!A.checkDominant()) {
        cout << "Warning: Input matrix is not diagonally dominant."
                << " Jacobi Method might not converge." << endl;
    }

    if (verbose >= 4) {
        cout << "The iteration matrix M_inv * N is: " << M_inv * U << endl;
    }

    return;
}

void runJacobi(const Matrix & A, const Matrix & b, Matrix & solution, int verbose) {
    // Jacobi Method
    //
    // For the linear system Ax = b,
    // let D be the diagonal matrix and R = A - D, so that A = D + R.
    // The iteration scheme is x_k+1 = D^-1 * (b - R * x_k)
    //

    // D is the diagonal matrix
    Matrix D(A.nrows(), A.ncols());
    for (size_t i = 0; i < D.nrows(); i++) D[i][i] = A[i][i];

    // Inverse matrix D
    Matrix D_inv(D.inverse());

    // Initialize the matrix R
    Matrix R = A - D;

    // Initialize the residual vector
    Matrix resids;

    // Initialize the residual metric
    double resid_metric = 999;

    // Initialize the residual threshold
    double small_resid = 1.0e-3;

    // Define the maximum iteration number
    size_t max_it = 1000;

    // Initialize the solution
    Matrix solution_new(b.nrows(), 1);
    std::srand(std::time(nullptr));
    for (size_t i = 0; i < solution_new.nrows(); i++) {
        solution_new[i][0] = rand();
    }

    if (verbose >= 3) {
        cout << "A is " << A << "b is " << b;
    }

    if (verbose >= 4) {
        cout  << "D is " << D << "R is " << R
            << "Initialized solution: " << solution_new << endl;
    }

    for (size_t i_it = 0; i_it < max_it && resid_metric > small_resid; i_it++) {

        solution = solution_new;
        solution_new = D_inv * (b - R * solution);

        resids = A * solution_new - b;
        resid_metric = accumulate(resids.begin(), resids.end(), 0.0, [](
                const double lhs, const vector<double> rhs) {
            return (lhs + abs(rhs[0]));
        });

        if (verbose >= 2) {
            cout << "Iteration " << i_it + 1 << " residual: " << resid_metric << endl;
        }
    }

    if (!A.checkDominant()) {
        cout << "Warning: Input matrix is not diagonally dominant."
                << " Jacobi Method might not converge." << endl;
    }

    if (verbose >= 4) {
        cout << "The iteration matrix D_inv * R is: " << D_inv * R << endl;
    }
    
    return;
}

int main(int argc, char** argv) {

    if (argc != 4 && argc != 5) {
        cout << "iterativeSolver <Jacobi,J|Gauss,G> <matrix csv> <vector csv> [A verbose flag integer]"
             << endl << endl << "\tVerbose level specification: " << endl << "\t\t0 - Quiet" << endl
             << "\t\t1 - Result only" << endl << "\t\t2 - The above plus iteration information" << endl
             << "\t\t3 - The above plus input " << endl << "\t\t4 - The above plus transformed matrix" << endl;
        return 0;
    }

    // Read verbose flag
    int verbose = 1;
    if (argc == 5) {
        verbose = atoi(argv[argc - 1]);
    }
    
    // I store the vector b in form of a matrix with only one column.
    Matrix A, b;

    // Read input files
    A.readMatrix(argv[2]);
    b.readMatrix(argv[3]);

#ifdef _PROFILE_TIME
    clock_t time_start = clock();
#endif

    // Define solution
    Matrix solution;

    // Read function name
    string function_str(argv[1]);
    if (function_str == "Jacobi" || function_str == "J") {
        runJacobi(A, b, solution, verbose);

    } else if (function_str == "Gauss" || function_str == "G") {
        runGauss(A, b, solution, verbose);

    } else {
        cout << "Error: Unknown function name " << function_str << endl;
        return 1;
    }

#ifdef _PROFILE_TIME
    clock_t time_end = clock();

    double duration_total = (time_end - time_start) / (double) CLOCKS_PER_SEC;
    cout << setprecision(4) << "Total time for the iterative method: "
         << duration_total << "s" << endl;
#endif

    if (verbose >= 1) {
        if (function_str == "Jacobi" || function_str == "J") {
            cout << "Result from Jacobi x is " << endl << solution << endl;
        } else if (function_str == "Gauss" || function_str == "G") {
            cout << "Result from Gauss-Seidel x is " << endl << solution << endl;
        }
    }

    return 0;
}

