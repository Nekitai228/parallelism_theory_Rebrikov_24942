#include "heat_solver.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <iomanip>

HeatSolver::HeatSolver(int nx, int ny) : nx(nx), ny(ny) {
    A.resize(nx * ny, 0.0);
    Anew.resize(nx * ny, 0.0);
}

double HeatSolver::lerp(double a, double b, double t) const {
    return a + t * (b - a);
}

void HeatSolver::initialize() {
    std::fill(A.begin(), A.end(), 0.0);
    std::fill(Anew.begin(), Anew.end(), 0.0);

    // Граничные условия
    for (int j = 0; j < ny; j++) A[idx(0, j)] = lerp(10.0, 20.0, static_cast<double>(j)/(ny-1));
    for (int i = 0; i < nx; i++) A[idx(i, ny-1)] = lerp(20.0, 30.0, static_cast<double>(i)/(nx-1));
    for (int j = 0; j < ny; j++) A[idx(nx-1, j)] = lerp(20.0, 30.0, static_cast<double>(j)/(ny-1));
    for (int i = 0; i < nx; i++) A[idx(i, 0)] = lerp(10.0, 20.0, static_cast<double>(i)/(nx-1));

    std::copy(A.begin(), A.end(), Anew.begin());

    double* pA = A.data();
    double* pAnew = Anew.data();
    #pragma acc enter data copyin(pA[:nx*ny], pAnew[:nx*ny])
}

// === ОПТИМИЗИРОВАННОЕ ЯДРО ЯКОБИ ===
double HeatSolver::compute_kernel(int nx, int ny, double* __restrict__ A, double* __restrict__ Anew) {
    double error = 0.0;

    // parallel loop по внешнему циклу, vector по внутреннему (непрерывная память)
    #pragma acc parallel loop present(A[:nx*ny], Anew[:nx*ny]) reduction(max:error)
    for (int i = 1; i < nx - 1; i++) {
        #pragma acc loop vector
        for (int j = 1; j < ny - 1; j++) {
            int idx = i * ny + j;
            double new_val = 0.25 * (A[idx - ny] + A[idx + ny] + A[idx - 1] + A[idx + 1]);
            Anew[idx] = new_val;

            double diff = new_val - A[idx];
            if (diff < 0.0) diff = -diff;
            if (diff > error) error = diff;
        }
    }
    return error;
}

void HeatSolver::printGrid(int rows, int cols) const {
    const double* ptr_A = A.data();
    #pragma acc update self(ptr_A[0:nx*ny])

    int step_i = std::max(1, nx / (rows - 1));
    int step_j = std::max(1, ny / (cols - 1));

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "\nGrid (" << rows << "x" << cols << " sample from " 
              << nx << "x" << ny << "):\n";

    for (int i = 0; i < nx; i += step_i) {
        for (int j = 0; j < ny; j += step_j) {
            std::cout << std::setw(8) << A[idx(i, j)] << " ";
        }
        std::cout << "\n";
    }
    std::cout << std::endl;
}